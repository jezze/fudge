#include <fudge.h>
#include <kernel.h>

static struct debug_interface debuginterface;

static void debuginterface_write(unsigned int level, unsigned int count, char *string, char *file, unsigned int line)
{

}

void module_init(void)
{

    debug_initinterface(&debuginterface, debuginterface_write);

}

void module_register(void)
{

    debug_registerinterface(&debuginterface);

}

void module_unregister(void)
{

    debug_unregisterinterface(&debuginterface);

}

