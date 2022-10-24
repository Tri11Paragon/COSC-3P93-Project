/*
 * Created by Brett Terpstra 6920201 on 22/10/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_INPUT_H
#define STEP_3_INPUT_H

#define XK_LATIN1
#define XK_MISCELLANY
#include <graphics/keys.h>

namespace Raytracing {
    
    extern bool* keysDown;
    
    static void deleteKeys(){
        //delete[](keysDown);
    }
    
}

#endif //STEP_3_INPUT_H
