/*
 * Created by Brett Terpstra 6920201 on 20/11/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_CL_H
#define STEP_3_CL_H

// OpenCL includes
#include <CL/cl.h>

#include <engine/util/std.h>

namespace Raytracing {

    class OpenCL {
        private:
            cl_int CL_err;
            cl_uint numPlatforms;
            int activePlatform;
            cl_int platformIDResult;
            
            cl_platform_id* platformsIDs;
            cl_uint numOfPlatformIDs;
            
            void printDeviceInfo(cl_device_id device);
            
        public:
            explicit OpenCL(int platformID);
            static void init();
        
            ~OpenCL();
    };

}

#endif //STEP_3_CL_H
