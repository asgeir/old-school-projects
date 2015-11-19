add_definitions("-D_REENTRANT")

if(APPLE)
	add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
                gecamera.cpp
                gecamera.h
		gecommon.cpp
		gecommon.h
		geframebuffer.cpp
		geframebuffer.h
		gemain.cpp
                genode.cpp
                genode.h
		gequad.cpp
		gequad.h
		gerenderer.cpp
		gerenderer.h
		gerenderstate.cpp
		gerenderstate.h
		geshader.cpp
		geshader.h
		geshaderprogram.cpp
		geshaderprogram.h
		getexture2d.cpp
		getexture2d.h
		getime.cpp
		getime.h)

	add_library(glengine STATIC ${SOURCES})
endif()
