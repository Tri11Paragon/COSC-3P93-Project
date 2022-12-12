/*
 * Created by Brett Terpstra 6920201 on 22/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <graphics/graphics.h>
#include <chrono>
#include <graphics/gl/gl.h>
#include "engine/image/stb_image.h"
#include "graphics/debug_gui.h"
#include <engine/util/std.h>

namespace Raytracing {
    
    extern Signals* RTSignal;
    
    const std::vector<float> vertices = {
            1.0f, 1.0f, 0.0f,  // top right
            1.0f, -1.0f, 0.0f,  // bottom right
            -1.0f, -1.0f, 0.0f,  // bottom left
            -1.0f, 1.0f, 0.0f   // top left
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
        delete (quad);
    }
    
    struct ImageData {
        unsigned char* data;
        int width;
        int height;
        int channels;
    };
    
    GLFWimage getImageData(const std::string& file) {
        GLFWimage imager;
        int channels = 0;
        imager.pixels = stbi_load(file.c_str(), &imager.width, &imager.height, &channels, 4);
        return imager;
    }

#ifdef USE_GLFW
    XWindow::XWindow(int width, int height):
            m_displayWidth(width), m_displayHeight(height) {
        // OpenGL 4.6 is like 5 years old at this point and most systems support it
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        // We only generated GLAD headers for GL core profile. Plus renderdoc only support core profile.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // Apple stuff. Not like apple supports GL4.6 anyways :/
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
        glfwWindowHint(GLFW_DECORATED, GL_TRUE);
        glfwWindowHint(GLFW_FOCUSED, GL_TRUE);
        
        glfwSetErrorCallback([](int error_code, const char* description) -> void {
            elog << "GLFW Error: " << error_code << "\n\t" << description << "\n";
        });
        
        if (!glfwInit())
            throw std::runtime_error("Unable to init GLFW!\n");
        
        // create a window using the above settings,
        window = glfwCreateWindow(width, height, "GLFW 3P93 Raytracing Project", NULL, NULL);
        if (!window)
            throw std::runtime_error("Unable to create GLFW window!\n");
        glfwMakeContextCurrent(window);
        // enable V-Sync.
        glfwSwapInterval(1);
        // setup the callbacks we might use.
        auto imageData16 = getImageData("../resources/icon/icon16.png");
        auto imageData32 = getImageData("../resources/icon/icon32.png");
        GLFWimage images[2];
        // TODO: delete STBI_resize
        images[0] = imageData16;
        images[1] = imageData32;
        
        glfwSetWindowIcon(window, 2, images);
        stbi_image_free(imageData16.pixels);
        stbi_image_free(imageData32.pixels);
        
        
        glfwSetKeyCallback(window, [](GLFWwindow* _window, int key, int scancode, int action, int mods) -> void {
            if (action == GLFW_PRESS)
                Input::keyPressed(key);
            else if (action == GLFW_RELEASE)
                Input::keyReleased(key);
        });
        
        glfwSetMouseButtonCallback(window, [](GLFWwindow* _window, int button, int action, int mods) -> void {
            if (action == GLFW_PRESS)
                Input::mousePressed(button);
            else if (action == GLFW_RELEASE)
                Input::mouseReleased(button);
        });
        
        glfwSetCursorPosCallback(window, [](GLFWwindow* _window, double x, double y) -> void {
            Input::moveMove(x, y);
        });
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void) io;
        ImGui::StyleColorsDark();
        
        // create ImGUI GLFW instance and install the callbacks
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
        
        int version = gladLoadGL(glfwGetProcAddress);
        if (!version)
            throw std::runtime_error("Unable to load Glad GL!\n");
        
        glfwShowWindow(window);
        ilog << "Loaded GL" << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << "!\n";
    }
    void XWindow::closeWindow() {
        if (isCloseRequested)
            return;
        tlog << "Closing window!\n";
        isCloseRequested = true;
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    void XWindow::beginUpdate() {
        // check for window events
        isCloseRequested = glfwWindowShouldClose(window);
        // reset the current key-pressed state.
        Input::state();
        glfwPollEvents();
        // update window settings
        glfwGetFramebufferSize(window, &m_displayWidth, &m_displayHeight);
        glViewport(0, 0, m_displayWidth, m_displayHeight);
        glClearColor(0.5, 0.7, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::ShowDemoWindow(nullptr);
    }
    void XWindow::endUpdate() {
        // Render ImGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        auto _p1 = std::chrono::high_resolution_clock::now();
        auto _now = std::chrono::duration_cast<std::chrono::nanoseconds>(_p1.time_since_epoch()).count();
        long currentFrameTime = _now;
        delta = double(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;
        frameTimeMs = delta / 1000000.0;
        frameTimeS = delta / 1000000000.0;
        fps = 1000 / frameTimeMs;
    }
#else
    XWindow::XWindow(int width, int height): m_width(width), m_height(height) {
        // open the DEFAULT display. We don't want to open a specific screen as that is annoying.
        dlog << "Creating X11 display!\n";
        display = XOpenDisplay(NULL);
        if (display == NULL)
            throw std::runtime_error("Unable to open an X11 display! Is the X server running?");
        // FBConfigs were added in GLX version 1.3.
        if (!glXQueryVersion(display, &glx_major, &glx_minor))
            throw std::runtime_error("Unable to get GLX version!");
        if ((glx_major < 1) || (glx_major == 1 && glx_minor < 3))
            throw std::runtime_error("Invalid GLX Version. At least 1.3 is required!");
        // get the frame buffer config from the X11 window
        frameBufferConfig = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &frameBufferCount);
        if (!frameBufferConfig)
            throw std::runtime_error("Unable to get window framebuffer configs!");
        dlog << "We have " << frameBufferCount << " framebuffers\n";
        // select the best config from available ones.
        int bestConfigIndex = 0, bestSamples = -1;
        for (int i = 0; i < frameBufferCount; i++) {
            XVisualInfo* xVisualInfo = glXGetVisualFromFBConfig(display, frameBufferConfig[i]);
            if (xVisualInfo) {
                int sampleBuffer, samples;
                glXGetFBConfigAttrib(display, frameBufferConfig[i], GLX_SAMPLE_BUFFERS, &sampleBuffer);
                glXGetFBConfigAttrib(display, frameBufferConfig[i], GLX_SAMPLES, &samples);
                
                // if the sample buffer exists, and we have more samples in this config, make this config the one we use.
                if (sampleBuffer && samples > bestSamples) {
                    bestConfigIndex = i;
                    bestSamples = samples;
                }
            }
            XFree(xVisualInfo);
        }
        dlog << "We selected config: " << bestConfigIndex << " with " << bestSamples << "# of samples!\n";
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
        dlog << "Getting visual info!\n";
        visualInfo = glXChooseVisual(display, 0, OpenGLAttributes);
        // if our attributes are too much for the display, let's try reducing them. (modern hardware should support 24bit depth though)
        if (visualInfo == NULL) {
            wlog << "Unable to open a window with a depth of 24. Trying 16 bits.\n";
            OpenGLAttributes[2] = 16;
            visualInfo = glXChooseVisual(display, 0, OpenGLAttributes);
            if (visualInfo == NULL) {
                throw std::runtime_error("Unable to create window's visual context. Is your driver up to date?\n");
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
        // install a error handler
        // maybe we should set back the old one but I'd rather it goto std:err than crash the window
        XSetErrorHandler([](Display* displayPtr, XErrorEvent* eventPtr) -> int {
            elog << "An error occurred while trying to setup X11: " << eventPtr->error_code << ";\n " << eventPtr->minor_code << ";\n "
                 << eventPtr->request_code << "\n";
            return 0;
        });
        
        XStoreName(display, window, "3P93 Raytracing Project");
        ImageInput image("../resources/icon/icon.png");
        auto* imageData = image.getImageAsIconBuffer();
        
        auto hints = XAllocWMHints();
        hints->flags = IconPixmapHint | StateHint | IconPositionHint;
        auto pixMapRob = XCreateBitmapFromData(display, window, (const char*) (icon_bits), 32, 32);
        if (!pixMapRob)
            flog << "Unable to create icon pixel map\n";
        hints->icon_pixmap = pixMapRob;
        hints->initial_state = IconicState;
        hints->icon_x = 0;
        hints->icon_y = 0;
        XSetWMHints(display, window, hints);
        XFree(hints);
        
        int length = 32 * 32 + 2;
        //int length = 16 * 16 * 4 + 32*32 * 4 + 4 * 4;
        XChangeProperty(display,
                        window,
                        XInternAtom(display, "_NET_WM_ICON", False),
                        XInternAtom(display, "CARDINAL", False),
                        32,
                        PropModeReplace,
                        (const unsigned char*) imageData,
                        length);
        delete[](imageData);
        // there might actually be an argument to be made about X11 being outdated....
        wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
        XSetWMProtocols(display, window, &wmDelete, 1);
        
        // Now show the window
        XMapWindow(display, window);
        
        // get the list of GLX extensions for this system
        const char* glExtensions = glXQueryExtensionsString(display, DefaultScreen(display));
        // much in the same way that we get GL function pointers and use them we will do the same with the context creation
        auto glXCreateContextAttribsARBPtr = (PFNGLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddressARB((unsigned char*) "glXCreateContextAttribsARB");
        // now we can finally create a OpenGL context for our window
        int OpenGLContextAttributes[] = {
                // OpenGL major version, we want GL4.5+
                GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                // OpenGL minor version,
                GLX_CONTEXT_MINOR_VERSION_ARB, 5,
                // I don't remember what this does, but I know GLFW recommends that forward compatability be set true, (Pretty sure it's only an issue
                // on MacOS but I've always included this in all my projects so :shrug:
                GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                // Core profile for better Renderdoc compatibility + I don't need non core extensions
                GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                None
        };
        // now we can actually create and acquire the context
        glContext = glXCreateContextAttribsARBPtr(display, bestConfig, 0, True, OpenGLContextAttributes);
        // make sure if there was any error we are notified
        XSync(display, False);
        if (!glContext)
            flog << "Unable to create GL context!";
        if (glXIsDirect(display, glContext)) {
            ilog << "A direct GL context was acquired!\n";
        } else // direct contexts are faster than indirect!
            wlog << "Warning! Indirect context!\n";
        // make the currently executing thread the one current to the OpenGL context
        // since OpenGL is a single threaded finite state machine if we want to do mutli-threading with OpenGL (we don't)
        // this has to be called in each thread before we make use of any OpenGL function.
        glXMakeCurrent(display, window, glContext);
        // Now we can issue some OpenGL commands
        // we want to respect depth
        glEnable(GL_DEPTH_TEST);
        
        //assignGLFunctionPointers();
        //glEnableVertexArrayAttribPtr = glXGetProcAddress((unsigned char*)("glEnableVertexArrayAttrib"));
        
        // Setup Dear IMGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        
        ImGui::StyleColorsDark();
        
        ImGui_ImplGlfw_Init();
        ImGui_ImplOpenGL3_Init("#version 130");
        
        
    }
    void XWindow::beginUpdate() {
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
    }
    void XWindow::endUpdate() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ImGui_ImplGlfw_UpdateMouseData(true, true);
        
        // we use double buffering to prevent screen tearing and other visual disturbances
        glXSwapBuffers(display, window);
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
        
        XFree(visualInfo);
        glXMakeCurrent(display, None, NULL);
        glXDestroyContext(display, glContext);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }
#endif
    XWindow::~XWindow() {
        closeWindow();
        deleteKeys();
    }
    void XWindow::setMouseGrabbed(bool grabbed) {
        glfwSetInputMode(window, GLFW_CURSOR, grabbed ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
    bool XWindow::isMouseGrabbed() {
        return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
    }
    
    static bool started = false, debug = false;
    static int maxRayBounce = 50;
    static int raysPerPixel = 50;
    static float yaw = 0, pitch = 0;
    
    std::pair<Mat4x4, Mat4x4> DisplayRenderer::getCameraMatrices() {
        auto projection = m_camera.project();
        if (m_window.isMouseGrabbed()) {
            yaw += (float) Input::getMouseDelta().x * (1000.0f / ImGui::GetIO().Framerate / 1000.0f) * 3;
            pitch += (float) Input::getMouseDelta().y * (1000.0f / ImGui::GetIO().Framerate / 1000.0f) * 1.5f;
            
            const PRECISION_TYPE turnSpeed = 50;
            
            if (Input::isKeyDown(GLFW_KEY_LEFT))
                yaw += float(turnSpeed * m_window.getFrameTimeSeconds());
            if (Input::isKeyDown(GLFW_KEY_RIGHT))
                yaw -= float(turnSpeed * m_window.getFrameTimeSeconds());
            if (Input::isKeyDown(GLFW_KEY_UP))
                pitch += float(turnSpeed * m_window.getFrameTimeSeconds());
            if (Input::isKeyDown(GLFW_KEY_DOWN))
                pitch -= float(turnSpeed * m_window.getFrameTimeSeconds());
            
            PRECISION_TYPE moveAtX = 0, moveAtY = 0, moveAtZ = 0;
            PRECISION_TYPE speed = 40.0f;
            
            if (Input::isKeyDown(GLFW_KEY_LEFT_ALT))
                speed = 5.0f;
            if (Input::isKeyDown(GLFW_KEY_LEFT_CONTROL))
                speed = speed * 2;
            
            if (Input::isKeyDown(GLFW_KEY_W))
                moveAtX = speed * m_window.getFrameTimeSeconds();
            else if (Input::isKeyDown(GLFW_KEY_S))
                moveAtX = -speed * m_window.getFrameTimeSeconds();
            
            if (Input::isKeyDown(GLFW_KEY_A))
                moveAtZ = -speed * m_window.getFrameTimeSeconds();
            else if (Input::isKeyDown(GLFW_KEY_D))
                moveAtZ = speed * m_window.getFrameTimeSeconds();
            
            if (Input::isKeyDown(GLFW_KEY_SPACE))
                moveAtY = speed * m_window.getFrameTimeSeconds();
            else if (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT))
                moveAtY = -speed * m_window.getFrameTimeSeconds();
            
            PRECISION_TYPE radYaw = degreeeToRadian(yaw);
            
            PRECISION_TYPE deltaX = -moveAtX * std::sin(radYaw) + moveAtZ * std::cos(radYaw);
            PRECISION_TYPE deltaY = moveAtY;
            PRECISION_TYPE deltaZ = -moveAtX * std::cos(radYaw) + -moveAtZ * std::sin(radYaw);
            
            m_camera.setPosition(m_camera.getPosition() + Vec4{deltaX, deltaY, deltaZ});
        }
        auto view = m_camera.view(yaw, pitch);
        return {projection, view};
    }
    
    void DisplayRenderer::draw() {
        if (RTSignal->haltExecution) { m_window.closeWindow(); }
        if (Input::isKeyDown(GLFW_KEY_ESCAPE) && Input::isState(GLFW_KEY_ESCAPE))
            m_window.setMouseGrabbed(!m_window.isMouseGrabbed());
        
        DebugUI::render([this]() -> void {
            if (ImGui::Button("Start") && !started) {
                started = true;
                RTSignal->haltRaytracing = false;
                ilog << "Running raycaster!\n";
                // we don't actually have to check for --single since it's implied to be default true.
                int threads = std::stoi(m_parser.getOptionValue("--threads"));
                if (m_parser.hasOption("--mpi")) {
                    //m_raycaster.runMPI(raycaster.partitionScreen());
                } else if (m_parser.hasOption("--openmp")) {
                    m_raycaster.runOpenMP(threads);
                } else {
                    m_raycaster.runSTDThread(threads);
                }
            }
            if (ImGui::Checkbox("Pause", &RTSignal->pauseRaytracing)) {}
            if (ImGui::Button("Stop") && started) {
                RTSignal->haltRaytracing = true;
                started = false;
                m_raycaster.deleteThreads();
            }
            ImGui::NewLine();
            ImGui::InputInt("Max Ray Bounce", &maxRayBounce);
            ImGui::InputInt("Rays Per Pixel", &raysPerPixel);
            m_raycaster.updateRayInfo(maxRayBounce, raysPerPixel);
            ImGui::Checkbox("Debug", &debug);
        });
        
        // we want to be able to move around, and the camera matrix functions automatically recalculate image region & projection data.
        if (m_parser.hasOption("--gpu")) {
            getCameraMatrices();
        }
        
        if (debug) {
//            if (Input::isKeyDown(GLFW_KEY_E) && Input::isState(GLFW_KEY_E)) {
//                auto ray = m_camera.projectRay((PRECISION_TYPE) m_window.displayWidth() / 2, (PRECISION_TYPE) m_window.displayHeight() / 2);
//
//                //auto results = m_world.checkIfHit(ray, 0, 1000).first;
//                auto bvh = m_world.getBVH()->rayAnyHitIntersect(ray, 0, 1000);
//                //if (results.hit)
//                //    ilog << "World Results: " << results.hitPoint << " " << results.length << "\n";
//                //else
//                //    ilog << "World not hit.\n";
//                if (!bvh.empty())
//                    ilog << "BVH Results: " << bvh.size() << " " << bvh[0].ptr->getPosition() << "\n";
//                else
//                    ilog << "BVH not hit.\n";
//            }
//            if (Input::isKeyDown(GLFW_KEY_R) && Input::isState(GLFW_KEY_R))
//                m_world.getBVH()->resetNodes();
            auto matrices = getCameraMatrices();
            m_worldShader.setMatrix("projectMatrix", matrices.first);
            m_worldShader.setMatrix("viewMatrix", matrices.second);
            m_worldShader.use();
            auto objs = m_world.getObjectsInWorld();
            for (auto obj: objs) {
                if (obj->getVAO() != nullptr) {
                    obj->getVAO()->bind();
                    obj->getVAO()->draw(m_worldShader, {obj->getPosition()});
                }
            }
            DebugMenus::render();
        } else {
            m_imageShader.use();
            m_mainImage.updateImage();
            m_mainImage.bind();
            m_mainImage.enableGlTextures(1);
            drawQuad();
        }
        
        m_mainImage.updateImage();
    }
}