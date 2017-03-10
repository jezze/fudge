#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include "udp.h"

static struct ipv4_protocol ipv4protocol;
static struct list hooks;

void ipv4protocol_notify(struct ipv4_header *ipv4header, void *buffer, unsigned int count)
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

    kernel_multicast(&ipv4protocol.datalinks, buffer, count);

}

void *udp_writeheader(void *buffer, unsigned char *tip, unsigned char *tp, unsigned char *sp, unsigned int count)
{

    struct udp_header *header = buffer;

    count += 8;
    header->sp[0] = sp[0];
    header->sp[1] = sp[1];
    header->tp[0] = tp[0];
    header->tp[1] = tp[1];
    header->length[0] = count >> 8;
    header->length[1] = count;
    header->checksum[0] = 0;
    header->checksum[1] = 0;

    return header + 1;

}

void udp_send(unsigned char *tip, unsigned char *tp, unsigned char *sp, void *payload, unsigned int count)
{

    unsigned char data[FUDGE_BSIZE];

    udp_writeheader(data, tip, tp, sp, count);
    memory_copy(data + 8, payload, count);
    ipv4_send(tip, ipv4protocol.id, data, 8 + count);

}

void udp_registerhook(struct udp_hook *hook)
{

    list_add(&hooks, &hook->item);

}

void udp_unregisterhook(struct udp_hook *hook)
{

    list_remove(&hooks, &hook->item);

}

void udp_inithook(struct udp_hook *hook, unsigned int (*match)(unsigned int port), void (*notify)(struct ipv4_header *ipv4header, struct udp_header *header, void *buffer, unsigned int count))
{

    list_inititem(&hook->item, hook);

    hook->match = match;
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

