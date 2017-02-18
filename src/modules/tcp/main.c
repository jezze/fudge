#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/ipv4/ipv4.h>
#include "tcp.h"

static struct ipv4_protocol ipv4protocol;
static struct list hooks;

void ipv4protocol_notify(struct ethernet_interface *interface, struct ipv4_header *ipv4header, void *buffer, unsigned int count)
{

    struct tcp_header *header = buffer;
    unsigned int port = (header->tp[0] << 8) | header->tp[1];
    unsigned int length = 0;
    struct list_item *current;

    for (current = hooks.head; current; current = current->next)
    {

        struct tcp_hook *hook = current->data;

        if (hook->match(port))
            hook->notify(interface, header + 1, length - sizeof (struct tcp_header));

    }

    kernel_multicast(&ipv4protocol.datalinks, buffer, count);

}

void tcp_registerhook(struct tcp_hook *hook)
{

    list_add(&hooks, &hook->item);

}

void tcp_unregisterhook(struct tcp_hook *hook)
{

    list_remove(&hooks, &hook->item);

}

void tcp_inithook(struct tcp_hook *hook, unsigned int (*match)(unsigned int port), void (*notify)(struct ethernet_interface *interface, void *buffer, unsigned int count))
{

    list_inititem(&hook->item, hook);

    hook->match = match;
    hook->notify = notify;

}

void module_init(void)
{

    ipv4_initprotocol(&ipv4protocol, "tcp", 0x06, ipv4protocol_notify);

}

void module_register(void)
{

    ipv4_registerprotocol(&ipv4protocol);

}

void module_unregister(void)
{

    ipv4_unregisterprotocol(&ipv4protocol);

}

