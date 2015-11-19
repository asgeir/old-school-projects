include_directories("src")

set(SOURCES
	src/alglibinternal.cpp
	src/alglibinternal.h
	src/alglibmisc.cpp
	src/alglibmisc.h
	src/ap.cpp
	src/ap.h
	src/dataanalysis.cpp
	src/dataanalysis.h
	src/diffequations.cpp
	src/diffequations.h
	src/fasttransforms.cpp
	src/fasttransforms.h
	src/integration.cpp
	src/integration.h
	src/interpolation.cpp
	src/interpolation.h
	src/linalg.cpp
	src/linalg.h
	src/optimization.cpp
	src/optimization.h
	src/solvers.cpp
	src/solvers.h
	src/specialfunctions.cpp
	src/specialfunctions.h
	src/statistics.cpp
	src/statistics.h
	src/stdafx.h)

add_library(alglib STATIC ${SOURCES})
