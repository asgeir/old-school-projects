set(SOURCES
	asstool.cpp)

add_executable(asstool ${SOURCES})
target_link_libraries(asstool "-L${PROJECT_SOURCE_DIR}/extern/lib" "-Wl,-rpath,${PROJECT_SOURCE_DIR}/extern/lib" assimp)
