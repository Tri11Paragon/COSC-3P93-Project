cmake_minimum_required(VERSION 3.23)
project(Step_2)

set(CMAKE_CXX_STANDARD 20)

if (UNIX AND (CMAKE_BUILD_TYPE MATCHES Debug))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

#Setup project source compilation
set(source_dir "${PROJECT_SOURCE_DIR}/src/")
file(GLOB_RECURSE source_files "${source_dir}/*.cpp")
file(GLOB_RECURSE source_c_files "${source_dir}/*.c")

#Setup project header files
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${source_files} ${source_c_files})
