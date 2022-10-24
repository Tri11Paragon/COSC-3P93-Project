/*
 * Created by Brett Terpstra 6920201 on 22/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */


#ifndef STEP_3_GRAPHICS_H
#define STEP_3_GRAPHICS_H

// includes required to open a window on the system and render with opengl.
// we are using the GLX extension to the X11 windowing system
// instead of using external libs like GLFW and GLAD.
// Wayland is not and will not be supported.
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <config.h>
#include <engine/util/std.h>
#include <functional>
#include <graphics/input.h>

namespace Raytracing {
    class XWindow {
        private:
            // X11 display itself
            Display *display;
            // the desktop window or root window
            Window desktop;
            // our window's GL attributes, using full RGBA, with a depth of 24. Double buffering on the window.
            GLint OpenGLAttributes[5] { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
            // X11 stuff we have to have
            XVisualInfo *visualInfo;
            Colormap colormap;
            XSetWindowAttributes xSetWindowAttributes{};
            // our window which we will use to draw on
            Window window;
            GLXContext glContext;
            XWindowAttributes windowAttributes {};
            // used for event handling, like pressing a button or moving the mouse.
            XEvent events{};
            int m_width, m_height;
            int m_displayWidth, m_displayHeight;
            bool isCloseRequested = false;
            Atom wmDelete;
        public:
            XWindow(int width, int height);
            // runs X11 event processing and some GL commands used for window drawing
            void runUpdates(const std::function<void()>& drawFunction);
            [[nodiscard]] inline bool shouldWindowClose() const{ return isCloseRequested; }
            void closeWindow();
            ~XWindow();
    };
}

#endif //STEP_3_GRAPHICS_H
