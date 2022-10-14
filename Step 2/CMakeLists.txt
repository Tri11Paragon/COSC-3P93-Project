cmake_minimum_required(VERSION 3.23)
project(Step_2)

set(CMAKE_CXX_STANDARD 20)

#Setup project source compilation
set(source_dir "${PROJECT_SOURCE_DIR}/src/")
file(GLOB_RECURSE source_files "${source_dir}/*.cpp")
file(GLOB_RECURSE source_c_files "${source_dir}/*.c")

#Setup project header files
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${source_files} ${source_c_files})
