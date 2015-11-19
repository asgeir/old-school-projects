add_definitions("-D_REENTRANT")

if(APPLE)
	add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		crepuscularapp.cpp
		crepuscularapp.h
		main.cpp)

	add_executable(crepusculartest ${SOURCES})
	target_link_libraries(crepusculartest glengine2 glcore32 ${SDL_LIBRARY})
endif()
