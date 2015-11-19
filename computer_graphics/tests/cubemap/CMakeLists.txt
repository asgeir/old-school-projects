add_definitions("-D_REENTRANT")

if(APPLE)
	add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		cubemapapp.cpp
		cubemapapp.h
		main.cpp)

	add_executable(cubemaptest ${SOURCES})
	target_link_libraries(cubemaptest glengine2 glcore32 ${SDL_LIBRARY})
endif()
