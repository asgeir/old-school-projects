add_definitions("-D_REENTRANT" "-DGLM_FORCE_RADIANS")

if(APPLE)
    add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		ball.cpp
                brick.cpp
		collisionmanager.cpp
		common.cpp
		main.cpp
                paddle.cpp
		state.cpp
		wall.cpp
		game_state.cpp
		end_state.cpp)

    add_executable(breakout ${SOURCES})
    target_link_libraries(breakout glengine glcore32 ${SDL_LIBRARY})
endif()
