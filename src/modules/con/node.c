#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/udp/udp.h>
#include "con.h"

#define UDPTABLESIZE                    8

static struct con con;
static struct udp_hook hook;
static struct udp_session udptable[UDPTABLESIZE];
static struct system_node udptablenode;

static void *writedata(void *buffer, void *payload, unsigned int count)
{

    unsigned char *data = buffer;

    memory_copy(buffer, payload, count);

    return data + count;

}

static unsigned int con_readctrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, &con.settings, sizeof (struct ctrl_consettings), offset);

}

static unsigned int con_writectrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write(&con.settings, sizeof (struct ctrl_consettings), buffer, count, offset);

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

    struct udp_session *session = &udptable[0];
    unsigned char response[FUDGE_BSIZE];
    unsigned char *c = response;

    c = udp_writehead(c, session->sip, session->sp, session->tip, session->tp, count);
    c = writedata(c, buffer, count);

    udp_send(response, c - response);

    return count;

}

static unsigned int hook_match(unsigned int port)
{

    return port == con.settings.port;

}

static void hook_notify(struct ipv4_header *ipv4header, struct udp_header *udpheader, void *buffer, unsigned int count)
{

    struct udp_session *session = &udptable[0];

    session->sip[0] = ipv4header->tip[0];
    session->sip[1] = ipv4header->tip[1];
    session->sip[2] = ipv4header->tip[2];
    session->sip[3] = ipv4header->tip[3];
    session->tip[0] = ipv4header->sip[0];
    session->tip[1] = ipv4header->sip[1];
    session->tip[2] = ipv4header->sip[2];
    session->tip[3] = ipv4header->sip[3];
    session->sp[0] = udpheader->tp[0];
    session->sp[1] = udpheader->tp[1];
    session->tp[0] = udpheader->sp[0];
    session->tp[1] = udpheader->sp[1];

    kernel_multicast(&con.data.states, buffer, count);

}

static unsigned int udptablenode_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, udptable, sizeof (struct udp_session) * UDPTABLESIZE, offset);

}

static unsigned int udptablenode_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write(udptable, sizeof (struct udp_session) * UDPTABLESIZE, buffer, count, offset);

}

void module_init(void)
{

    con_init(&con);
    udp_inithook(&hook, hook_match, hook_notify);
    system_initnode(&udptablenode, SYSTEM_NODETYPE_NORMAL, "udptable");

    con.ctrl.operations.read = con_readctrl;
    con.ctrl.operations.write = con_writectrl;
    con.data.operations.open = con_opendata;
    con.data.operations.close = con_closedata;
    con.data.operations.write = con_writedata;
    udptablenode.operations.read = udptablenode_read;
    udptablenode.operations.write = udptablenode_write;

}

void module_register(void)
{

    con_register(&con);
    system_addchild(&con.root, &udptablenode);

}

void module_unregister(void)
{

    con_unregister(&con);
    system_removechild(&con.root, &udptablenode);

}

