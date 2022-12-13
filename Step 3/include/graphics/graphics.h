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
#include <config.h>

#ifndef USE_GLFW
    #include <X11/X.h>
    #include <X11/Xlib.h>
    #include <GLES3/gl32.h>
    #include <GL/gl.h>
    #include <GL/glx.h>
    #include "graphics/imgui/imgui_impl_x11.h"
#else
    
    #include <graphics/gl/glad/gl.h>
    #include <GLFW/glfw3.h>
    #include "graphics/imgui/imgui_impl_glfw.h"

#endif

#include <config.h>
#include <engine/util/std.h>
#include <functional>
#include <graphics/input.h>
#include "graphics/imgui/imgui_impl_opengl3.h"
#include "graphics/gl/gl.h"
#include "engine/raytracing.h"
#include <engine/image/image.h>
#include <engine/types.h>

namespace Raytracing {
    
    void drawQuad();
    
    void deleteQuad();

#ifdef USE_GLFW
    
    /**
     * The class used to create and handle window management.
     */
    class XWindow {
        private:
            GLFWwindow* window;
            int m_displayWidth, m_displayHeight;
            bool isCloseRequested = false;
            long lastFrameTime{};
            PRECISION_TYPE delta{};
            PRECISION_TYPE frameTimeMs{}, frameTimeS{};
            PRECISION_TYPE fps{};
        public:
            XWindow(int width, int height);
            
            // runs X11 event processing and some GL commands used for window drawing
            void beginUpdate();
            
            void endUpdate();
            
            [[nodiscard]] inline bool shouldWindowClose() const { return isCloseRequested; }
            
            /**
             * @return time between frames in milliseconds
             */
            [[nodiscard]] inline PRECISION_TYPE getFrameTimeMillis() const { return frameTimeMs; }
            
            /**
             * @return time between frames in seconds
             */
            [[nodiscard]] inline PRECISION_TYPE getFrameTimeSeconds() const { return frameTimeS; }
            
            /**
             * @return the number of frames in the last second
             */
            [[nodiscard]] inline PRECISION_TYPE getFPS() const { return fps; }
            
            void setMouseGrabbed(bool grabbed);
            
            bool isMouseGrabbed();
            
            [[nodiscard]] inline int displayWidth() const { return m_displayWidth; }
            
            [[nodiscard]] inline int displayHeight() const { return m_displayHeight; }
            
            [[nodiscard]] inline GLFWwindow* getWindow() const { return window; }
            
            void closeWindow();
            
            ~XWindow();
    };

#else
    class XWindow {
        private:
            // X11 display itself
            Display *display;
            // the desktop window or root window
            Window desktop;
            // our window's GL attributes, using full RGBA, with a depth of 24. Double buffering on the window.
            GLint OpenGLAttributes[5] { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
            const int visual_attribs[23] = {
                            GLX_X_RENDERABLE    , True,
                            GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
                            GLX_RENDER_TYPE     , GLX_RGBA_BIT,
                            GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
                            GLX_RED_SIZE        , 8,
                            GLX_GREEN_SIZE      , 8,
                            GLX_BLUE_SIZE       , 8,
                            GLX_ALPHA_SIZE      , 8,
                            GLX_DEPTH_SIZE      , 24,
                            GLX_STENCIL_SIZE    , 8,
                            GLX_DOUBLEBUFFER    , True,
                            //GLX_SAMPLE_BUFFERS  , 1,
                            //GLX_SAMPLES         , 4,
                            None
                    };
            int glx_major, glx_minor, frameBufferCount;
            GLXFBConfig* frameBufferConfig;
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
#endif
    
    /**
     * The display renderer class handles all the major rendering events outside of window functions
     * like ImGUI and GL stuff. These events include running ImGUI draw commands, pushing of current raycasting result to the gpu, and debug mode graphics.
     */
    class DisplayRenderer {
        private:
            XWindow& m_window;
            Texture& m_mainImage;
            World& m_world;
            Shader& m_imageShader;
            Shader& m_worldShader;
            RayCaster& m_raycaster;
            Parser& m_parser;
            Camera& m_camera;
        public:
            DisplayRenderer(
                    XWindow& mWindow,
                    Texture& mMainImage,
                    World& world,
                    Shader& mImageShader,
                    Shader& mWorldShader,
                    RayCaster& mRaycaster,
                    Parser& mParser,
                    Camera& mCamera
            )
                    :
                    m_window(mWindow), m_mainImage(mMainImage), m_imageShader(mImageShader), m_worldShader(mWorldShader), m_raycaster(mRaycaster),
                    m_parser(mParser), m_camera(mCamera), m_world(world) {}
            
            std::pair<Mat4x4, Mat4x4> getCameraMatrices();
            
            /**
             * Handles all the drawing nonsense, call from the main update loop once per frame.
             */
            void draw();
    };
}

#endif //STEP_3_GRAPHICS_H
