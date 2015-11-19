#include "mazeapplication.h"

ge2::Application *geConstructApplication(int argc, char *argv[])
{
	return (new MazeApplication(argc, argv));
}
