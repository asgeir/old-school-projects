add_definitions("-D_REENTRANT")

if(APPLE)
	add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		compositorapp.cpp
		compositorapp.h
		main.cpp)

	add_executable(compositortest ${SOURCES})
	target_link_libraries(compositortest glengine2 glcore32 ${SDL_LIBRARY})
endif()
