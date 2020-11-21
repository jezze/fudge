#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/ipv6/ipv6.h>
#include "udp.h"

static struct ipv4_hook ipv4hook;
static struct ipv6_hook ipv6hook;
static struct list hooks;

static void ipv4hook_notify(struct ipv4_header *ipv4header, void *buffer, unsigned int count)
{

    struct udp_header *header = buffer;
    unsigned int port = (header->tp[0] << 8) | header->tp[1];
    unsigned int length = (header->length[0] << 8) | header->length[1];
    struct list_item *current;

    for (current = hooks.head; current; current = current->next)
    {

        struct udp_hook *hook = current->data;

        if (hook->match(port))
            hook->notify(ipv4header, header, header + 1, length - sizeof (struct udp_header));

    }

    kernel_notify(&ipv4hook.data.links, EVENT_DATA, buffer, count);

}

static void ipv6hook_notify(struct ipv6_header *ipv6header, void *buffer, unsigned int count)
{

    kernel_notify(&ipv6hook.data.links, EVENT_DATA, buffer, count);

}

void *udp_writehead(void *buffer, unsigned char *sip, unsigned char *sp, unsigned char *tip, unsigned char *tp, unsigned int count)
{

    struct udp_header *header = ipv4_writehead(buffer, sip, tip, ipv4hook.id, count + 8);

    if (!header)
        return 0;

    udp_initheader(header, sp, tp, count);

    return header + 1;

}

void udp_send(void *buffer, unsigned int count)
{

    ipv4_send(buffer, count);

}

void udp_registerhook(struct udp_hook *hook)
{

    list_add(&hooks, &hook->item);

}

void udp_unregisterhook(struct udp_hook *hook)
{

    list_remove(&hooks, &hook->item);

}

void udp_inithook(struct udp_hook *hook, unsigned int (*match)(unsigned int port), void (*notify)(struct ipv4_header *ipv4header, struct udp_header *udpheader, void *buffer, unsigned int count))
{

    list_inititem(&hook->item, hook);

    hook->match = match;
    hook->notify = notify;

}

void module_init(void)
{

    list_init(&hooks);
    ipv4_inithook(&ipv4hook, "udp", 0x11, ipv4hook_notify);
    ipv6_inithook(&ipv6hook, "udp", 0x11, ipv6hook_notify);

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

