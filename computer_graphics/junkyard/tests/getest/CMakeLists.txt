add_definitions("-D_REENTRANT")

if(APPLE)
	add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		main.cpp)

	add_executable(getest ${SOURCES})
	target_link_libraries(getest glengine glcore32 ${SDL_LIBRARY})
endif()
