#include "compositorapp.h"

ge2::Application *geConstructApplication(int argc, char *argv[])
{
	return (new CompositorApplication(argc, argv));
}
