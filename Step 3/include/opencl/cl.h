/*
 * Created by Brett Terpstra 6920201 on 20/11/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_CL_H
#define STEP_3_CL_H

// OpenCL includes
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <config.h>

#ifdef COMPILE_GUI

#endif


#include <engine/util/std.h>

namespace Raytracing {
    
    
    class CLProgram {
        private:
            cl_int m_CL_ERR{};
            std::string m_source;
            
            cl_device_id m_deviceID{};
            cl_context m_context{};
            cl_command_queue m_commandQueue{};
            cl_program m_program{};
            
            std::unordered_map<std::string, cl_mem> buffers;
            std::unordered_map<std::string, cl_kernel> kernels;
            
            void checkBasicErrors() const;
        public:
            explicit CLProgram(const std::string& file);
            void loadCLShader(cl_context context, cl_device_id deviceID);
            
            void createKernel(const std::string& kernelName);
            void createBuffer(const std::string& bufferName, cl_mem_flags flags, size_t bytes);
            void createBuffer(const std::string& bufferName, cl_mem_flags flags, size_t bytes, void* ptr);
            void createImage(const std::string& imageName, int width, int height);
            
            void setKernelArgument(const std::string& kernel, const std::string& buffer, int argIndex);
            
            void runKernel(const std::string& kernel, size_t globalWorkSize, size_t localWorkSize, const size_t* globalWorkOffset = NULL);
            void runKernel(const std::string& kernel, size_t* globalWorkSize, size_t* localWorkSize, cl_uint workDim = 1, const size_t* globalWorkOffset = NULL);
            
            void writeBuffer(const std::string& buffer, size_t bytes, void* ptr, cl_bool blocking = CL_TRUE, size_t offset = 0);
            void readBuffer(const std::string& buffer, size_t bytes, void* ptr, cl_bool blocking = CL_TRUE, size_t offset = 0);
            void readImage(const std::string& imageName, size_t width, size_t height, void* ptr, cl_bool blocking = CL_TRUE, size_t x = 0, size_t y = 0);
            
            void flushCommands();
            void finishCommands();
            
            ~CLProgram();
    };
    
    class OpenCL {
        private:
            cl_int m_CL_ERR;
            cl_uint m_numPlatforms;
            int m_activePlatform;
            
            cl_platform_id* m_platformIDs;
            cl_uint m_numOfPlatformIDs{};
            
            cl_device_id m_deviceID{};
            cl_uint m_numOfDevices{};
            
            cl_uint m_computeUnits;
            cl_uint m_deviceClockFreq;
            
            cl_context m_context;
            
            void printDeviceInfo(cl_device_id device);
        public:
            explicit OpenCL(int platformID = 0, int deviceID = 0);
            static void init();
            static void createCLProgram(CLProgram& program);
            static cl_uint activeDeviceComputeUnits();
            static cl_uint activeDeviceFrequency();
        
            ~OpenCL();
    };

}

#endif //STEP_3_CL_H
