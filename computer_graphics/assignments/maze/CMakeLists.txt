add_definitions("-D_REENTRANT")

if(APPLE)
	add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		gamecamera.cpp
		gamecamera.h
		gamestate.cpp
		gamestate.h
		main.cpp
		mazeapplication.cpp
		mazeapplication.h
		winstate.cpp
		winstate.h)

	add_executable(maze ${SOURCES})
	target_link_libraries(maze glengine2 glcore32 ${SDL_LIBRARY})
endif()
