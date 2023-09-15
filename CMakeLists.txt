cmake_minimum_required(VERSION 3.2)

project(WebServer CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
    -D_PTHREADS
    -Wno-unused-parameter
)


set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")


string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})



#头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/WebServer/include)
include_directories(${PROJECT_SOURCE_DIR}/WebServer/base/include)

#注意要先设置，然后再添加子文件夹
# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out_bin)
# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out_lib)

add_subdirectory(WebServer)

message(STATUS "PROJECT_SOURCE_DIR is ${PROJECT_SOURCE_DIR}")

