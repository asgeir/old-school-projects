add_definitions("-D_REENTRANT")

if(APPLE)
    add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
    add_executable(ass13 ass13.cpp)
    target_link_libraries(ass13 glcore32 ${SDL_LIBRARY})
endif()
