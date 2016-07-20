#include <fudge.h>
#include <net/arp.h>
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

static void ethernetprotocol_addinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_removeinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
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
        unsigned char *hardwareaddress;
        unsigned int c = 0;

        if (hook->htype != htype || hook->ptype != ptype)
            continue;

        switch (operation)
        {

        case 1:
            hardwareaddress = hook->gethardwareaddress(header->plength, data + header->hlength + header->plength + header->hlength);

            if (!hardwareaddress)
                continue;

            c += ethernet_writeheader(ethernetprotocol.type, interface->hardwareaddress, data, response);
            c += arp_writeheader(htype, header->hlength, ptype, header->plength, 2, hardwareaddress, data + header->hlength + header->plength + header->hlength, data, data + header->hlength, response + c);

            interface->send(c, response);

            break;

        }

    }

    system_multicast(&ethernetprotocol.data.links, count, buffer);

}

void arp_registerhook(struct arp_hook *hook)
{

    list_add(&hooks, &hook->item);

}

void arp_unregisterhook(struct arp_hook *hook)
{

    list_remove(&hooks, &hook->item);

}

void arp_inithook(struct arp_hook *hook, unsigned short htype, unsigned short ptype, unsigned char *(*gethardwareaddress)(unsigned int count, void *protocoladdress))
{

    list_inititem(&hook->item, hook);

    hook->htype = htype;
    hook->ptype = ptype;
    hook->gethardwareaddress = gethardwareaddress;

}

void module_init(void)
{

    ethernet_initprotocol(&ethernetprotocol, "arp", 0x0806, ethernetprotocol_addinterface, ethernetprotocol_removeinterface, ethernetprotocol_notify);

}

void module_register(void)
{

    ethernet_registerprotocol(&ethernetprotocol);

}

void module_unregister(void)
{

    ethernet_unregisterprotocol(&ethernetprotocol);

}

