/*
 * Created by Brett Terpstra 6920201 on 20/11/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */

#ifndef STEP_3_CL_H
#define STEP_3_CL_H

// OpenCL includes
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <engine/image/image.h>
#include <config.h>

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
            
            /**
             * Checks for some basic errors after calling OpenCL commands. Stuff like GPU out of memory... etc.
             */
            void checkBasicErrors() const;
        
        public:
            /**
             * Loads the shader from a file on class creation
             * @param file file to load OpenCL "shader" (code) file
             */
            explicit CLProgram(const std::string& file);
            
            /**
             * Used by the OpenCL class to create a basic OpenCL program
             * @param context provided by the OpenCL class.
             * @param deviceID provided by the OpenCL class.
             */
            void loadCLShader(cl_context context, cl_device_id deviceID);
            
            /**
             * Kernels are the entry points in OpenCL. You can have multiple of them in a single program.
             * @param kernelName both the name of the kernel function in the source and the reference to the kernel object used in other functions in this class.
             */
            void createKernel(const std::string& kernelName);
            
            
            /**
             * Buffers are the quintessential datastructures in OpenCL. They are basically regions of memory allocated to a program.
             * @param bufferName the name of the buffer used to store internally
             * @param flags read write flags for the buffer. One of CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE
             * @param bytes the number of bytes to be allocated.
             */
            void createBuffer(const std::string& bufferName, cl_mem_flags flags, size_t bytes);
            
            /**
             * Creates a buffer on the GPU using the data pointed to by the supplied pointer. This copy happens as soon as this is called.
             * @param bufferName the name of the buffer used to store internally
             * @param flags One of CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE
             * @param bytes the number of bytes to be allocated. Must be less than equal to the number of bytes at ptr
             * @param ptr the pointer to copy to the GPU.
             */
            void createBuffer(const std::string& bufferName, cl_mem_flags flags, size_t bytes, void* ptr);
            
            /**
             * Creates a buffer on the GPU using the data pointed to by the supplied pointer. This copy happens as soon as this is called.
             * @param bufferName the name of the buffer used to store internally
             * @param flags One of CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE
             * @param bytes the number of bytes to be allocated. Must be less than equal to the number of bytes at ptr
             * @param ptr the pointer to copy to the GPU.
             */
            void createImage(const std::string& imageName, int width, int height);
            
            /**
             * Allows you to bind certain buffers to a specific index in the kernel's argument list.
             * @param kernel kernel to bind to
             * @param buffer buffer to bind to argIndex
             * @param argIndex the index of the argument for this buffer.
             */
            void setKernelArgument(const std::string& kernel, const std::string& buffer, int argIndex);
            
            /**
             * Runs the kernel code on the GPU. Is blocking.
             * @param kernel kernel function name to call
             * @param globalWorkSize the total number of times to execute the kernel function code. Corresponds to the result of get_global_id
             * @param localWorkSize how many work items make up a work group to be executed by a kernel. 64 is recommended, must not exceed the printed value "device max workgroup size"
             * @param globalWorkOffset not used. can be used to set an offset to the result of get_global_id
             */
            void runKernel(const std::string& kernel, size_t globalWorkSize, size_t localWorkSize, const size_t* globalWorkOffset = NULL);
            
            
            /**
             * Runs the kernel code on the GPU. Is blocking.
             * This version allows you to specify the number of work dimensions.
             * globalWorkSize and localWorkSize must be an array of workDim size which specify the work size for each kernel
             * For example a work dim of 2 allows for two separate work sizes to be set per dimension.
             * An image is two dimensional and so global work size would be {width of image, height of image}
             * and local work size would be size_t localWork = {8, 8} for a total of 64 (again recommended).
             * The resulting execution causes get_global_id(0) to run [0, width) times and get_global_id(1) to run [0, height) times
             * @param kernel kernel function name to call
             * @param globalWorkSize the total number of times to execute the kernel function code. Corresponds to the result of get_global_id(dim)
             * @param localWorkSize how many work items make up a work group to be executed by a kernel. total 64 is recommended, total must not exceed the printed value "device max workgroup size"
             * @param workDim number of dimensions to the work group being executed.
             * @param globalWorkOffset not used. can be used to set an offset to the result of get_global_id
             */
            void runKernel(
                    const std::string& kernel, size_t* globalWorkSize, size_t* localWorkSize, cl_uint workDim = 1,
                    const size_t* globalWorkOffset = NULL
            );
            
            /**
             * Enqueues a write command to the buffer specified by the buffer name,
             * @param buffer the buffer to write to
             * @param bytes the number of bytes to be copied
             * @param ptr the pointer to copy from. Must have at least bytes available
             * @param blocking should this function wait for the bytes to be uploaded to the GPU?
             * @param offset offset in the buffer object to write to
             */
            void writeBuffer(const std::string& buffer, size_t bytes, void* ptr, cl_bool blocking = CL_TRUE, size_t offset = 0);
            
            /**
             * Enqueues a read command from the buffered specified by the buffer name.
             * Defaults to blocking but can be set to be non-blocking.
             * @param buffer buffer to read from
             * @param bytes the number of bytes to read. Make sure ptr has at least those bytes available.
             * @param ptr the ptr to write the read bytes to.
             * @param blocking should we wait for the read or do it async?
             * @param offset offset in the buffer to read from.
             */
            void readBuffer(const std::string& buffer, size_t bytes, void* ptr, cl_bool blocking = CL_TRUE, size_t offset = 0);
            
            /**
             * Reads an image from the GPU into the memory region specified. Allocated memory region must be large enough to hold the image.
             * @param imageName name of the buffer to read from
             * @param width width of the image. Must be less than or equal to the width of the image on the GPU
             * @param height height of the image. Also must be less than or equal to the height of the image on the GPU
             * @param ptr pointer to the memory region to read into
             * @param blocking should we wait for the read operation to complete? Defaults to yes.
             * @param x x coordinate to start the read from. Defaults to zero since it's unlikely to be needed here. Included for possible future use.
             * @param y y coordinate to start the read from.
             */
            void readImage(
                    const std::string& imageName, size_t width, size_t height, void* ptr, cl_bool blocking = CL_TRUE, size_t x = 0, size_t y = 0
            );
            
            /**
             * Reads an image buffer into a RayCasting Image class.
             * Image supplied must have a with and height that matches the width and height of the image buffer specified by the name.
             * @param imageName name of the buffer you wish to read from
             * @param image reference to an image that you want the GPU data read into.
             */
            void readImage(const std::string& imageName, Image& image);
            
            /**
             * Issues all previously queued OpenCL commands in a command-queue to the device associated with the command-queue.
             */
            void flushCommands();
            
            /**
             * Blocks until all previously queued OpenCL commands in a command-queue are issued to the associated device and have completed.
             */
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
            
            cl_uint m_computeUnits{};
            cl_uint m_deviceClockFreq{};
            
            cl_context m_context;
            
            /**
             * prints out the important info about the specified device.
             * @param device device to data dump
             */
            void printDeviceInfo(cl_device_id device);
        
        public:
            /**
             * creates an opencl instance on the specified platform and device. Defaults to the first GPU device
             */
            explicit OpenCL(int platformID = 0, int deviceID = 0);
            
            /**
             * Creates the global OpenCL instance for the engine
             */
            static void init();
            
            /**
             * Creates an OpenCL program object using the global OpenCL connection
             * @param program
             */
            static void createCLProgram(CLProgram& program);
            
            /**
             * @return the number of compute units the device has
             */
            static cl_uint activeDeviceComputeUnits();
            
            /**
             * the frequency in megahertz of the device
             * @return
             */
            static cl_uint activeDeviceFrequency();
            
            ~OpenCL();
    };
    
}

#endif //STEP_3_CL_H
