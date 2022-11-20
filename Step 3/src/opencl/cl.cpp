/*
 * Created by Brett Terpstra 6920201 on 20/11/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <opencl/cl.h>
#include <engine/util/loaders.h>

#include <utility>

namespace Raytracing {
    
    OpenCL openCl {0};
    
    void OpenCL::init() {
        openCl = OpenCL{0};
    }
    OpenCL::OpenCL(int platformID, int deviceID): m_activePlatform(platformID) {
        m_CL_ERR = CL_SUCCESS;
        m_numPlatforms = 0;
        m_CL_ERR = clGetPlatformIDs(0, NULL, &m_numPlatforms );
    
        if (m_CL_ERR == CL_SUCCESS)
            dlog << "We found " << m_numPlatforms << " OpenCL Platforms.\n";
        else
            elog << "OpenCL Error! " << m_CL_ERR << "\n";
    
        m_platformIDs = new cl_platform_id[m_numPlatforms];
        
        m_CL_ERR = clGetPlatformIDs(m_numPlatforms, m_platformIDs, &m_numOfPlatformIDs);
        m_CL_ERR = clGetDeviceIDs(m_platformIDs[platformID], CL_DEVICE_TYPE_GPU, 1, &m_deviceID, &m_numOfDevices);
    
        printDeviceInfo(m_deviceID);
    
        m_context = clCreateContext(NULL, 1, &m_deviceID, NULL, NULL, &m_CL_ERR);
    
        if (m_CL_ERR != CL_SUCCESS)
            elog << "OpenCL Error Creating Context! " << m_CL_ERR << "\n";
        
    }
    
    void OpenCL::printDeviceInfo(cl_device_id device) {
        cl_uint deviceAddressBits;
        clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &deviceAddressBits, NULL);
        cl_bool deviceAvailable;
        clGetDeviceInfo(device, CL_DEVICE_AVAILABLE, sizeof(cl_bool), &deviceAvailable, NULL);
        cl_ulong cacheSize;
        cl_uint cacheLineSize;
        clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &cacheSize, NULL);
        clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cl_uint), &cacheLineSize, NULL);
        cl_bool textureSupport;
        clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &textureSupport, NULL);
        size_t maxWorkgroups;
        clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkgroups, NULL);
        
        dlog << "Opening OpenCL Device!\n";
        dlog << "Device Address Bits: " << deviceAddressBits << "\n";
        dlog << "Device is currently " << (deviceAvailable ? "available" : "unavailable") << "\n";
        dlog << "Device has " << cacheSize/1024 << "kb of cache with a cache line width of " << cacheLineSize << " bytes\n";
        dlog << "Device " << (textureSupport ? "has" : "doesn't have") << " texture support\n";
        dlog << "Device has " << maxWorkgroups << " max workgroups.\n";
        if (!textureSupport)
            elog << "Warning! The OpenCL device lacks texture support!\n";
    }
    void OpenCL::createCLProgram(CLProgram& program) {
        program.loadCLShader(openCl.m_context, openCl.m_deviceID);
    }
    OpenCL::~OpenCL() {
        delete[](m_platformIDs);
        clReleaseDevice(m_deviceID);
        clReleaseContext(m_context);
    }
    
    
    CLProgram::CLProgram(const std::string& file) {
        m_source = ShaderLoader::loadShaderFile(file);
    }
    void CLProgram::loadCLShader(cl_context context, cl_device_id deviceID) {
        this->m_context = context;
        this->m_deviceID = deviceID;
        this->m_commandQueue = clCreateCommandQueueWithProperties(context, deviceID, 0, &m_CL_ERR);
        
        const char* source_cstr = m_source.c_str();
        size_t sourceSize = m_source.length();
        this->m_program = clCreateProgramWithSource(context, 1, &source_cstr, &sourceSize, &m_CL_ERR);
        
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to create CL program!\n";
    
        m_CL_ERR = clBuildProgram(m_program, 1, &deviceID, NULL, NULL, NULL);
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to build CL program!\n";
    }
    /**
     * Buffers are the quintessential datastructures in OpenCL. They are basically regions of memory allocated to a program.
     * @param bufferName the name of the buffer used to store internally
     * @param flags read write flags for the buffer. One of CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE
     * @param bytes the number of bytes to be allocated.
     */
    void CLProgram::createBuffer(const std::string& bufferName, cl_mem_flags flags, size_t bytes) {
        // create the buffer on the GPU
        cl_mem buff = clCreateBuffer(m_context, flags, bytes, NULL, &m_CL_ERR);
        // then store it in our buffer map for easy access.
        buffers.insert({bufferName, buff});
    }
    /**
     * Creates a buffer on the GPU using the data pointed to by the supplied pointer. This copy happens as soon as this is called.
     * @param bufferName the name of the buffer used to store internally
     * @param flags One of CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE
     * @param bytes the number of bytes to be allocated. Must be less than equal to the number of bytes at ptr
     * @param ptr the pointer to copy to the GPU.
     */
    void CLProgram::createBuffer(const std::string& bufferName, cl_mem_flags flags, size_t bytes, void* ptr) {
        // create the buffer on the GPU
        cl_mem buff = clCreateBuffer(m_context, CL_MEM_COPY_HOST_PTR | flags, bytes, ptr, &m_CL_ERR);
        // then store it in our buffer map for easy access.
        buffers.insert({bufferName, buff});
    }
    /**
     * Kernels are the entry points in OpenCL. You can have multiple of them in a single program.
     * @param kernelName both the name of the kernel function in the source and the reference to the kernel object used in other functions in this class.
     */
    void CLProgram::createKernel(const std::string& kernelName) {
        auto kernel = clCreateKernel(m_program, kernelName.c_str(), &m_CL_ERR);
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to create CL kernel" << kernelName << "!\n";
        kernels.insert({kernelName, kernel});
    }
    /**
     * Allows you to bind certain buffers to a specific index in the kernel's argument list.
     * @param kernel kernel to bind to
     * @param buffer buffer to bind to argIndex
     * @param argIndex the index of the argument for this buffer.
     */
    void CLProgram::setKernelArgument(const std::string& kernel, const std::string& buffer, int argIndex) {
        m_CL_ERR = clSetKernelArg(kernels[kernel], argIndex, sizeof(cl_mem), (void*) &buffers[buffer]);
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to bind argument " << buffer << " to CL kernel " << kernel << "!\n";
    }
    /**
     * Enqueues a write command to the buffer specified by the buffer name,
     * @param buffer the buffer to write to
     * @param bytes the number of bytes to be copied
     * @param ptr the pointer to copy from. Must have at least bytes available
     * @param blocking should this function wait for the bytes to be uploaded to the GPU?
     * @param offset offset in the buffer object to write to
     */
    void CLProgram::writeBuffer(const std::string& buffer, size_t bytes, void* ptr, cl_bool blocking, size_t offset) {
        m_CL_ERR = clEnqueueWriteBuffer(m_commandQueue, buffers[buffer], blocking, offset, bytes, ptr, 0, NULL, NULL);
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to enqueue write to " << buffer << " buffer!\n";
    }
    /**
     * Enqueues a read command from the buffered specified by the buffer name.
     * Defaults to blocking but can be set to be non-blocking.
     * @param buffer buffer to read from
     * @param bytes the number of bytes to read. Make sure ptr has at least those bytes available.
     * @param ptr the ptr to write the read bytes to.
     * @param blocking should we wait for the read or do it async?
     * @param offset offset in the buffer to read from.
     */
    void CLProgram::readBuffer(const std::string& buffer, size_t bytes, void* ptr, cl_bool blocking, size_t offset) {
        m_CL_ERR = clEnqueueReadBuffer(m_commandQueue, buffers[buffer], blocking, offset, bytes, ptr, 0, NULL, NULL);
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to enqueue read from " << buffer << " buffer!\n";
    }
    /**
     * Issues all previously queued OpenCL commands in a command-queue to the device associated with the command-queue.
     */
    void CLProgram::flushCommands() {
        clFlush(m_commandQueue);
    }
    /**
     * Blocks until all previously queued OpenCL commands in a command-queue are issued to the associated device and have completed.
     */
    void CLProgram::finishCommands() {
        flushCommands();
        clFinish(m_commandQueue);
    }
    
    CLProgram::~CLProgram() {
        finishCommands();
        for (const auto& kernel : kernels)
            clReleaseKernel(kernel.second);
        clReleaseProgram(m_program);
        for (const auto& buffer : buffers)
            clReleaseMemObject(buffer.second);
        clReleaseCommandQueue(m_commandQueue);
    }
    
    
}