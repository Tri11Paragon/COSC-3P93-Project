/*
 * Created by Brett Terpstra 6920201 on 25/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 *
 * I HAVE MODIFIED THIS FILE SIGNIFICANTLY!
 * I AM NOT USING GLFW
 * BUT HAVE TAKEN THE BACKEND FOR IT AND ADAPTED IT FOR RAW X11
 */

#ifndef STEP_3_IMGUI_IMPL_X11_H
#define STEP_3_IMGUI_IMPL_X11_H

// dear imgui: Platform Backend for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass ImGuiKey values to all key functions e.g. ImGui::IsKeyPressed(ImGuiKey_Space). [Legacy GLFW_KEY_* values will also be supported unless IMGUI_DISABLE_OBSOLETE_KEYIO is set]
//  [X] Platform: Gamepad support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange' (note: the resizing cursors requires GLFW 3.4+).

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "graphics/imgui/imgui.h"      // IMGUI_IMPL_API

IMGUI_IMPL_API void     ImGui_ImplX11_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplX11_NewFrame(double time, int display_w, int display_h, int w, int h);
IMGUI_IMPL_API bool     ImGui_ImplX11_Init();

// GLFW callbacks (installer)
// - When calling Init with 'install_callbacks=true': ImGui_ImplX11_InstallCallbacks() is called. GLFW callbacks will be installed for you. They will chain-call user's previously installed callbacks, if any.
// - When calling Init with 'install_callbacks=false': GLFW callbacks won't be installed. You will need to call individual function yourself from your own GLFW callbacks.
IMGUI_IMPL_API void     ImGui_ImplX11_InstallCallbacks();
IMGUI_IMPL_API void     ImGui_ImplX11_RestoreCallbacks();

// GLFW callbacks (individual callbacks to call if you didn't install callbacks)
IMGUI_IMPL_API void     ImGui_ImplX11_WindowFocusCallback(int focused);        // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplX11_CursorEnterCallback(int entered, bool cursorVisible);        // Since 1.84
IMGUI_IMPL_API void     ImGui_ImplX11_CursorPosCallback(double x, double y, bool cursorVisible);   // Since 1.87
IMGUI_IMPL_API void     ImGui_ImplX11_MouseButtonCallback(unsigned int button, bool pressed, int mods);
IMGUI_IMPL_API void     ImGui_ImplX11_ScrollCallback(double xoffset, double yoffset);
IMGUI_IMPL_API void     ImGui_ImplX11_KeyCallback(unsigned int key, int scancode, bool pressed, int mods);
IMGUI_IMPL_API void     ImGui_ImplX11_CharCallback(unsigned int c);
IMGUI_IMPL_API void     ImGui_ImplX11_MonitorCallback(int event);
IMGUI_IMPL_API void     ImGui_ImplX11_UpdateMouseData(bool cursorVisible, bool focused);

#endif //STEP_3_IMGUI_IMPL_X11_H
