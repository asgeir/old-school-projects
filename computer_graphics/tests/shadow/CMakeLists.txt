add_definitions("-D_REENTRANT")

if(APPLE)
	add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		shadowapp.cpp
		shadowapp.h
		main.cpp)

	add_executable(shadowtest ${SOURCES})
	target_link_libraries(shadowtest glengine2 glcore32 ${SDL_LIBRARY})
endif()
