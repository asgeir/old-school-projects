#include "msc_library.h"
#include "msc_secure_socket.h"

static MSC_BOOL g_initialized = MSC_FALSE;

MSC_BOOL msc_library_init(MSC_LIBRARY_MODE mode)
{
	if (!msc_secure_socket_library_init(mode)) return MSC_FALSE;

	g_initialized = MSC_TRUE;
	return 1;
}

void msc_library_finalize()
{
	msc_secure_socket_library_finalize();
	g_initialized = MSC_FALSE;
}
