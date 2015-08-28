# 
# Author: <gfxcc_stevens@outlook.com>
# created: 2014-08-27


PROJECT(iShare)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

MESSAGE(STATUS "Project: ${PROJECT_NAME}")
MESSAGE(STATUS "Project Directory: ${PORJECT_SOURCE_DIR}")
message(status ${source})

file(GLOB_RECURSE header ${iShare_SOURCE_DIR}/*.h)
file(GLOB_RECURSE source ${iShare_SOURCE_DIR}/*.cc)

SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "-std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

include_directories(
    /usr/local/mysql/include/
    /usr/local/mysql/lib/
    /usr/include/mysql
)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
/lib
/usr/lib
/usr/lib64
/usr/local/lib
/usr/local/lib64
/usr/lib/x86_64-linux-gnu
/usr/local/mysql-5.6.25-osx10.8-x86_64/lib/
/usr/lib/x86_64-linux-gnu)


add_executable(iShare ${source})

find_library(MYSQL_CLIENT NAMES libmysqlclient.a)

if(MYSQL_CLIENT)
    message(STATUS ${MYSQL_CLIENT})
target_link_libraries(iShare
    grpc++_unsecure
    protobuf
    grpc
    gpr
    pthread
    ${MYSQL_CLIENT}     
    dl
    z
    )
else()
message(FATAL_ERROR "FATAL ERROR: missing library")
endif()
