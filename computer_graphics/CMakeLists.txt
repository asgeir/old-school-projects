cmake_minimum_required(VERSION 2.8)

find_package(OpenGL REQUIRED)
find_package(Threads)

set(OPENGL_VERSION "3.2" CACHE STRING "Maximum OpenGL version to build for")
set_property(CACHE OPENGL_VERSION PROPERTY STRINGS "3.2" "4.3")

set(BUILD_OPENGL_3_2 Off)
set(BUILD_OPENGL_4_3 Off)

if(OPENGL_VERSION VERSION_EQUAL "3.2" OR OPENGL_VERSION VERSION_GREATER "3.2")
    set(BUILD_OPENGL_3_2 On)
endif()

if(OPENGL_VERSION VERSION_EQUAL "4.3" OR OPENGL_VERSION VERSION_GREATER "3.2")
    set(BUILD_OPENGL_4_3 On)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX On CACHE BOOL "Linux system" FORCE)
endif()

if(LINUX)
    find_package(X11 REQUIRED)

    # setup warnings, fatals, and c++11
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror=return-type -std=c++11 -DEIGEN_MPL2_ONLY")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.0")
        add_definitions("-D'alignas(xxXXXxx)=__attribute__((aligned((xxXXXxx))))'")
    endif()
endif()

if(APPLE)
    # setup warnings, fatals, and c++11
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -DEIGEN_MPL2_ONLY")
endif()

add_definitions("-DGLM_FORCE_RADIANS")

add_subdirectory(alglib)
add_subdirectory(glcore)

if(APPLE)
    set(SDL_LIBRARY
        "-L${PROJECT_SOURCE_DIR}/extern/lib"
        "-Wl,-rpath,${PROJECT_SOURCE_DIR}/extern/lib"
        "-framework AudioUnit"
        "-framework CoreAudio"
        "-framework Carbon"
        "-framework ForceFeedback"
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
        "-liconv"
        "-lm"
        "-lSDL2"
        ${OPENGL_gl_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT})
else()
    set(SDL_LIBRARY
        "-L${PROJECT_SOURCE_DIR}/extern/lib"
        "-Wl,-rpath,${PROJECT_SOURCE_DIR}/extern/lib"
        "-lSDL2"
        "-ldl"
        "-lm"
        ${OPENGL_gl_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT})

    include_directories(
        ${OPENGL_INCLUDE_DIR}
        ${X11_INCLUDE_DIR})
endif()

include_directories(
    "${PROJECT_SOURCE_DIR}/alglib/src"
    "${PROJECT_SOURCE_DIR}/glcore"
    "${PROJECT_SOURCE_DIR}/glengine2"
    "${PROJECT_SOURCE_DIR}/extern/include"
    "${PROJECT_SOURCE_DIR}/extern/include/eigen3"
    "${PROJECT_SOURCE_DIR}/extern/include/SDL2")

add_subdirectory(glengine2)
add_subdirectory(assignments)
add_subdirectory(tests)
