cmake_minimum_required(VERSION 3.23)
project(Step_3)

set(CMAKE_CXX_STANDARD 20)

# used to debug memory related issues
if ((CMAKE_BUILD_TYPE MATCHES Debug))
    if (UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
    set(COMPILER_DEBUG_ENABLED true)
else()
    set(COMPILER_DEBUG_ENABLED false)
endif()
set(COMPILER_DEBUG_ENABLED_BOOL true)

option(EnableDebugMenu "Enable the debug utils" OFF)

if(EnableDebugMenu MATCHES ON)
    message("debug mode")
    set(DEBUG_ENABLED true)
else()
    message("release mode")
    set(DEBUG_ENABLED false)
endif(EnableDebugMenu MATCHES ON)
set(DEBUG_ENABLED_BOOL true)

#config stuff
configure_file(${PROJECT_SOURCE_DIR}/include/config.h.in config.h @ONLY)
# include the config file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# enables AVX instructions. if you have issues, try disabling this.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

#Setup project source compilation
set(source_dir "${PROJECT_SOURCE_DIR}/src/")
file(GLOB_RECURSE source_files "${source_dir}/*.cpp")
file(GLOB_RECURSE source_c_files "${source_dir}/*.c")

#Setup project header files
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${source_files} ${source_c_files})
