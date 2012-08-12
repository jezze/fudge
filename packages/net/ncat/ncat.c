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
    struct arp_header *aheader;
    unsigned int type;

    if (!eheader)
        return 0;

    type = (eheader->type[0] << 8) | eheader->type[1];

    if (type != 0x0806)
        return 0;

    aheader = (struct arp_header *)eheader + 14;

    return aheader;

}

void handle_network_event()
{

    unsigned char buffer[0x800];
    struct arp_header *header;
    struct ethernet_header eheader;
    struct arp_header aheader;

    call_write(FILE_STDOUT, 0, 9, "INCOMING\n");

    call_open(3, 20, "/module/rtl8139_data");
    call_read(3, 0, 0x800, buffer);
    call_close(3);

    header = read_arp(buffer);

    if (!header)
        call_idle();

    eheader.type[0] = 0x08;
    eheader.type[1] = 0x06;

    memory_copy(eheader.sha, eth0.mac, 6);
    memory_copy(eheader.tha, header->sha, 6);

    aheader.htype[0] = 0x00;
    aheader.htype[1] = 0x01;
    aheader.ptype[0] = 0x08;
    aheader.ptype[1] = 0x00;
    aheader.hlength = 0x06;
    aheader.plength = 0x04;
    aheader.operation[0] = 0x00;
    aheader.operation[1] = 0x02;

    memory_copy(aheader.sha, eth0.mac, 6);
    memory_copy(aheader.spa, eth0.ip, 4);
    memory_copy(aheader.tha, header->sha, 6);
    memory_copy(aheader.tpa, header->spa, 4);

    memory_copy(buffer, &eheader, sizeof (struct ethernet_header));
    memory_copy(buffer + sizeof (struct ethernet_header), &aheader, sizeof (struct arp_header));

    call_open(3, 20, "/module/rtl8139_data");
    call_write(3, 0, sizeof (struct ethernet_header) + sizeof (struct arp_header), buffer);
    call_close(3);
    call_write(FILE_STDOUT, 0, 18, "Responding to ARP\n");
    call_idle();

}

void main()
{

    call_attach(0x2B, handle_network_event);

    call_write(FILE_STDOUT, 0, 13, "Listening...\n");

    eth0.ip[0] = 0xC0;
    eth0.ip[1] = 0xA8;
    eth0.ip[2] = 0x01;
    eth0.ip[3] = 0x02;

    call_open(3, 19, "/module/rtl8139_mac");
    call_read(3, 0, 6, eth0.mac);
    call_close(3);

    call_idle();

}

