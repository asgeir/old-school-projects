add_definitions("-D_REENTRANT")

if(APPLE)
    add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		common.cpp
		common.h
		main.cpp
		quad.cpp
		quad.h
		texture2d.cpp
		texture2d.h)

    add_executable(texture ${SOURCES})
    target_link_libraries(texture glcore32 ${SDL_LIBRARY})
endif()
