#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/udp/udp.h>
#include "con.h"

static struct con con;
static struct udp_hook hook;

static void con_open(void)
{

    udp_registerhook(&hook);

}

static void con_close(void)
{

    udp_unregisterhook(&hook);

}

static unsigned int hook_match(unsigned int port)
{

    return port == con.settings.port;

}

static void hook_notify(struct ipv4_header *ipv4header, struct udp_header *header, void *buffer, unsigned int count)
{

    kernel_multicast(&con.links, buffer, count);
    udp_send(ipv4header->sip, header->sp[0], "k", 1);

}

void module_init(void)
{

    con_init(&con, con_open, con_close);
    udp_inithook(&hook, hook_match, hook_notify);

}

void module_register(void)
{

    con_register(&con);

}

void module_unregister(void)
{

    con_unregister(&con);

}

