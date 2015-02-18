#include <fudge.h>
#include <net/arp.h>
#include <kernel.h>
#include <system/system.h>
#include <network/ethernet/ethernet.h>
#include "arp.h"

static struct ethernet_protocol protocol;
static struct list hooks;

void protocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    struct arp_message *message = buffer;
    unsigned short ptype;
    struct list_item *current;

    if (count < sizeof (struct arp_message))
        return;

    ptype = (message->ptype[0] << 8) | message->ptype[1];

    for (current = hooks.head; current; current = current->next)
    {

        struct arp_hook *hook = current->data;

        if (hook->ptype == ptype)
            hook->notify(interface, message);

    }

    scheduler_mailboxes_send(&protocol.data.mailboxes, count, buffer);

}

void arp_registerhook(struct arp_hook *hook)
{

    list_add(&hooks, &hook->item);

}

void arp_unregisterhook(struct arp_hook *hook)
{

    list_remove(&hooks, &hook->item);

}

void arp_inithook(struct arp_hook *hook, unsigned short ptype, void (*notify)(struct ethernet_interface *interface, struct arp_message *message))
{

    list_inititem(&hook->item, hook);

    hook->ptype = ptype;
    hook->notify = notify;

}

void module_init()
{

    list_init(&hooks);
    ethernet_initprotocol(&protocol, "arp", 0x0806, protocol_notify);

}

void module_register()
{

    ethernet_registerprotocol(&protocol);

}

void module_unregister()
{

    ethernet_unregisterprotocol(&protocol);

}

