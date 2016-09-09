#include <fudge.h>
#include <net/udp.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/ipv4/ipv4.h>
#include "udp.h"

static struct ipv4_protocol ipv4protocol;
static struct list hooks;

void ipv4protocol_notify(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    struct udp_header *header = buffer;
    unsigned int port = (header->tp[0] << 8) | header->tp[1];
    unsigned int length = (header->length[0] << 8) | header->length[1];
    struct list_item *current;

    for (current = hooks.head; current; current = current->next)
    {

        struct udp_hook *hook = current->data;

        if (hook->port == port)
            hook->notify(hook->self, interface, header + 1, length - sizeof (struct udp_header));

    }

    system_multicast(&ipv4protocol.data, buffer, count);

}

void udp_registerhook(struct udp_hook *hook)
{

    list_add(&hooks, &hook->item);

}

void udp_unregisterhook(struct udp_hook *hook)
{

    list_remove(&hooks, &hook->item);

}

void udp_inithook(struct udp_hook *hook, void *self, unsigned int port, void (*notify)(void *self, struct ethernet_interface *interface, void *buffer, unsigned int count))
{

    list_inititem(&hook->item, hook);

    hook->self = self;
    hook->port = port;
    hook->notify = notify;

}

void module_init(void)
{

    ipv4_initprotocol(&ipv4protocol, "udp", 0x11, ipv4protocol_notify);

}

void module_register(void)
{

    ipv4_registerprotocol(&ipv4protocol);

}

void module_unregister(void)
{

    ipv4_unregisterprotocol(&ipv4protocol);

}

