#include "cubemapapp.h"

ge2::Application *geConstructApplication(int argc, char *argv[])
{
	return (new CubemapApplication(argc, argv));
}
