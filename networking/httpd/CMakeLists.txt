cmake_minimum_required(VERSION 3.2)
project(httpd)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -g") #-std=c99
endif()

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})

# pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
# include_directories(${LIBXML2_INCLUDE_DIRS})

set(SOURCE_FILES
    src/main.c
    src/httpd_connection.c
    src/httpd_connection.h
    src/httpd_helpers.c
    src/httpd_helpers.h
    src/httpd_req_handler.c
    src/httpd_req_handler.h
    src/httpd_req_parser.c
    src/httpd_req_parser.h
    src/httpd_server.c
    src/httpd_server.h
    src/httpd_common.h)

add_executable(httpd ${SOURCE_FILES})
target_link_libraries(httpd ${GLIB2_LIBRARIES})
