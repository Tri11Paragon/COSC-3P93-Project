/*
 * Created by Brett Terpstra 6920201 on 30/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_DEBUG_GUI_H
#define STEP_3_DEBUG_GUI_H

#include <functional>
#include <string>

namespace Raytracing {
    class DebugUI {
        public:
            static void render(const std::function<void()>& generalTab);
            static void registerTab(const std::string& name, const std::function<void()>& tabFunc);
    };
}

#endif //STEP_3_DEBUG_GUI_H
