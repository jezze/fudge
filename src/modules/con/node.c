#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/udp/udp.h>
#include "con.h"

static struct con con;
static struct udp_hook hook;
static struct ipv4_header *tempipv4header;
static struct udp_header *tempudpheader;

static void *writedata(void *buffer, void *payload, unsigned int count)
{

    unsigned char *data = buffer;

    memory_copy(buffer, payload, count);

    return data + count;

}

static void send(unsigned char *sip, unsigned char *sp, unsigned char *tip, unsigned char *tp, void *buffer, unsigned int count)
{

    unsigned char response[FUDGE_BSIZE];
    unsigned char *current = response;

    current = udp_writehead(current, sip, sp, tip, tp, count);
    current = writedata(current, buffer, count);

    udp_send(response, current - response);

}

static void con_open(void)
{

    udp_registerhook(&hook);

}

static void con_close(void)
{

    udp_unregisterhook(&hook);

}

static unsigned int con_write(void *buffer, unsigned int count)
{

    send(tempipv4header->tip, tempudpheader->tp, tempipv4header->sip, tempudpheader->sp, buffer, count);

    return count;

}

static unsigned int hook_match(unsigned int port)
{

    return port == con.settings.port;

}

static void hook_notify(struct ipv4_header *ipv4header, struct udp_header *udpheader, void *buffer, unsigned int count)
{

    tempipv4header = ipv4header;
    tempudpheader = udpheader;

    kernel_multicast(&con.links, buffer, count);

}

void module_init(void)
{

    con_init(&con, con_open, con_close, con_write);
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

