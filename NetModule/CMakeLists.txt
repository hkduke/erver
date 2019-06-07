cmake_minimum_required(VERSION 2.8)

#Set project info
set(PROJECT_NAME netcore)
project(${PROJECT_NAME})

message("CMAKE_SOURCE_DIR= ${CMAKE_SOURCE_DIR}")
message("CMAKE_BINARY_DIR= ${CMAKE_BINARY_DIR}")

#Set compiler info
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRANT")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -g -Wall -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRANT")

set(CMAKE_CXX_FLAGS_DEBUG   "-O0" )             # 调试包不优化
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG ")    # release包优化

#add include library path
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../include/acl
	${CMAKE_CURRENT_SOURCE_DIR}/../include/glog_linux
	/usr/local/mysql/include)

link_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../lib
	/usr/local/mysql/lib)

#Get all cpp files
file(GLOB CPP_FILES "*.cpp")

SET(LIBRARY_OUTPUT_PATH  "../../lib")
add_library(${PROJECT_NAME} STATIC ${CPP_FILES})

target_link_libraries(${PROJECT_NAME}
libglog.a
libmysqlclient_r.so
libacl.a
libacl_cpp.a
pthread
libuuid.a
)