cmake_minimum_required(VERSION 3.3)
project(chat)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -g") #-std=c99
endif()

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})

pkg_check_modules(SQLITE3 REQUIRED sqlite3)
include_directories(${SQLITE3_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

set(MSC_LIB_SOURCE
    src/msc_client.c
    src/msc_client.h
    src/msc_common.c
    src/msc_common.h
    src/msc_srp_auth.c
    src/msc_srp_auth.h
    src/msc_secure_socket.c
    src/msc_secure_socket.h
    src/msc_library.c
    src/msc_library.h
    src/msc_server.c
    src/msc_server.h
    src/msc_server_connection.c
    src/msc_server_connection.h)

set(CLIENT_SOURCE
    src/chat.c)

set(DAEMON_SOURCE
    src/chatd.c)

add_library(msc STATIC ${MSC_LIB_SOURCE})

add_executable(chat ${CLIENT_SOURCE})
target_link_libraries(chat msc readline ${SQLITE3_LIBRARIES} ${GLIB2_LIBRARIES} ${OPENSSL_LIBRARIES})

add_executable(chatd ${DAEMON_SOURCE})
target_link_libraries(chatd msc ${SQLITE3_LIBRARIES} ${GLIB2_LIBRARIES} ${OPENSSL_LIBRARIES})

add_executable(chatd_user src/chatd_user.c)
target_link_libraries(chatd_user msc ${SQLITE3_LIBRARIES} ${GLIB2_LIBRARIES} ${OPENSSL_LIBRARIES})
