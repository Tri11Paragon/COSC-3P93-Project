/*
 * Created by Brett Terpstra 6920201 on 25/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 *
 *
 * I HAVE MODIFIED THIS FILE SIGNIFICANTLY!
 * I AM NOT USING GLFW
 * BUT HAVE TAKEN THE BACKEND FOR IT AND ADAPTED IT FOR RAW X11
 *
 */
// dear imgui: Platform Backend for GLFW
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)
// (Info: GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (Requires: GLFW 3.1+)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass ImGuiKey values to all key functions e.g. ImGui::IsKeyPressed(ImGuiKey_Space). [Legacy GLFW_KEY_* values will also be supported unless IMGUI_DISABLE_OBSOLETE_KEYIO is set]
//  [X] Platform: Gamepad support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange' (note: the resizing cursors requires GLFW 3.4+).

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs


#include "graphics/imgui/imgui.h"
#include "graphics/imgui/imgui_impl_glfw.h"
#define XK_LATIN1
#define XK_MISCELLANY
#include <graphics/keys.h>
#include <engine/util/logging.h>

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// - Because glfwPollEvents() process all windows and some events may be called outside of it, you will need to register your own callbacks
//   (passing install_callbacks=false in ImGui_ImplGlfw_InitXXX functions), set the current dear imgui context and then call our callbacks.
// - Otherwise we may need to store a GLFWWindow* -> ImGuiContext* map and handle this in the backend, adding a little bit of extra complexity to it.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.


//// Functions
//static const char* ImGui_ImplGlfw_GetClipboardText(void* data) {
////    auto sel = XInternAtom(display, "CLIPBOARD", false);
////    auto utf = XInternAtom(display, "UTF8_STRING", false);
////    Window WindowWithSelection = XGetSelectionOwner(display, sel);
////    if (WindowWithSelection == None)
////        return "";
////    auto fake = XCreateSimpleWindow(display, root, -10, -10, 1, 1, 0, 0, 0);
////    auto fakeProperty = XInternAtom(display, "UGHX11", false);
////    XConvertSelection(display, sel, utf, fakeProperty, fake, CurrentTime);
////    int di;
////    unsigned long size, dul;
////    unsigned char* clipboard;
////    XGetWindowProperty(display, fake, fakeProperty, 0, 0, false, AnyPropertyType, &type, )
//// TODO:
//    return "\0";
//}
//
//void ImGui_ImplGlfw_SetClipboardText(void* data) {
//    // TODO:
//    //glfwSetClipboardString((GLFWwindow*)user_data, text);
//}

static ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(unsigned int key) {
    switch (key) {
        case XK_Tab: return ImGuiKey_Tab;
        case XK_Left: return ImGuiKey_LeftArrow;
        case XK_Right: return ImGuiKey_RightArrow;
        case XK_Up: return ImGuiKey_UpArrow;
        case XK_Down: return ImGuiKey_DownArrow;
        case XK_Page_Up: return ImGuiKey_PageUp;
        case XK_Page_Down: return ImGuiKey_PageDown;
        case XK_Home: return ImGuiKey_Home;
        case XK_End: return ImGuiKey_End;
        case XK_Insert: return ImGuiKey_Insert;
        case XK_Delete: return ImGuiKey_Delete;
        case XK_BackSpace: return ImGuiKey_Backspace;
        case XK_space: return ImGuiKey_Space;
        case XK_Return: return ImGuiKey_Enter;
        case XK_Escape: return ImGuiKey_Escape;
        case XK_apostrophe: return ImGuiKey_Apostrophe;
        case XK_comma: return ImGuiKey_Comma;
        case XK_minus: return ImGuiKey_Minus;
        case XK_period: return ImGuiKey_Period;
        case XK_slash: return ImGuiKey_Slash;
        case XK_semicolon: return ImGuiKey_Semicolon;
        case XK_equal: return ImGuiKey_Equal;
        case XK_bracketleft: return ImGuiKey_LeftBracket;
        case XK_backslash: return ImGuiKey_Backslash;
        case XK_bracketright: return ImGuiKey_RightBracket;
        case XK_grave: return ImGuiKey_GraveAccent;
        case XK_Caps_Lock: return ImGuiKey_CapsLock;
        case XK_Scroll_Lock: return ImGuiKey_ScrollLock;
        case XK_Num_Lock: return ImGuiKey_NumLock;
        case XK_Print: return ImGuiKey_PrintScreen;
        case XK_Pause: return ImGuiKey_Pause;
        case XK_KP_0: return ImGuiKey_Keypad0;
        case XK_KP_1: return ImGuiKey_Keypad1;
        case XK_KP_2: return ImGuiKey_Keypad2;
        case XK_KP_3: return ImGuiKey_Keypad3;
        case XK_KP_4: return ImGuiKey_Keypad4;
        case XK_KP_5: return ImGuiKey_Keypad5;
        case XK_KP_6: return ImGuiKey_Keypad6;
        case XK_KP_7: return ImGuiKey_Keypad7;
        case XK_KP_8: return ImGuiKey_Keypad8;
        case XK_KP_9: return ImGuiKey_Keypad9;
        case XK_KP_Decimal: return ImGuiKey_KeypadDecimal;
        case XK_KP_Divide: return ImGuiKey_KeypadDivide;
        case XK_KP_Multiply: return ImGuiKey_KeypadMultiply;
        case XK_KP_Subtract: return ImGuiKey_KeypadSubtract;
        case XK_KP_Add: return ImGuiKey_KeypadAdd;
        case XK_KP_Enter: return ImGuiKey_KeypadEnter;
        case XK_KP_Equal: return ImGuiKey_KeypadEqual;
        case XK_Shift_L: return ImGuiKey_LeftShift;
        case XK_Control_L: return ImGuiKey_LeftCtrl;
        case XK_Alt_L: return ImGuiKey_LeftAlt;
        case XK_Super_L: return ImGuiKey_LeftSuper;
        case XK_Shift_R: return ImGuiKey_RightShift;
        case XK_Control_R: return ImGuiKey_RightCtrl;
        case XK_Alt_R: return ImGuiKey_RightAlt;
        case XK_Super_R: return ImGuiKey_RightSuper;
        case XK_Menu: return ImGuiKey_Menu;
        case XK_0: return ImGuiKey_0;
        case XK_1: return ImGuiKey_1;
        case XK_2: return ImGuiKey_2;
        case XK_3: return ImGuiKey_3;
        case XK_4: return ImGuiKey_4;
        case XK_5: return ImGuiKey_5;
        case XK_6: return ImGuiKey_6;
        case XK_7: return ImGuiKey_7;
        case XK_8: return ImGuiKey_8;
        case XK_9: return ImGuiKey_9;
        case XK_A: return ImGuiKey_A;
        case XK_B: return ImGuiKey_B;
        case XK_C: return ImGuiKey_C;
        case XK_D: return ImGuiKey_D;
        case XK_E: return ImGuiKey_E;
        case XK_F: return ImGuiKey_F;
        case XK_G: return ImGuiKey_G;
        case XK_H: return ImGuiKey_H;
        case XK_I: return ImGuiKey_I;
        case XK_J: return ImGuiKey_J;
        case XK_K: return ImGuiKey_K;
        case XK_L: return ImGuiKey_L;
        case XK_M: return ImGuiKey_M;
        case XK_N: return ImGuiKey_N;
        case XK_O: return ImGuiKey_O;
        case XK_P: return ImGuiKey_P;
        case XK_Q: return ImGuiKey_Q;
        case XK_R: return ImGuiKey_R;
        case XK_S: return ImGuiKey_S;
        case XK_T: return ImGuiKey_T;
        case XK_U: return ImGuiKey_U;
        case XK_V: return ImGuiKey_V;
        case XK_W: return ImGuiKey_W;
        case XK_X: return ImGuiKey_X;
        case XK_Y: return ImGuiKey_Y;
        case XK_Z: return ImGuiKey_Z;
        case XK_a: return ImGuiKey_A;
        case XK_b: return ImGuiKey_B;
        case XK_c: return ImGuiKey_C;
        case XK_d: return ImGuiKey_D;
        case XK_e: return ImGuiKey_E;
        case XK_f: return ImGuiKey_F;
        case XK_g: return ImGuiKey_G;
        case XK_h: return ImGuiKey_H;
        case XK_i: return ImGuiKey_I;
        case XK_j: return ImGuiKey_J;
        case XK_k: return ImGuiKey_K;
        case XK_l: return ImGuiKey_L;
        case XK_m: return ImGuiKey_M;
        case XK_n: return ImGuiKey_N;
        case XK_o: return ImGuiKey_O;
        case XK_p: return ImGuiKey_P;
        case XK_q: return ImGuiKey_Q;
        case XK_r: return ImGuiKey_R;
        case XK_s: return ImGuiKey_S;
        case XK_t: return ImGuiKey_T;
        case XK_u: return ImGuiKey_U;
        case XK_v: return ImGuiKey_V;
        case XK_w: return ImGuiKey_W;
        case XK_x: return ImGuiKey_X;
        case XK_y: return ImGuiKey_Y;
        case XK_z: return ImGuiKey_Z;
        case XK_F1: return ImGuiKey_F1;
        case XK_F2: return ImGuiKey_F2;
        case XK_F3: return ImGuiKey_F3;
        case XK_F4: return ImGuiKey_F4;
        case XK_F5: return ImGuiKey_F5;
        case XK_F6: return ImGuiKey_F6;
        case XK_F7: return ImGuiKey_F7;
        case XK_F8: return ImGuiKey_F8;
        case XK_F9: return ImGuiKey_F9;
        case XK_F10: return ImGuiKey_F10;
        case XK_F11: return ImGuiKey_F11;
        case XK_F12: return ImGuiKey_F12;
        default: return ImGuiKey_None;
    }
}

int ImGui_ImplGlfw_KeyToModifier(unsigned int key) {
    if (key == XK_Control_L || key == XK_Control_R)
        return RT_CONTROL;
    if (key == XK_Shift_L || key == XK_Shift_R)
        return RT_SHIFT;
    if (key == XK_Alt_L || key == XK_Alt_R)
        return RT_ALT;
    if (key == XK_Super_L || key == XK_Super_R)
        return RT_SUPER;
    return 0;
}

void ImGui_ImplGlfw_UpdateKeyModifiers(int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiModFlags_Ctrl, (mods & RT_CONTROL) != 0);
    io.AddKeyEvent(ImGuiModFlags_Shift, (mods & RT_SHIFT) != 0);
    io.AddKeyEvent(ImGuiModFlags_Alt, (mods & RT_ALT) != 0);
    io.AddKeyEvent(ImGuiModFlags_Super, (mods & RT_SUPER) != 0);
}

void ImGui_ImplGlfw_MouseButtonCallback(unsigned int button, bool press, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (button < ImGuiMouseButton_COUNT)
        io.AddMouseButtonEvent((int)button, press);
}

void ImGui_ImplGlfw_ScrollCallback(double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
}

static int ImGui_ImplGlfw_TranslateUntranslatedKey(unsigned int key, int scancode) {
#if GLFW_HAS_GETKEYNAME && !defined(__EMSCRIPTEN__)
    // GLFW 3.1+ attempts to "untranslate" keys, which goes the opposite of what every other framework does, making using lettered shortcuts difficult.
    // (It had reasons to do so: namely GLFW is/was more likely to be used for WASD-type game controls rather than lettered shortcuts, but IHMO the 3.1 change could have been done differently)
    // See https://github.com/glfw/glfw/issues/1502 for details.
    // Adding a workaround to undo this (so our keys are translated->untranslated->translated, likely a lossy process).
    // This won't cover edge cases but this is at least going to cover common cases.
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_EQUAL)
        return key;
    const char* key_name = glfwGetKeyName(key, scancode);
    if (key_name && key_name[0] != 0 && key_name[1] == 0)
    {
        const char char_names[] = "`-=[]\\,;\'./";
        const int char_keys[] = { GLFW_KEY_GRAVE_ACCENT, GLFW_KEY_MINUS, GLFW_KEY_EQUAL, GLFW_KEY_LEFT_BRACKET, GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH, GLFW_KEY_COMMA, GLFW_KEY_SEMICOLON, GLFW_KEY_APOSTROPHE, GLFW_KEY_PERIOD, GLFW_KEY_SLASH, 0 };
        IM_ASSERT(IM_ARRAYSIZE(char_names) == IM_ARRAYSIZE(char_keys));
        if (key_name[0] >= '0' && key_name[0] <= '9')               { key = GLFW_KEY_0 + (key_name[0] - '0'); }
        else if (key_name[0] >= 'A' && key_name[0] <= 'Z')          { key = GLFW_KEY_A + (key_name[0] - 'A'); }
        else if (key_name[0] >= 'a' && key_name[0] <= 'z')          { key = GLFW_KEY_A + (key_name[0] - 'a'); }
        else if (const char* p = strchr(char_names, key_name[0]))   { key = char_keys[p - char_names]; }
    }
    // if (action == GLFW_PRESS) printf("key %d scancode %d name '%s'\n", key, scancode, key_name);
#else
    IM_UNUSED(scancode);
#endif
    return key;
}

void ImGui_ImplGlfw_KeyCallback(unsigned int keycode, int scancode, bool press, int mods) {
    // Workaround: X11 does not include current pressed/released modifier key in 'mods' flags. https://github.com/glfw/glfw/issues/1630
    if (int keycode_to_mod = ImGui_ImplGlfw_KeyToModifier(keycode))
        mods = (press) ? (mods | keycode_to_mod) : (mods & ~keycode_to_mod);
    ImGui_ImplGlfw_UpdateKeyModifiers(mods);
    
    //keycode = ImGui_ImplGlfw_TranslateUntranslatedKey(keycode, scancode);
    
    ImGuiIO& io = ImGui::GetIO();
    ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(keycode);
    if (!press && ((keycode < 127 && keycode > 80) || (keycode < 64 && keycode > 31)))
        io.AddInputCharacter(keycode);
    io.AddKeyEvent(imgui_key, press);
    io.SetKeyEventNativeData(imgui_key, (int)keycode, scancode); // To support legacy indexing (<1.87 user code)
}

void ImGui_ImplGlfw_WindowFocusCallback(int focused) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddFocusEvent(focused != 0);
}

void ImGui_ImplGlfw_CursorPosCallback(double x, double y, bool cursorVisible) {
    if (!cursorVisible)
        return;
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent((float)x, (float)y);
}

ImVec2 LastValidMousePos {0, 0};

// Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
// so we back it up and restore on Leave/Enter (see https://github.com/ocornut/imgui/issues/4984)
void ImGui_ImplGlfw_CursorEnterCallback(int entered, bool cursorVisible) {
    if (!cursorVisible)
        return;
    
    ImGuiIO& io = ImGui::GetIO();
    if (entered) {
        io.AddMousePosEvent(LastValidMousePos.x, LastValidMousePos.y);
    } else {
        LastValidMousePos = io.MousePos;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }
}

void ImGui_ImplGlfw_CharCallback(unsigned int c) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

void ImGui_ImplGlfw_MonitorCallback(int) {
    // Unused in 'master' branch but 'docking' branch will use this, so we declare it ahead of it so if you have to install callbacks you can install this one too.
}

void ImGui_ImplGlfw_InstallCallbacks() {
}

void ImGui_ImplGlfw_RestoreCallbacks() {

}

bool ImGui_ImplGlfw_Init() {
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    //printf("GLFW_VERSION: %d.%d.%d (%d)", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION, GLFW_VERSION_COMBINED);
    
    // Setup backend capabilities flags
    io.BackendPlatformName = "imgui_impl_glfw";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    
    // TODO
    //io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
    //io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
    //io.ClipboardUserData = bd->Window;
    
    // Create mouse cursors
    // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
    // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
    // Missing cursors will return nullptr and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
/*    GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
    bd->MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
    bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
    bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif*/
    // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    return true;
}

void ImGui_ImplGlfw_Shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    
    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
}

void ImGui_ImplGlfw_UpdateMouseData(bool cursorVisible, bool focused) {
    ImGuiIO& io = ImGui::GetIO();
    
    if (!cursorVisible) {
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        return;
    }
    if (focused) {
        // TODO:
        // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
        //if (io.WantSetMousePos)
        //    glfwSetCursorPos(bd->Window, (double)io.MousePos.x, (double)io.MousePos.y);
        
        // (Optional) Fallback to provide mouse position when focused (ImGui_ImplGlfw_CursorPosCallback already provides this when hovered or captured)
        /*if (is_app_focused && bd->MouseWindow == nullptr)
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(bd->Window, &mouse_x, &mouse_y);
            io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
            bd->LastValidMousePos = ImVec2((float)mouse_x, (float)mouse_y);
        }*/
    }
}

void ImGui_ImplGlfw_UpdateMouseCursor() {
    // TODO:
    /*ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(bd->Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;
    
    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        glfwSetInputMode(bd->Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    else
    {
        // Show OS mouse cursor
        // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
        glfwSetCursor(bd->Window, bd->MouseCursors[imgui_cursor] ? bd->MouseCursors[imgui_cursor] : bd->MouseCursors[ImGuiMouseCursor_Arrow]);
        glfwSetInputMode(bd->Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }*/
}

double DTime = 0;

void ImGui_ImplGlfw_NewFrame(double time, int display_w, int display_h, int w, int h) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / (float)w, (float)display_h / (float)h);
    
    // Setup time step
    double current_time = time / 1000.0;
    io.DeltaTime = DTime > 0.0 ? (float)(current_time - DTime) : (float)(1.0f / 60.0f);
    DTime = current_time;
    
    ImGui_ImplGlfw_UpdateMouseCursor();
}
