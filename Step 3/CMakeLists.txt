cmake_minimum_required(VERSION 3.22)
project(Step_3)

set(CMAKE_CXX_STANDARD 20)

option(EnableDebugMenu "Enable the debug utils" OFF)
option(COMPILE_GUI "Enable compilation of the GUI + utils" OFF)
option(USE_GLFW "Will try to use GLFW instead of X11. If libglfw3 and libglfw3-dev are not installed it will default to X11" ON)
option(USE_OPENMP "Will try to use OpenMP over std::thread. Requires OpenMP on your system." ON)
option(USE_MPI "Will try to use OpenMPI over OpenMP or std::thread. Requires OpenMPI on your system." ON)
option(COMPILE_OPENCL "Enable compilation of the OpenCL GPU Compute." OFF)

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
