#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/udp/udp.h>
#include "con.h"

static struct con con;
static struct udp_hook hook;
static struct ipv4_socket socket;

static void *writedata(void *buffer, void *payload, unsigned int count)
{

    unsigned char *data = buffer;

    memory_copy(buffer, payload, count);

    return data + count;

}

static unsigned int con_readctrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, &socket, sizeof (struct ipv4_socket), offset);

}

static unsigned int con_writectrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write(&socket, sizeof (struct ipv4_socket), buffer, count, offset);

}

static unsigned int con_writedata(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct ctrl_conheader *header = buffer;
    unsigned char response[FUDGE_BSIZE];
    unsigned char *c = response;

    c = udp_writehead(c, header->sender.address, header->sender.port, header->target.address, header->target.port, header->count);
    c = writedata(c, header + 1, header->count);

    udp_send(response, c - response);

    return count;

}

static unsigned int hook_match(unsigned int port)
{

    return port == ((socket.port[0] << 8) | socket.port[1]);

}

static void hook_notify(struct ipv4_header *ipv4header, struct udp_header *udpheader, void *buffer, unsigned int count)
{

/*
    struct ctrl_conheader header;

    header.sender.address[0] = ipv4header->sip[0];
    header.sender.address[1] = ipv4header->sip[1];
    header.sender.address[2] = ipv4header->sip[2];
    header.sender.address[3] = ipv4header->sip[3];
    header.sender.port[0] = udpheader->sp[0];
    header.sender.port[1] = udpheader->sp[1];
    header.target.address[0] = ipv4header->tip[0];
    header.target.address[1] = ipv4header->tip[1];
    header.target.address[2] = ipv4header->tip[2];
    header.target.address[3] = ipv4header->tip[3];
    header.target.port[0] = udpheader->tp[0];
    header.target.port[1] = udpheader->tp[1];
    header.count = count;
*/

}

void module_init(void)
{

    con_init(&con);
    udp_inithook(&hook, hook_match, hook_notify);

    con.ctrl.operations.read = con_readctrl;
    con.ctrl.operations.write = con_writectrl;
    con.data.operations.write = con_writedata;

}

void module_register(void)
{

    con_register(&con);
    udp_registerhook(&hook);

}

void module_unregister(void)
{

    con_unregister(&con);
    udp_unregisterhook(&hook);

}

