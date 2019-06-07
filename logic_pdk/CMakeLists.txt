cmake_minimum_required(VERSION 2.8)

#Set project info
set(PROJECT_NAME pdk)
project(${PROJECT_NAME})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

#Set compiler info
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -shared -Wall -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRANT")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -g -shared -Wall -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRANT")

set(CMAKE_CXX_FLAGS_DEBUG   "-O0" )             # 调试包不优化
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG ")    # release包优化

#add include library path
include_directories(
	../include
	../include/acl
	../include/glog_linux
	../NetModule)

link_directories()

#Get all cpp files
file(GLOB CPP_FILES "*.cpp")


SET(LIBRARY_OUTPUT_PATH "../../bin")
#add_library(${PROJECT_NAME} STATIC ${CPP_FILES})
add_library(${PROJECT_NAME} SHARED ${CPP_FILES})

target_link_libraries(${PROJECT_NAME}

)