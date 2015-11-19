#include "shadowapp.h"

ge2::Application *geConstructApplication(int argc, char *argv[])
{
	return (new ShadowApplication(argc, argv));
}
