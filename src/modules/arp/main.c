#include <fudge.h>
#include <net/arp.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include "arp.h"

static struct ethernet_protocol ethernetprotocol;
static struct list hooks;

static void ethernetprotocol_addinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_removeinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    struct arp_header *header = buffer;
    unsigned short htype;
    unsigned short ptype;
    unsigned short operation;
    unsigned int length;
    struct list_item *current;
    unsigned char response[512];
    struct arp_header *responseheader = (struct arp_header *)response;

    if (count < sizeof (struct arp_header))
        return;

    htype = (header->htype[0] << 8) | header->htype[1];
    ptype = (header->ptype[0] << 8) | header->ptype[1];
    operation = (header->operation[0] << 8) | header->operation[1];
    length = header->hlength + header->plength;

    for (current = hooks.head; current; current = current->next)
    {

        struct arp_hook *hook = current->data;
        unsigned char *hardwareaddress;

        if (hook->htype != htype || hook->ptype != ptype)
            continue;

        switch (operation)
        {

        case 1:
            hardwareaddress = hook->gethardwareaddress(header->plength, (unsigned char *)buffer + sizeof (struct arp_header) + length + header->hlength);

            if (!hardwareaddress)
                continue;

            memory_write(response, 512, header, sizeof (struct arp_header), 0);
            memory_write(response, 512, (unsigned char *)buffer + sizeof (struct arp_header), length, sizeof (struct arp_header) + length);
            memory_write(response, 512, (unsigned char *)buffer + sizeof (struct arp_header) + length, length, sizeof (struct arp_header));
            memory_write(response, 512, hardwareaddress, header->hlength, sizeof (struct arp_header));

            responseheader->operation[1] = 2;

            interface->send(count, response);

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

