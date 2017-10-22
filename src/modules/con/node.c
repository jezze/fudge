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

    struct udp_session *session = &udptable[0];
    unsigned char response[FUDGE_BSIZE];
    unsigned char *current = response;

    current = udp_writehead(current, session->sip, session->sp, session->tip, session->tp, count);
    current = writedata(current, buffer, count);

    udp_send(response, current - response);

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

    kernel_multicast(&con.states, &con.statelock, buffer, count);

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

    con_init(&con, con_open, con_close, con_write);
    udp_inithook(&hook, hook_match, hook_notify);
    system_initnode(&udptablenode, SYSTEM_NODETYPE_NORMAL, "udptable");

    udptablenode.read = udptablenode_read;
    udptablenode.write = udptablenode_write;

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

