add_definitions("-D_REENTRANT")

if(APPLE)
    add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
    add_executable(persp32 persp_3_2.cpp)
    target_link_libraries(persp32 glcore32 ${SDL_LIBRARY})

    add_executable(persp32glm persp_3_2_glm.cpp)
    target_link_libraries(persp32glm glcore32 ${SDL_LIBRARY})
endif()

if(BUILD_OPENGL_4_3)
    add_executable(persp43 persp_4_3.cpp)
    target_link_libraries(persp43 glcore43 ${SDL_LIBRARY})
endif()
