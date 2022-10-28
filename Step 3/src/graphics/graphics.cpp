/*
 * Created by Brett Terpstra 6920201 on 22/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <graphics/graphics.h>
#include <chrono>
#include <GLES/glext.h>
#include <graphics/gl/gl.h>

namespace Raytracing {
    
//    const std::vector<float> vertices = {
//            1.0f,  1.0f, 0.0f,  // top right
//            1.0f,  0.0f, 0.0f,  // bottom right
//            0.0f,  0.0f, 0.0f,  // bottom left
//            0.0f,  1.0f, 0.0f   // top left
//    };
//
//    const std::vector<unsigned int> indices = {
//            3, 1, 0,   // first triangle
//            3, 2, 1    // second triangle
//    };
//
//    const std::vector<float> texCoords = {
//            1.0f, 1.0f,   // top right
//            1.0f, 0.0f,   // bottom right
//            0.0f, 0.0f,   // bottom left
//            0.0f, 1.0f    // top left
//    };
    const std::vector<float> vertices = {
            1.0f,  1.0f, 0.0f,  // top right
            1.0f,  -1.0f, 0.0f,  // bottom right
            -1.0f,  -1.0f, 0.0f,  // bottom left
            -1.0f,  1.0f, 0.0f   // top left
    };
    
    const std::vector<unsigned int> indices = {
            3, 1, 0,   // first triangle
            3, 2, 1    // second triangle
    };
    
    const std::vector<float> texCoords = {
            1.0f, 1.0f,   // top right
            1.0f, 0.0f,   // bottom right
            0.0f, 0.0f,   // bottom left
            0.0f, 1.0f    // top left
    };
    
    VAO* quad = nullptr;
    
    void drawQuad() {
        if (quad == nullptr)
            quad = new VAO(vertices, texCoords, indices);
        
        quad->bind();
        quad->draw();
        quad->unbind();
    }
    
    void deleteQuad() {
        delete(quad);
    }
    
    XWindow::XWindow(int width, int height): m_width(width), m_height(height) {
        // open the DEFAULT display. We don't want to open a specific screen as that is annoying.
        dlog << "Creating X11 display!";
        display = XOpenDisplay(NULL);
        if (display == NULL)
            throw std::runtime_error("Unable to open an X11 display! Is the X server running?");
        // FBConfigs were added in GLX version 1.3.
        if (!glXQueryVersion( display, &glx_major, &glx_minor ))
            throw std::runtime_error("Unable to get GLX version!");
        if ((glx_major < 1) || (glx_major == 1  && glx_minor < 3))
            throw std::runtime_error("Invalid GLX Version. At least 1.3 is required!");
        // get the frame buffer config from the X11 window
        frameBufferConfig = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &frameBufferCount);
        if (!frameBufferConfig)
            throw std::runtime_error("Unable to get window framebuffer configs!");
        dlog << "We have " << frameBufferCount << " framebuffers\nSelecting the best!";
        // select the best config from available ones.
        int bestConfigIndex = -1, worstConfig = -1, bestSamples = -1, worstSamples = 999;
        for (int i = 0; i < frameBufferCount; i++){
            XVisualInfo *xVisualInfo = glXGetVisualFromFBConfig(display, frameBufferConfig[i]);
            if (xVisualInfo){
                int sampleBuffer, samples;
                glXGetFBConfigAttrib(display, frameBufferConfig[i], GLX_SAMPLE_BUFFERS, &sampleBuffer);
                glXGetFBConfigAttrib(display, frameBufferConfig[i], GLX_SAMPLES, &samples);
    
                dlog << "Checking framebuffer config " << i << " with samples: " << samples;
    
                if ( bestConfigIndex < 0 || sampleBuffer && samples > bestSamples )
                    bestConfigIndex = i, bestSamples = samples;
                if ( worstConfig < 0 || !sampleBuffer || samples < worstSamples )
                    worstConfig = i, worstSamples = samples;
            }
            XFree(xVisualInfo);
        }
        GLXFBConfig bestConfig = frameBufferConfig[bestConfigIndex];
        // we need to make sure we remember to free memory since we are working with c pointers!
        XFree(frameBufferConfig);
        
        // as I understand it every window in X11 is a sub-window of the root, or desktop window
        // which is why I guess wayland was created, because X11 can't handle a bunch of stuff like VRF (variable refresh rate)
        // because your say two monitors are treated as one big window, in effect limiting the refresh rate
        // to whatever the lowest is. I still don't like Wayland though. Forced VSync and a fragmented design is annoying.
        // plus needless security in a low level lib preventing stuff like discord screen sharing. Annoying as hell. /rant/.
        desktop = DefaultRootWindow(display);
        // try to open a gl visual context that meets our attributes' requirements
        dlog << "Getting visual info!";
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
        xSetWindowAttributes.event_mask =
                ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyReleaseMask | LeaveWindowMask |
                EnterWindowMask | FocusChangeMask | PointerMotionMask;
        m_displayWidth = XDisplayWidth(display, 0);
        m_displayHeight = XDisplayHeight(display, 0);
        window = XCreateWindow(display,
                               desktop,
                // center the display, even if the window manager ignores this.
                               m_displayWidth / 2 - width / 2,
                               m_displayHeight / 2 - height / 2,
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
        wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
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
    
        assignGLFunctionPointers();
        //glEnableVertexArrayAttribPtr = glXGetProcAddress((unsigned char*)("glEnableVertexArrayAttrib"));
        
        // Setup Dear IMGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        
        ImGui::StyleColorsDark();
        
        ImGui_ImplGlfw_Init();
        ImGui_ImplOpenGL3_Init("#version 130");
        
        
    }
    void XWindow::runUpdates(const std::function<void()>& drawFunction) {
        // only try to check events if they are queued
        while (XPending(display) > 0) {
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
                    break;
                case KeyPress: {
                    // translates xkeycodes to ascii keys
                    KeySym _key = XLookupKeysym(&events.xkey, 0);
                    ImGui_ImplGlfw_KeyCallback(_key, 0, true, 0);
                    break;
                }
                case KeyRelease: {
                    KeySym _key = XLookupKeysym(&events.xkey, 0);
                    ImGui_ImplGlfw_KeyCallback(_key, 0, false, 0);
                    break;
                }
                case ButtonPress:
                    if (events.xbutton.button < 4)
                        ImGui_ImplGlfw_MouseButtonCallback(events.xbutton.button - 1, true, 0);
                    else {
                        if (events.xbutton.button == 4)
                            ImGui_ImplGlfw_ScrollCallback(0, 1);
                        else if (events.xbutton.button == 5)
                            ImGui_ImplGlfw_ScrollCallback(0, -1);
                        else if (events.xbutton.button == 6)
                            ImGui_ImplGlfw_ScrollCallback(1, 0);
                        else if (events.xbutton.button == 7)
                            ImGui_ImplGlfw_ScrollCallback(-1, 0);
                    }
                    break;
                case ButtonRelease:
                    if (events.xbutton.button < 4)
                        ImGui_ImplGlfw_MouseButtonCallback(events.xbutton.button - 1, false, 0);
                    else {
                        ImGui_ImplGlfw_ScrollCallback(0, 0);
                    }
                    break;
                case MotionNotify:
                    ImGui_ImplGlfw_CursorPosCallback(events.xmotion.x, events.xmotion.y, true);
                    break;
                case EnterNotify:
                    ImGui_ImplGlfw_CursorEnterCallback(1, true);
                    break;
                case LeaveNotify:
                    ImGui_ImplGlfw_CursorEnterCallback(0, true);
                    break;
                case FocusIn:
                    ImGui_ImplGlfw_WindowFocusCallback(1);
                    break;
                case FocusOut:
                    ImGui_ImplGlfw_WindowFocusCallback(0);
                    break;
                case ClientMessage:
                    if (events.xclient.data.l[0] == wmDelete) {
                        closeWindow();
                        return;
                    }
                    break;
            }
        }
        glClearColor(0, 0, 0, 1);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame((double) std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::high_resolution_clock::now().time_since_epoch()).count(),
                                m_width,
                                m_height,
                                m_width,
                                m_height);
        ImGui::NewFrame();
    
        static bool t = true;
        if (t)
            ImGui::ShowDemoWindow(&t);
    
        // do our drawing
        drawFunction();
    
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ImGui_ImplGlfw_UpdateMouseData(true, true);
    
        // we use double buffering to prevent screen tearing and other visual disturbances
        glXSwapBuffers(display, window);
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
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        glXMakeCurrent(display, None, NULL);
        glXDestroyContext(display, glContext);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }
}