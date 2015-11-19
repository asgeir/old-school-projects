add_definitions("-D_REENTRANT")

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})
add_definitions(${PNG_DEFINITIONS})

if(APPLE)
	add_definitions("-D_THREAD_SAFE")
endif()

if(BUILD_OPENGL_3_2)
	set(SOURCES
		ge2application.h
		ge2camera.cpp
		ge2camera.h
		ge2common.cpp
		ge2common.h
		ge2compositor.cpp
		ge2compositor.h
		ge2cubeframebuffer.cpp
		ge2cubeframebuffer.h
		ge2cubemap.cpp
		ge2cubemap.h
		ge2debugcamera.cpp
		ge2debugcamera.h
		ge2framebuffer.cpp
		ge2framebuffer.h
		ge2fsquad.cpp
		ge2fsquad.h
		ge2gamestate.cpp
		ge2gamestate.h
		ge2geometry.cpp
		ge2geometry.h
		ge2main.cpp
		ge2material.cpp
		ge2material.h
		ge2mesh.cpp
		ge2mesh.h
		ge2node.cpp
		ge2node.h
		ge2posteffects.cpp
		ge2posteffects.h
		ge2renderer.cpp
		ge2renderer.h
		ge2resourcemgr.cpp
		ge2resourcemgr.h
		ge2shader.cpp
		ge2shader.h
		ge2texture2d.cpp
		ge2texture2d.h
		ge2time.cpp
		ge2time.h)

	add_library(glengine2 STATIC ${SOURCES})
	target_link_libraries(glengine2 ${PNG_LIBRARIES} assimp)
endif()
