cmake_minimum_required(VERSION 2.8)

#SET( CMAKE_BUILD_TYPE Debug )
set(PROJECT_NAME maintainTool)
project(${PROJECT_NAME})

message(STATUS "Project name:${PROJECT_NAME}\r\n")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRANT")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -g -Wall -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRANT")

set(CMAKE_CXX_FLAGS_DEBUG   "-O0" )             # 调试包不优化
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG ")    # releaseO2优化

file(GLOB CPP_FILES "*.cpp")

foreach(SOURCE ${CPP_FILES})
    string(REPLACE ${PROJECT_SOURCE_DIR} "." CPP_FILE ${SOURCE})
    message(${CPP_FILE})
endforeach()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../include/acl
	${CMAKE_CURRENT_SOURCE_DIR}/../include/glog_linux
	${CMAKE_CURRENT_SOURCE_DIR}/../NetModule)

link_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../lib)

SET(EXECUTABLE_OUTPUT_PATH "../../bin")
add_executable(${PROJECT_NAME} ${CPP_FILES})

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	target_link_libraries(${PROJECT_NAME}
			libglog.a
			libnetcore.a
			libacl_cpp.a
			libacl.a
			pthread
			libuuid.a
			)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin" AND APPLE)
	target_link_libraries(${PROJECT_NAME}
			libnetcore.a
			libglog.a
			libacl_cpp.a
			libacl.a
			pthread
			libiconv.a
			)
ENDIF()