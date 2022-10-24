/*
 * Created by Brett Terpstra 6920201 on 22/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <graphics/graphics.h>

namespace Raytracing {
    
    void DrawAQuad() {
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1., 1., -1., 1., 1., 20.);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);
        
        glBegin(GL_QUADS);
        glColor3f(1., 0., 0.);
        glVertex3f(-.75, -.75, 0.);
        glColor3f(0., 1., 0.);
        glVertex3f(.75, -.75, 0.);
        glColor3f(0., 0., 1.);
        glVertex3f(.75, .75, 0.);
        glColor3f(1., 1., 0.);
        glVertex3f(-.75, .75, 0.);
        glEnd();
    }
    
    XWindow::XWindow(int width, int height): m_width(width), m_height(height) {
        // open the DEFAULT display. We don't want to open a specific screen as that is annoying.
        display = XOpenDisplay(NULL);
        if (display == NULL)
            throw std::runtime_error("Unable to open an X11 display! Is the X server running?");
        // as I understand it every window in X11 is a sub-window of the root, or desktop window
        // which is why I guess wayland was created, because X11 can't handle a bunch of stuff like VRF (variable refresh rate)
        // because your say two monitors are treated as one big window, in effect limiting the refresh rate
        // to whatever the lowest is. I still don't like Wayland though. Forced VSync and a fragmented design is annoying.
        // plus needless security in a low level lib preventing stuff like discord screen sharing. Annoying as hell. /rant/.
        desktop = DefaultRootWindow(display);
        // try to open a gl visual context that meets our attributes' requirements
        visualInfo = glXChooseVisual(display, 0, OpenGLAttributes);
        // if our attributes are too much for the display, let's try reducing them. (modern hardware should support 24bit depth though)
        if (visualInfo == NULL) {
            wlog << "Unable to open a window with a depth of 24. Trying 16 bits.\n";
            OpenGLAttributes[2] = 16;
            visualInfo = glXChooseVisual(display, 0, OpenGLAttributes);
            if (visualInfo == NULL) {
                throw std::runtime_error("Unable to create window's visual context. Is your driver up to date?");
            }
        }
        ilog << visualInfo->visualid << ": With depth: " << visualInfo->depth << " and RGB: " << visualInfo->bits_per_rgb << "\n";
        // use our requirements to create a colormap for the screen.
        colormap = XCreateColormap(display, desktop, visualInfo->visual, AllocNone);
        // arguments used to open a window for us
        xSetWindowAttributes.colormap = colormap;
        // what kind of events we want to receive
        xSetWindowAttributes.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyReleaseMask;
        m_displayWidth = XDisplayWidth(display, 0);
        m_displayHeight = XDisplayHeight(display, 0);
        window = XCreateWindow(display,
                               desktop,
                               // center the display, even if the window manager ignores this.
                               m_displayWidth/2 - width/2,
                               m_displayHeight/2 - height/2,
                               width,
                               height,
                               0,
                               visualInfo->depth,
                               InputOutput,
                               visualInfo->visual,
                               CWColormap | CWEventMask,
                               &xSetWindowAttributes);
        
        // Now show the window
        XMapWindow(display, window);
        XStoreName(display, window, "3P93 Raytracing Project");
        // there might actually be an argument to be made about X11 being outdated....
        wmDelete=XInternAtom(display, "WM_DELETE_WINDOW", True);
        XSetWMProtocols(display, window, &wmDelete, 1);
        // now we can finally create a OpenGL context for our window
        glContext = glXCreateContext(display, visualInfo, NULL, GL_TRUE);
        // make the currently executing thread the one current to the OpenGL context
        // since OpenGL is a single threaded finite state machine if we want to do mutli-threading with OpenGL (we don't)
        // this has to be called in each thread before we make use of any OpenGL function.
        glXMakeCurrent(display, window, glContext);
        // Now we can issue some OpenGL commands
        // we want to respect depth
        glEnable(GL_DEPTH_TEST);
        
    }
    void XWindow::runUpdates(const std::function<void()>& drawFunction) {
        XNextEvent(display, &events);
    
        // one of the few times I'll use a switch statement
        switch (events.type) {
            // called when the system thinks that the window should be updated
            // so on window resize events and of course on actual window update
            case Expose:
                // update window information
                XGetWindowAttributes(display, window, &windowAttributes);
                glViewport(0, 0, windowAttributes.width, windowAttributes.height);
                this->m_width = windowAttributes.width;
                this->m_height = windowAttributes.height;
                // do our drawing
                DrawAQuad();
                drawFunction();
                // we use double buffering to prevent screen tearing and other visual disturbances
                glXSwapBuffers(display, window);
                break;
            case KeyPress:
                tlog << events.xkey.keycode<< " " << "\n";
                break;
            case KeyRelease:
                
                break;
            case ClientMessage:
                if (events.xclient.data.l[0] == wmDelete)
                    closeWindow();
                break;
        }
    }
    XWindow::~XWindow() {
        closeWindow();
    }
    void XWindow::closeWindow() {
        // since this is called in the destructor, we don't want to double delete our window
        if (isCloseRequested)
            return;
        tlog << "Closing window!\n";
        isCloseRequested = true;
        glXMakeCurrent(display, None, NULL);
        glXDestroyContext(display, glContext);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }
}