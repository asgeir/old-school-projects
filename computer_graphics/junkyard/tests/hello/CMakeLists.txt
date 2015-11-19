add_definitions("-D_REENTRANT")

if(APPLE)
    add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
    add_executable(hello32 hello_3_2.cpp)
    target_link_libraries(hello32 glcore32 ${SDL_LIBRARY})
endif()

if(BUILD_OPENGL_4_3)
    add_executable(hello43 hello_4_3.cpp)
    target_link_libraries(hello43 glcore43 ${SDL_LIBRARY})
endif()
