add_definitions("-D_REENTRANT")

if(APPLE)
    add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
    add_executable(ass11 ass11.cpp)
    target_link_libraries(ass11 glcore32 ${SDL_LIBRARY})
endif()
