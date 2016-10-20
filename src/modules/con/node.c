#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/udp/udp.h>
#include "con.h"

static struct con con;
static struct udp_hook hook;

static void con_configure(unsigned int port)
{

    hook.port = port;

}

static void con_open()
{

    udp_registerhook(&hook);

}

static void con_close()
{

    udp_unregisterhook(&hook);

}

static void hook_notify(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    system_multicast(&con.data, buffer, count);

}

void module_init(void)
{

    con_init(&con, con_configure, con_open, con_close);
    udp_inithook(&hook, 0, hook_notify);

}

void module_register(void)
{

    con_register(&con);

}

void module_unregister(void)
{

    con_unregister(&con);

}

