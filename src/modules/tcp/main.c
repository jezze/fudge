#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/ipv6/ipv6.h>
#include "tcp.h"

static struct ipv4_protocol ipv4protocol;
static struct ipv6_protocol ipv6protocol;
static struct list hooks;

static void ipv4protocol_notify(struct ipv4_header *ipv4header, void *buffer, unsigned int count)
{

    union event_message message;
    struct tcp_header *header = buffer;
    unsigned int port = (header->tp[0] << 8) | header->tp[1];
    unsigned int length = ((ipv4header->length[0] << 8) | ipv4header->length[1]) - (sizeof (struct ipv4_header) + sizeof (struct tcp_header));
    struct list_item *current;

    for (current = hooks.head; current; current = current->next)
    {

        struct tcp_hook *hook = current->data;

        if (hook->match(port))
            hook->notify(header, header + 1, length - sizeof (struct tcp_header));

    }

    event_createdata(&message.header, EVENT_BROADCAST, 0);
    event_appenddata(&message.header, count, buffer);
    kernel_multicast(&ipv4protocol.data.states, &message.header, message.header.length);

}

static void ipv6protocol_notify(struct ipv6_header *ipv6header, void *buffer, unsigned int count)
{

    union event_message message;

    event_createdata(&message.header, EVENT_BROADCAST, 0);
    event_appenddata(&message.header, count, buffer);
    kernel_multicast(&ipv6protocol.data.states, &message.header, message.header.length);

}

void tcp_registerhook(struct tcp_hook *hook)
{

    list_add(&hooks, &hook->item);

}

void tcp_unregisterhook(struct tcp_hook *hook)
{

    list_remove(&hooks, &hook->item);

}

void tcp_inithook(struct tcp_hook *hook, unsigned int (*match)(unsigned int port), void (*notify)(struct tcp_header *header, void *buffer, unsigned int count))
{

    list_inititem(&hook->item, hook);

    hook->match = match;
    hook->notify = notify;

}

void module_init(void)
{

    ipv4_initprotocol(&ipv4protocol, "tcp", 0x06, ipv4protocol_notify);
    ipv6_initprotocol(&ipv6protocol, "tcp", 0x06, ipv6protocol_notify);

}

void module_register(void)
{

    ipv4_registerprotocol(&ipv4protocol);
    ipv6_registerprotocol(&ipv6protocol);

}

void module_unregister(void)
{

    ipv4_unregisterprotocol(&ipv4protocol);
    ipv6_unregisterprotocol(&ipv6protocol);

}

