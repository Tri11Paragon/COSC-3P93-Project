cmake_minimum_required(VERSION 3.22)
project(Step_3)

# I am using GCC-12
set(CMAKE_CXX_STANDARD 20)

# You can ignore this as I am not using it for this project anymore
option(EnableDebugMenu "Enable the debug utils" OFF)
# Enable the GUI via GLFW. This requires libglfw3 and libglfw3-dev installed on your system. (X11 version is here but isn't support. Use at own risk.
# Why?
# I started with X11 because I really didn't want to bring GLFW in. When attempting to set the icon I found it extremely difficult
# I only managed to set the window icon but not the task bar icon and the window icon wasn't loading correct.
# plus debug mode was running at 5 fps which I initially blamed on XOrg (it wasn't X).
# also GLFW has better support as it deals with the quirks of XOrg / the X11 protocol for me.
# GUI isn't a requirement for ANY feature.
option(COMPILE_GUI "Enable compilation of the GUI + utils" OFF)
# Leave this unless you're an extremist who loves living on the edge.
option(USE_GLFW "Will try to use GLFW instead of X11. If libglfw3 and libglfw3-dev are not installed it will default to X11" ON)
# Requires you to have OpenMP installed on your system. The engine will work without this.
option(USE_OPENMP "Will try to use OpenMP. Requires OpenMP on your system." ON)
# Requires you have OpenMPI installed. The engine will work without this.
option(USE_MPI "Will try to use OpenMPI. Requires OpenMPI on your system." ON)
# Requires you to have an OpenCL IDC loader installed and working, should support CL2.0+ and you should also have any relevant dev libs/headers installed.
# Nvidia GPUs should work but I've only tested this on my 6700xt using a driver which was released a year before my card was even announced.
# Thanks AMD for only supporting ubuntu :/ Us debian users left in the dust >;(
# Why OpenCL? Let's go over the GPGPU options
# OpenGL Compute Shaders - Nice option but requires GUI and I don't require much more than what is provided by GLSL
# Vulkan Compute Shaders - Better option but requires GUI and I don't know Vulkan (yet)
# CUDA - Really good for devs with lots of great tools but is platform locked to nvidia. I have a 6700xt and run linux. Neither of these things play well with nvidia.
# OpenACC - No clue what this is
# ROCM - AMD's answer to CUDA. See above about AMD's ability to support things. (Pretty sure windows is completely out of the question here)
# OpenCL - Basically deprecated and treated as a joke. Tooling on AMD sucks so much especially on Linux. But it's the only "well" supported headless GPGPU option on this list.
option(COMPILE_OPENCL "Enable compilation of the OpenCL GPU Compute module." OFF)



#
# DO NOT TOUCH ANYTHING BELOW THIS LINE
# ====================================================================================================================
#




# used to debug memory related issues
if ((CMAKE_BUILD_TYPE MATCHES Debug))
    if (UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif ()
    set(COMPILER_DEBUG_ENABLED true)
    message("Building for Debug")
else ()
    set(COMPILER_DEBUG_ENABLED false)
    message("Building for Release")
endif ()
set(COMPILER_DEBUG_ENABLED_BOOL true)

if (EnableDebugMenu MATCHES ON)
    message("debug mode")
    set(DEBUG_ENABLED true)
else ()
    message("release mode")
    set(DEBUG_ENABLED false)
endif (EnableDebugMenu MATCHES ON)
set(DEBUG_ENABLED_BOOL true)

#config stuff
configure_file(include/engine/config.h.in config.h @ONLY)
# include the config file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# enables AVX instructions. if you have issues, try disabling this.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

#Setup project source compilation
set(engine_source_dir "${PROJECT_SOURCE_DIR}/src/engine")
file(GLOB_RECURSE engine_source_files "${engine_source_dir}/*.cpp" "${engine_source_dir}/*.c")
# only want to attempt to compile graphics if user requests it
# plus we can only compile on X11 supported systems, so basically unix
if (COMPILE_GUI MATCHES ON AND UNIX)
    message("Compiling with GUI.")
    find_package(OpenGL REQUIRED)
    set(SHOULD_FIND_X ON)

    if (USE_GLFW)
        find_package(glfw3 3.3)
        if (glfw3_FOUND)
            message("Found GLFW3!")
            set(SHOULD_FIND_X OFF)
        else ()
            message("Unable to find GLFW3. Please install with sudo apt install libglfw3 libglfw3-dev")
        endif ()
    endif ()

    if (SHOULD_FIND_X MATCHES ON)
        message("Defaulting to X11. Please consider using GLFW as it is consider to be more stable and is what is actively developed. Bugs are to be expected using X11.")
        find_package(X11)
        if (NOT X11_FOUND)
            message("X11 wasn't found on your system. Do you have the development libs? sudo apt install libx11-dev")
        endif ()

        include_directories(${X11_INCLUDE_DIR})
    endif ()

    set(graphics_source_dir "${PROJECT_SOURCE_DIR}/src/graphics")
    file(GLOB_RECURSE graphics_source_files "${graphics_source_dir}/*.cpp" "${graphics_source_dir}/*.c")
endif ()

if (USE_OPENMP)
    find_package(OpenMP)

    if (NOT OpenMP_CXX_FOUND)
        message("OpenMP C++ Requested but was not found on system!")
    endif()

    include_directories(${OpenMP_CXX_INCLUDE_DIRS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if (USE_MPI)
    find_package(MPI)

    if (NOT ${MPI_CCX_FOUND})
        message("MPI Requested but was not found on system!")
    endif()

    include_directories(${MPI_CXX_INCLUDE_DIRS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_OPTIONS}")
endif()

# Windows should be able to handle opencl no problem.
# i decided that i wanted to use opencl due to it having a much nicer c99 language.
if (COMPILE_OPENCL MATCHES ON)
    find_package(OpenCL REQUIRED)

    if (NOT OpenCL_FOUND)
        message("Unable to find OpenCL on your system. Do you have the required libs?")
    endif ()

    include_directories(${OpenCL_INCLUDE_DIRS})

    set(opencl_source_dir "${PROJECT_SOURCE_DIR}/src/opencl")
    file(GLOB_RECURSE opencl_source_files "${opencl_source_dir}/*.cpp" "${opencl_source_dir}/*.c")
endif ()

#Setup project header files
include_directories(${PROJECT_SOURCE_DIR}/include)

#add_subdirectory(test/glm)

add_executable(${PROJECT_NAME} ${engine_source_files} ${graphics_source_files} ${opencl_source_files})

target_link_libraries(${PROJECT_NAME} pthread)
#target_link_libraries(${PROJECT_NAME} glm)

if (COMPILE_GUI MATCHES ON AND UNIX)
    target_link_libraries(${PROJECT_NAME} OpenGL::GL OpenGL::GLU OpenGL::GLX)
    target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES})
endif ()

if (USE_GLFW MATCHES ON AND glfw3_FOUND)
    target_link_libraries(${PROJECT_NAME} glfw)
endif ()

if (COMPILE_OPENCL)
    message("Compiling OpenCL ${OpenCL_LIBRARIES} || ${OpenCL_LIBRARY}")
    target_link_libraries(${PROJECT_NAME} ${OpenCL_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} ${OpenCL_LIBRARY})
    target_compile_definitions(${PROJECT_NAME} PRIVATE CL_TARGET_OPENCL_VERSION=220)
endif ()

if (USE_OPENMP)
    target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_LIBRARIES})
endif()

if (USE_MPI)
    target_link_libraries(${PROJECT_NAME} ${MPI_CXX_LIBRARIES})
endif()
