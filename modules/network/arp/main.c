#include <fudge.h>
#include <net/arp.h>
#include <kernel.h>
#include <system/system.h>
#include <network/ethernet/ethernet.h>
#include "arp.h"

static struct ethernet_protocol ethernetprotocol;
static struct list hooks;

static void ethernetprotocol_addinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    struct arp_header *header = buffer;
    unsigned short htype;
    unsigned short ptype;
    unsigned short operation;
    struct list_item *current;

    if (count < sizeof (struct arp_header))
        return;

    htype = (header->htype[0] << 8) | header->htype[1];
    ptype = (header->ptype[0] << 8) | header->ptype[1];
    operation = (header->operation[0] << 8) | header->operation[1];

    for (current = hooks.head; current; current = current->next)
    {

        struct arp_hook *hook = current->data;

        if (hook->htype != htype || hook->ptype != ptype)
            continue;

        switch (operation)
        {

        case 1:
        case 2:
            hook->notify(count, buffer);

            break;

        }

    }

    scheduler_mailboxes_send(&ethernetprotocol.data.mailboxes, count, buffer);

}

void arp_registerhook(struct arp_hook *hook)
{

    list_add(&hooks, &hook->item);

}

void arp_unregisterhook(struct arp_hook *hook)
{

    list_remove(&hooks, &hook->item);

}

void arp_inithook(struct arp_hook *hook, unsigned short htype, unsigned short ptype, void (*notify)(unsigned int count, void *buffer))
{

    list_inititem(&hook->item, hook);

    hook->htype = htype;
    hook->ptype = ptype;
    hook->notify = notify;

}

void module_init()
{

    list_init(&hooks);
    ethernet_initprotocol(&ethernetprotocol, "arp", 0x0806, ethernetprotocol_addinterface, ethernetprotocol_notify);

}

void module_register()
{

    ethernet_registerprotocol(&ethernetprotocol);

}

void module_unregister()
{

    ethernet_unregisterprotocol(&ethernetprotocol);

}

