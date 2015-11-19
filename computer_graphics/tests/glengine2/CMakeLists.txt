add_definitions("-D_REENTRANT")

if(APPLE)
	add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		main.cpp
		testapplication.cpp
		testapplication.h)

	add_executable(ge2test ${SOURCES})
	target_link_libraries(ge2test glengine2 glcore32 ${SDL_LIBRARY})
endif()
