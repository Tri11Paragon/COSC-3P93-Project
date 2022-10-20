/*
 * Created by Brett Terpstra 6920201 on 17/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
// Yes, globals are bad.
#include <util/debug.h>

namespace Raytracing {
    std::unordered_map<std::string, profiler*> profiles;
}