#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include "arp.h"

static struct ethernet_protocol ethernetprotocol;
static struct list hooks;

unsigned int arp_writeheader(unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength, unsigned short operation, unsigned char *sha, unsigned char *shp, unsigned char *tha, unsigned char *thp, void *buffer)
{

    struct arp_header *header = buffer;
    unsigned char *data = (unsigned char *)(header + 1);

    header->htype[0] = htype >> 8;
    header->htype[1] = htype;
    header->ptype[0] = ptype >> 8;
    header->ptype[1] = ptype;
    header->hlength = hlength;
    header->plength = plength;
    header->operation[0] = operation >> 8;
    header->operation[1] = operation;

    memory_copy(data, sha, hlength);
    memory_copy(data + hlength, shp, plength);
    memory_copy(data + hlength + plength, tha, hlength);
    memory_copy(data + hlength + plength + hlength, thp, plength);

    return sizeof (struct arp_header) + hlength + plength + hlength + plength;

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, struct ethernet_header *ethernetheader, void *buffer, unsigned int count)
{

    struct arp_header *header = buffer;
    unsigned char *data = (unsigned char *)(header + 1);
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
        unsigned char response[ETHERNET_MTU];
        unsigned char *haddress;
        unsigned int c = 0;

        if (!hook->match(htype, header->hlength, ptype, header->plength))
            continue;

        switch (operation)
        {

        case ARP_REQUEST:
            haddress = hook->lookup(data + header->hlength + header->plength + header->hlength);

            if (!haddress)
                continue;

            c += ethernet_writeheader(&ethernetprotocol, interface->haddress, data, response);
            c += arp_writeheader(htype, header->hlength, ptype, header->plength, ARP_REPLY, haddress, data + header->hlength + header->plength + header->hlength, data, data + header->hlength, response + c);

            interface->send(response, c);

            break;

        }

    }

    kernel_multicast(&ethernetprotocol.datalinks, buffer, count);

}

void arp_registerhook(struct arp_hook *hook)
{

    list_add(&hooks, &hook->item);

}

void arp_unregisterhook(struct arp_hook *hook)
{

    list_remove(&hooks, &hook->item);

}

void arp_inithook(struct arp_hook *hook, unsigned int (*match)(unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength), unsigned char *(*lookup)(void *paddress))
{

    list_inititem(&hook->item, hook);

    hook->match = match;
    hook->lookup = lookup;

}

void module_init(void)
{

    ethernet_initprotocol(&ethernetprotocol, "arp", ARP_PROTOCOL, ethernetprotocol_notify);

}

void module_register(void)
{

    ethernet_registerprotocol(&ethernetprotocol);

}

void module_unregister(void)
{

    ethernet_unregisterprotocol(&ethernetprotocol);

}

