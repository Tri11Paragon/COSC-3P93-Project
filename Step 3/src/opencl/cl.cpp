/*
 * Created by Brett Terpstra 6920201 on 20/11/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <opencl/cl.h>

namespace Raytracing {
    
    OpenCL openCl {0};
    
    void OpenCL::init() {
        openCl = OpenCL{0};
    }
    OpenCL::OpenCL(int platformID): activePlatform(platformID) {
        CL_err = CL_SUCCESS;
        numPlatforms = 0;
        CL_err = clGetPlatformIDs( 0, NULL, &numPlatforms );
    
        if (CL_err == CL_SUCCESS)
            dlog << "We found " << numPlatforms << " OpenCL Platforms.\n";
        else
            elog << "OpenCL Error! " << CL_err;
    
        platformsIDs = new cl_platform_id[numPlatforms];
        platformIDResult = clGetPlatformIDs(numPlatforms, platformsIDs, &numOfPlatformIDs);
        
    }
    OpenCL::~OpenCL() {
        delete[](platformsIDs);
    }
    void OpenCL::printDeviceInfo(cl_device_id device) {
    
    }
}