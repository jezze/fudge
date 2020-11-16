#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/ipv6/ipv6.h>
#include "tcp.h"

static struct ipv4_hook ipv4hook;
static struct ipv6_hook ipv6hook;
static struct list hooks;

static void ipv4hook_notify(struct ipv4_header *ipv4header, void *buffer, unsigned int count)
{

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

    kernel_notify(&ipv4hook.data.links, EVENT_DATA, buffer, count);

}

static void ipv6hook_notify(struct ipv6_header *ipv6header, void *buffer, unsigned int count)
{

    kernel_notify(&ipv6hook.data.links, EVENT_DATA, buffer, count);

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

    list_init(&hooks);
    ipv4_inithook(&ipv4hook, "tcp", 0x06, ipv4hook_notify);
    ipv6_inithook(&ipv6hook, "tcp", 0x06, ipv6hook_notify);

}

void module_register(void)
{

    ipv4_registerhook(&ipv4hook);
    ipv6_registerhook(&ipv6hook);

}

void module_unregister(void)
{

    ipv4_unregisterhook(&ipv4hook);
    ipv6_unregisterhook(&ipv6hook);

}

