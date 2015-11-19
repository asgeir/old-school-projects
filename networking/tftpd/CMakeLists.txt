cmake_minimum_required(VERSION 3.1)
project(tsam_tftpd)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

set(SOURCES
    src/tftpd.c
    src/tftpd_server.c
    src/tftpd_server.h
    src/tftpd_common.h)

add_executable(tftpd ${SOURCES})
