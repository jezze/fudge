#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include "arp.h"

static struct ethernet_protocol ethernetprotocol;
static struct list hooks;

void *arp_writehead(void *buffer, unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength, unsigned int operation, unsigned char *sha, unsigned char *spa, unsigned char *tha, unsigned char *tpa)
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

    memory_copy(data, sha, header->hlength);
    memory_copy(data + header->hlength, spa, header->plength);
    memory_copy(data + header->hlength + header->plength, tha, header->hlength);
    memory_copy(data + header->hlength + header->plength + header->hlength, tpa, header->plength);

    return data + header->hlength + header->plength + header->hlength + header->plength;

}

void arp_send(void *buffer, unsigned int count)
{

    struct resource *resource;
    struct ethernet_interface *interface;

    resource = resource_findtype(0, RESOURCE_ETHERNETINTERFACE);

    if (!resource)
        return;

    interface = resource->data;

    if (!interface)
        return;

    ethernet_send(interface, buffer, count);

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

static void ethernetprotocol_notify(struct ethernet_header *ethernetheader, void *buffer, unsigned int count)
{

    struct arp_header *header = buffer;
    unsigned char *data = (unsigned char *)(header + 1);
    unsigned int htype = (header->htype[0] << 8) | header->htype[1];
    unsigned int ptype = (header->ptype[0] << 8) | header->ptype[1];
    unsigned int operation = (header->operation[0] << 8) | header->operation[1];
    unsigned char *sha = data;
    unsigned char *spa = data + header->hlength;
    unsigned char *tha = data + header->hlength + header->plength;
    unsigned char *tpa = data + header->hlength + header->plength + header->hlength;
    struct arp_hook *hook = findhook(htype, header->hlength, ptype, header->plength);

    if (hook)
    {

        switch (operation)
        {

        case ARP_REQUEST:
            hook->save(sha, spa);

            tha = hook->lookup(tpa);

            if (tha)
            {

                unsigned char response[ETHERNET_MTU];
                unsigned char *current = response;

                current = ethernet_writehead(current, ethernetprotocol.type, tha, sha);
                current = arp_writehead(current, htype, header->hlength, ptype, header->plength, ARP_REPLY, tha, tpa, sha, spa);

                arp_send(response, current - response);

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

