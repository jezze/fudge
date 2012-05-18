#include <fudge.h>

static struct ethernet_interface eth0;

static struct ethernet_header *read_ethernet(void *data)
{

    struct ethernet_header *eheader = data;

    return eheader;

}

static struct arp_header *read_arp(void *data)
{

    struct ethernet_header *eheader = read_ethernet(data);

    if (!eheader)
        return 0;

    unsigned short type = (eheader->typeHigh << 8) | eheader->typeLow;

    if (type != 0x0806)
        return 0;

    struct arp_header *aheader = (void *)eheader + 14;

    return aheader;

}

void handle_network_event()
{

    unsigned char buffer[0x800];

    call_write(FILE_STDOUT, 0, 9, "INCOMING\n");

    unsigned int id = call_open(FILE_NEW, "/module/rtl8139/data");
    call_read(id, 0, 0x800, buffer);
    call_close(id);

    struct arp_header *header = read_arp(buffer);

    if (!header)
        call_wait();

    struct ethernet_header eheader;

    eheader.typeHigh = 0x08;
    eheader.typeLow = 0x06;

    memory_copy(eheader.sha, eth0.mac, 6);
    memory_copy(eheader.tha, header->sha, 6);

    struct arp_header aheader;

    aheader.htypeHigh = 0x00;
    aheader.htypeLow = 0x01;
    aheader.ptypeHigh = 0x08;
    aheader.ptypeLow = 0x00;
    aheader.hlength = 0x06;
    aheader.plength = 0x04;
    aheader.operationHigh = 0x00;
    aheader.operationLow = 0x02;

    memory_copy(aheader.sha, eth0.mac, 6);
    memory_copy(aheader.spa, eth0.ip, 4);
    memory_copy(aheader.tha, header->sha, 6);
    memory_copy(aheader.tpa, header->spa, 4);

    memory_copy(buffer, &eheader, sizeof (struct ethernet_header));
    memory_copy(buffer + sizeof (struct ethernet_header), &aheader, sizeof (struct arp_header));

    unsigned int id2 = call_open(FILE_NEW, "/module/rtl8139/data");
    call_write(id2, 0, sizeof (struct ethernet_header) + sizeof (struct arp_header), buffer);
    call_close(id2);
    call_write(FILE_STDOUT, 0, 18, "Responding to ARP\n");
    call_wait();

}

void main()
{

    call_attach(0x2B, handle_network_event);

    call_write(FILE_STDOUT, 0, 13, "Listening...\n");

    eth0.ip[0] = 0xC0;
    eth0.ip[1] = 0xA8;
    eth0.ip[2] = 0x00;
    eth0.ip[3] = 0x05;

    unsigned int id = call_open(FILE_NEW, "/module/rtl8139/mac");
    call_read(id, 0, 6, eth0.mac);
    call_close(id);

    call_wait();

}

