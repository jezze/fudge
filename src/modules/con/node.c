#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/udp/udp.h>
#include "con.h"

static struct con con;
static struct udp_hook hook;
static struct ctrl_consettings consettings;

static void *writedata(void *buffer, void *payload, unsigned int count)
{

    unsigned char *data = buffer;

    memory_copy(buffer, payload, count);

    return data + count;

}

static unsigned int con_readctrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, &consettings, sizeof (struct ctrl_consettings), offset);

}

static unsigned int con_writectrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write(&consettings, sizeof (struct ctrl_consettings), buffer, count, offset);

}

static struct system_node *con_opendata(struct system_node *self, struct service_state *state)
{

    list_add(&self->states, &state->item);
    udp_registerhook(&hook);

    return self;

}

static struct system_node *con_closedata(struct system_node *self, struct service_state *state)
{

    list_remove(&self->states, &state->item);
    udp_unregisterhook(&hook);

    return self;

}

static unsigned int con_writedata(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct ipv4_pair *pair = buffer;
    unsigned char response[FUDGE_BSIZE];
    unsigned char *c = response;

    c = udp_writehead(c, pair->sender.address, pair->sender.port, pair->target.address, pair->target.port, pair->count);
    c = writedata(c, pair + 1, pair->count);

    udp_send(response, c - response);

    return count;

}

static unsigned int hook_match(unsigned int port)
{

    return port == consettings.port;

}

static void hook_notify(struct ipv4_header *ipv4header, struct udp_header *udpheader, void *buffer, unsigned int count)
{

    struct ipv4_pair pair;

    pair.sender.address[0] = ipv4header->sip[0];
    pair.sender.address[1] = ipv4header->sip[1];
    pair.sender.address[2] = ipv4header->sip[2];
    pair.sender.address[3] = ipv4header->sip[3];
    pair.sender.port[0] = udpheader->sp[0];
    pair.sender.port[1] = udpheader->sp[1];
    pair.target.address[0] = ipv4header->tip[0];
    pair.target.address[1] = ipv4header->tip[1];
    pair.target.address[2] = ipv4header->tip[2];
    pair.target.address[3] = ipv4header->tip[3];
    pair.target.port[0] = udpheader->tp[0];
    pair.target.port[1] = udpheader->tp[1];
    pair.count = count;

    kernel_multicast(&con.data.states, &pair, sizeof (struct ipv4_pair));
    kernel_multicast(&con.data.states, buffer, count);

}

void module_init(void)
{

    con_init(&con);
    udp_inithook(&hook, hook_match, hook_notify);

    con.ctrl.operations.read = con_readctrl;
    con.ctrl.operations.write = con_writectrl;
    con.data.operations.open = con_opendata;
    con.data.operations.close = con_closedata;
    con.data.operations.write = con_writedata;

}

void module_register(void)
{

    con_register(&con);

}

void module_unregister(void)
{

    con_unregister(&con);

}

