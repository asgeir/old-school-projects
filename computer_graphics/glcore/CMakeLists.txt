if(BUILD_OPENGL_3_2)
    add_library(glcore32 gl_core_3_2.c loadshaders_3_2.cpp)
endif()

if(BUILD_OPENGL_4_3)
    add_library(glcore43 gl_core_4_3.c loadshaders_4_3.cpp)
endif()
