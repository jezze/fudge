#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node wm;

void module_init(void)
{

    system_initnode(&wm, SYSTEM_NODETYPE_NORMAL, "wm");

}

void module_register(void)
{

    system_registernode(&wm);

}

void module_unregister(void)
{

    system_unregisternode(&wm);

}

