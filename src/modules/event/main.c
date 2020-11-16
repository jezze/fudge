#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node wserver;

void module_init(void)
{

    system_initnode(&wserver, SYSTEM_NODETYPE_NORMAL, "wserver");

}

void module_register(void)
{

    system_registernode(&wserver);

}

void module_unregister(void)
{

    system_unregisternode(&wserver);

}

