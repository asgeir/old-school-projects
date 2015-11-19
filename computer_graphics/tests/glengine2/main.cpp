#include "testapplication.h"

ge2::Application *geConstructApplication(int argc, char *argv[])
{
	return (new TestApplication(argc, argv));
}
