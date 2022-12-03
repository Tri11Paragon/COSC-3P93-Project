/*
 * Created by Brett Terpstra 6920201 on 20/11/22.
 * Copyright (c) 2022 Brett Terpstra. All Rights Reserved.
 */
#include <opencl/cl.h>
#include <engine/util/loaders.h>
#include <config.h>

#ifdef COMPILE_GUI
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#endif

#include <cstddef>
#include <utility>
#include "engine/math/vectors.h"

namespace Raytracing {
    
    std::shared_ptr<OpenCL> openCl;
    
    void OpenCL::init() {
        openCl = std::make_shared<OpenCL>(0, 0);
    }
    
    OpenCL::OpenCL(int platformID, int deviceID):
            m_activePlatform(platformID) {
        m_CL_ERR = CL_SUCCESS;
        m_numPlatforms = 0;
        m_CL_ERR = clGetPlatformIDs(0, NULL, &m_numPlatforms);
        
        if (m_CL_ERR == CL_SUCCESS)
            dlog << "We found " << m_numPlatforms << " OpenCL Platforms.\n";
        else
            elog << "OpenCL Error! " << m_CL_ERR << "\n";
        
        m_platformIDs = new cl_platform_id[m_numPlatforms];
        
        m_CL_ERR = clGetPlatformIDs(m_numPlatforms, m_platformIDs, &m_numOfPlatformIDs);
        m_CL_ERR = clGetDeviceIDs(m_platformIDs[platformID], CL_DEVICE_TYPE_GPU, 1, &m_deviceID, &m_numOfDevices);
        
        printDeviceInfo(m_deviceID);
        
        cl_context_properties proper[3] = {
                CL_CONTEXT_PLATFORM, (cl_context_properties) m_platformIDs[platformID],
                0
        };
        
        m_context = clCreateContext(proper, 1, &m_deviceID, NULL, NULL, &m_CL_ERR);
        
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
        clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &m_computeUnits, NULL);
        clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &m_deviceClockFreq, NULL);
        struct {
            cl_uchar opencl_space[7]; // "OpenCL_"
            cl_uchar major;
            cl_uchar dot;
            cl_uchar minor;
            cl_uchar space;
            cl_uchar vendor[32];
        } dv{};
        clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(dv), &dv, NULL);
        
        dlog << "Opening OpenCL Device!\n";
        dlog << "Device CL String " << dv.opencl_space << dv.major << dv.dot << dv.minor << dv.space << dv.vendor << "\n";
        dlog << "Device Address Bits: " << deviceAddressBits << "\n";
        dlog << "Device is currently " << (deviceAvailable ? "available" : "unavailable") << "\n";
        dlog << "Device has " << cacheSize / 1024 << "kb of cache with a cache line width of " << cacheLineSize << " bytes\n";
        dlog << "Device " << (textureSupport ? "has" : "doesn't have") << " texture support\n";
        dlog << "Device has " << maxWorkgroups << " max workgroup size.\n";
        dlog << "Device has " << m_computeUnits << " compute units running at a max clock frequency " << m_deviceClockFreq << "\n";
        if (!textureSupport)
            elog << "Warning! The OpenCL device lacks texture support!\n";
    }
    
    void OpenCL::createCLProgram(CLProgram& program) {
        program.loadCLShader(openCl->m_context, openCl->m_deviceID);
    }
    
    OpenCL::~OpenCL() {
        delete[](m_platformIDs);
        clReleaseDevice(m_deviceID);
        clReleaseContext(m_context);
    }
    
    cl_uint OpenCL::activeDeviceComputeUnits() {
        return openCl->m_computeUnits;
    }
    
    cl_uint OpenCL::activeDeviceFrequency() {
        return openCl->m_deviceClockFreq;
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
        if (m_CL_ERR != CL_SUCCESS) {
            elog << "Unable to build CL program!\n";
            size_t len;
            
            clGetProgramBuildInfo(m_program, m_deviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
            char buffer[len];
            
            clGetProgramBuildInfo(m_program, m_deviceID, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
            
            elog << buffer << "\n";
            
        }
    }
    
    
    void CLProgram::createBuffer(const std::string& bufferName, cl_mem_flags flags, size_t bytes) {
        // create the buffer on the GPU
        cl_mem buff = clCreateBuffer(m_context, flags, bytes, NULL, &m_CL_ERR);
        // then store it in our buffer map for easy access.
        buffers.insert({bufferName, buff});
    }
    
    
    void CLProgram::createBuffer(const std::string& bufferName, cl_mem_flags flags, size_t bytes, void* ptr) {
        // create the buffer on the GPU
        cl_mem buff = clCreateBuffer(m_context, CL_MEM_COPY_HOST_PTR | flags, bytes, ptr, &m_CL_ERR);
        // then store it in our buffer map for easy access.
        buffers.insert({bufferName, buff});
    }
    
    void CLProgram::createImage(const std::string& imageName, int width, int height) {
        // create the texture on the GPU
        cl_image_format format{CL_RGBA, CL_UNORM_INT8};
        cl_image_desc imageDesc;
        imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
        imageDesc.image_width = width;
        imageDesc.image_height = height;
        imageDesc.image_row_pitch = 0;
        imageDesc.image_slice_pitch = 0;
        imageDesc.num_mip_levels = 0;
        imageDesc.num_samples = 0;
        imageDesc.buffer = NULL;
        cl_mem tex = clCreateImage(m_context, CL_MEM_READ_WRITE, &format, &imageDesc, NULL, &m_CL_ERR);
        if (m_CL_ERR != CL_SUCCESS) {
            elog << "Unable to create image texture!\n";
            checkBasicErrors();
            switch (m_CL_ERR) {
                case CL_INVALID_VALUE:
                    elog << "\tFlags are not valid!\n";
                    // this is straight from the docs
                    elog << "\tOR if a 1D image buffer is being created and the "
                            "buffer object was created with CL_MEM_WRITE_ONLY and flags specifies "
                            "CL_MEM_READ_WRITE or CL_MEM_READ_ONLY, or if the buffer object was created "
                            "with CL_MEM_READ_ONLY and flags specifies CL_MEM_READ_WRITE or CL_MEM_WRITE_ONLY, "
                            "or if flags specifies CL_MEM_USE_HOST_PTR or CL_MEM_ALLOC_HOST_PTR or CL_MEM_COPY_HOST_PTR. \n";
                    elog << "\tOR if a 1D image buffer is being created and the buffer object was created with CL_MEM_HOST_WRITE_ONLY "
                            "and flags specifies CL_MEM_HOST_READ_ONLY, or if the buffer object was created with CL_MEM_HOST_READ_ONLY "
                            "and flags specifies CL_MEM_HOST_WRITE_ONLY, or if the buffer object was created with CL_MEM_HOST_NO_ACCESS "
                            "and flags specifies CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_WRITE_ONLY. \n";
                    break;
                case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
                    elog << "\tImage format isn't valid!\n";
                    break;
                case CL_INVALID_IMAGE_DESCRIPTOR:
                    elog << "\tValues specified in image_desc are not valid or if image_desc is NULL!\n";
                    break;
                case CL_INVALID_IMAGE_SIZE:
                    elog << "\tImage dimensions exceed the minimum maximum image dimensions\n";
                    break;
                case CL_IMAGE_FORMAT_NOT_SUPPORTED:
                    elog << "\tImage format not supported!\n";
                    break;
                case CL_INVALID_OPERATION:
                    elog << "\tImages are not supported on this device!\n";
                    break;
            }
        }
        // then store it in our buffer map for easy access.
        buffers.insert({imageName, tex});
    }
    
    
    void CLProgram::createKernel(const std::string& kernelName) {
        auto kernel = clCreateKernel(m_program, kernelName.c_str(), &m_CL_ERR);
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to create CL kernel " << kernelName << "!\n";
        kernels.insert({kernelName, kernel});
    }
    
    void CLProgram::setKernelArgument(const std::string& kernel, const std::string& buffer, int argIndex) {
        m_CL_ERR = clSetKernelArg(kernels[kernel], argIndex, sizeof(cl_mem), (void*) &buffers[buffer]);
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to bind argument " << buffer << " to CL kernel " << kernel << "!\n";
    }
    
    void CLProgram::writeBuffer(const std::string& buffer, size_t bytes, void* ptr, cl_bool blocking, size_t offset) {
        m_CL_ERR = clEnqueueWriteBuffer(m_commandQueue, buffers[buffer], blocking, offset, bytes, ptr, 0, NULL, NULL);
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to enqueue write to " << buffer << " buffer!\n";
    }
    
    void CLProgram::readBuffer(const std::string& buffer, size_t bytes, void* ptr, cl_bool blocking, size_t offset) {
        m_CL_ERR = clEnqueueReadBuffer(m_commandQueue, buffers[buffer], blocking, offset, bytes, ptr, 0, NULL, NULL);
        if (m_CL_ERR != CL_SUCCESS)
            elog << "Unable to enqueue read from " << buffer << " buffer!\n";
    }
    
    void CLProgram::flushCommands() {
        clFlush(m_commandQueue);
    }
    
    void CLProgram::finishCommands() {
        flushCommands();
        clFinish(m_commandQueue);
    }
    
    void CLProgram::runKernel(const std::string& kernel, size_t globalWorkSize, size_t localWorkSize, const size_t* globalWorkOffset) {
        m_CL_ERR = clEnqueueNDRangeKernel(m_commandQueue, kernels[kernel], 1, globalWorkOffset, &globalWorkSize, &localWorkSize, 0, NULL, NULL);
    }
    
    void
    CLProgram::runKernel(const std::string& kernel, size_t* globalWorkSize, size_t* localWorkSize, cl_uint workDim, const size_t* globalWorkOffset) {
        m_CL_ERR = clEnqueueNDRangeKernel(m_commandQueue, kernels[kernel], workDim, globalWorkOffset, globalWorkSize, localWorkSize, 0, NULL, NULL);
    }
    
    void CLProgram::readImage(const std::string& imageName, size_t width, size_t height, void* ptr, cl_bool blocking, size_t x, size_t y) {
        size_t origin[3]{x, y, 0};
        size_t region[3]{width, height, 1};
        m_CL_ERR = clEnqueueReadImage(m_commandQueue, buffers[imageName], blocking, origin, region, 0, 0, ptr, 0, NULL, NULL);
        if (m_CL_ERR != CL_SUCCESS) {
            elog << "Unable to enqueue read from " << imageName << " image:\n";
            checkBasicErrors();
            switch (m_CL_ERR) {
                case CL_INVALID_MEM_OBJECT:
                    elog << "\tInvalid Memory Object. " << imageName << " isn't a valid memory object (" << buffers[imageName] << ")!\n";
                    break;
                case CL_INVALID_VALUE:
                    elog << "\tOrigin / Region is out of bounds or your pointer is null!\n";
                    break;
            }
        }
    }
    
    CLProgram::~CLProgram() {
        finishCommands();
        for (const auto& kernel: kernels)
            clReleaseKernel(kernel.second);
        clReleaseProgram(m_program);
        for (const auto& buffer: buffers)
            clReleaseMemObject(buffer.second);
        clReleaseCommandQueue(m_commandQueue);
    }
    
    void CLProgram::checkBasicErrors() const {
        switch (m_CL_ERR) {
            case CL_OUT_OF_HOST_MEMORY:
                elog << "\tHost is out of memory!\n";
                break;
            case CL_OUT_OF_RESOURCES:
                elog << "\tDevice is out of resources!\n";
                break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                elog << "\tUnable to allocate memory!\n";
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                elog << "\tInvalid wait list. (Why?)\n";
                break;
            case CL_INVALID_COMMAND_QUEUE:
                elog << "\tInvalid Command Queue!\n";
                break;
            case CL_INVALID_CONTEXT:
                elog << "\tInvalid Context!\n";
                break;
            case CL_INVALID_HOST_PTR:
                elog << "\tHost pointer is null OR flags are incorrectly set and pointer is NULL!\n";
                break;
        }
    }
    
    void CLProgram::readImage(const std::string& imageName, Image& image) {
        // Create an array for copying bytes from the GPU to the CPU with
        unsigned char bytes[image.getWidth() * image.getHeight() * 4];
        for (int i = 0; i < image.getWidth() * image.getHeight() * 4; i++)
            bytes[i] = 0;
        // Call to OpenCL to read the image from the GPU and send it to the location of our buffer
        this->readImage(imageName, image.getWidth(), image.getHeight(), bytes);
        // multiplication is much faster than division,
        // so to speed up the function precalculate the color multiplier used to convert from RGBA8 format to floating point format used by the raytracer.
        const PRECISION_TYPE colorFactor = 1.0 / 255.0;
        // copy the data from the buffer into the image by calculating its byte offset based on its pixel position.
        for (int i = 0; i < image.getWidth(); i++) {
            for (int j = 0; j < image.getHeight(); j++) {
                const auto pixelData = bytes + (j * 4 * image.getWidth() + i * 4);
                image.setPixelColor(i, j, {pixelData[0] * colorFactor, pixelData[1] * colorFactor, pixelData[2] * colorFactor});
            }
        }
    }
    
}