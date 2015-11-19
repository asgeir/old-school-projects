#include "crepuscularapp.h"

ge2::Application *geConstructApplication(int argc, char *argv[])
{
	return (new CrepuscularApplication(argc, argv));
}
