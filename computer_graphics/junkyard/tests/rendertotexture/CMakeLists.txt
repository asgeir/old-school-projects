add_definitions("-D_REENTRANT")

if(APPLE)
    add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		common.cpp
		common.h
		framebuffer.cpp
		framebuffer.h
		main.cpp
		quad.cpp
		quad.h
		texture2d.cpp
		texture2d.h
		time.cpp
		time.h)

    add_executable(rendertotexture ${SOURCES})
    target_link_libraries(rendertotexture glcore32 ${SDL_LIBRARY})
endif()
