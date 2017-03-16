#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include "arp.h"

static struct ethernet_protocol ethernetprotocol;
static struct list hooks;

void *arp_writeheader(void *buffer, unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength, unsigned int operation)
{

    struct arp_header *header = buffer;

    header->htype[0] = htype >> 8;
    header->htype[1] = htype;
    header->ptype[0] = ptype >> 8;
    header->ptype[1] = ptype;
    header->hlength = hlength;
    header->plength = plength;
    header->operation[0] = operation >> 8;
    header->operation[1] = operation;

    return header + 1;

}

void *arp_writedata(void *buffer, struct arp_header *header, unsigned char *sha, unsigned char *spa, unsigned char *tha, unsigned char *tpa)
{

    unsigned char *data = buffer;

    memory_copy(data, sha, header->hlength);
    memory_copy(data + header->hlength, spa, header->plength);
    memory_copy(data + header->hlength + header->plength, tha, header->hlength);
    memory_copy(data + header->hlength + header->plength + header->hlength, tpa, header->plength);

    return data + header->hlength + header->plength + header->hlength + header->plength;

}

static struct arp_hook *findhook(unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned int plength)
{

    struct list_item *current;

    for (current = hooks.head; current; current = current->next)
    {

        struct arp_hook *hook = current->data;

        if (hook->match(htype, hlength, ptype, plength))
            return hook;

    }

    return 0;

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, struct ethernet_header *ethernetheader, void *buffer, unsigned int count)
{

    struct arp_header *header = buffer;
    unsigned int htype = (header->htype[0] << 8) | header->htype[1];
    unsigned int ptype = (header->ptype[0] << 8) | header->ptype[1];
    unsigned int operation = (header->operation[0] << 8) | header->operation[1];
    struct arp_hook *hook = findhook(htype, header->hlength, ptype, header->plength);

    if (hook)
    {

        unsigned char *data = (unsigned char *)(header + 1);
        unsigned char *sha = data;
        unsigned char *spa = data + header->hlength;
        unsigned char *tha = data + header->hlength + header->plength;
        unsigned char *tpa = data + header->hlength + header->plength + header->hlength;

        switch (operation)
        {

        case ARP_REQUEST:
            hook->save(sha, spa);

            tha = hook->lookup(tpa);

            if (tha)
            {

                unsigned char response[ETHERNET_MTU];
                struct arp_header *arpheader = ethernet_writehead(response, ethernetprotocol.type, interface->haddress, sha);
                unsigned char *arpdata = arp_writeheader(arpheader, htype, header->hlength, ptype, header->plength, ARP_REPLY);
                unsigned char *responsetotal = arp_writedata(arpdata, arpheader, tha, tpa, sha, spa);

                interface->send(response, responsetotal - response);

            }

            break;

        case ARP_REPLY:
            hook->save(sha, spa);

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

void arp_inithook(struct arp_hook *hook, unsigned int (*match)(unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength), unsigned char *(*lookup)(void *paddress), void (*save)(void *haddress, void *paddress))
{

    list_inititem(&hook->item, hook);

    hook->match = match;
    hook->lookup = lookup;
    hook->save = save;

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

