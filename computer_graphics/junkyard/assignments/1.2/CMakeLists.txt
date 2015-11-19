add_definitions("-D_REENTRANT")

if(APPLE)
    add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
    add_executable(ass12 ass12.cpp)
    target_link_libraries(ass12 glcore32 ${SDL_LIBRARY})
endif()
