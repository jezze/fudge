#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/ipv4/ipv4.h>
#include <modules/udp/udp.h>
#include "con.h"

static struct con con;

static void conhook_notify(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    system_multicast(&con.data, buffer, count);

}

void module_init(void)
{

    con_init(&con);
    udp_inithook(&con.hook, 0, conhook_notify);

}

void module_register(void)
{

    con_register(&con);

}

void module_unregister(void)
{

    con_unregister(&con);

}

