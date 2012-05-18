#include <fudge.h>

static struct ethernet_interface eth0;

static struct ethernet_header *read_ethernet_header(void *buffer)
{

    return (struct ethernet_header *)buffer;

}

static void write_ethernet_header(void *buffer, struct ethernet_header *header)
{

    memory_copy(buffer, header, sizeof (struct ethernet_header));

}

static struct arp_header *read_arp_header(void *buffer)
{

    char *in = buffer;

    return (struct arp_header *)(in + sizeof (struct ethernet_header));

}

static void write_arp_header(void *buffer, struct arp_header *header)
{

    char *out = buffer;

    memory_copy(out + sizeof (struct ethernet_header), header, sizeof (struct arp_header));

}

static void receive()
{

    char buffer[0x800];

    unsigned int id = call_open(FILE_NEW, "/module/rtl8139/data");
    call_read(id, 0, 0x800, buffer);
    call_close(id);

    struct ethernet_header *eheader = read_ethernet_header(buffer);
    struct arp_header *aheader = read_arp_header(buffer);

    char num[32];

    call_write(FILE_STDOUT, 0, 6, "FROM: ");
    string_write_num(num, eheader->sha[0], 16);
    call_write(FILE_STDOUT, 0, string_length(num), num);
    call_write(FILE_STDOUT, 0, 1, ":");
    string_write_num(num, eheader->sha[1], 16);
    call_write(FILE_STDOUT, 0, string_length(num), num);
    call_write(FILE_STDOUT, 0, 1, ":");
    string_write_num(num, eheader->sha[2], 16);
    call_write(FILE_STDOUT, 0, string_length(num), num);
    call_write(FILE_STDOUT, 0, 1, ":");
    string_write_num(num, eheader->sha[3], 16);
    call_write(FILE_STDOUT, 0, string_length(num), num);
    call_write(FILE_STDOUT, 0, 1, ":");
    string_write_num(num, eheader->sha[4], 16);
    call_write(FILE_STDOUT, 0, string_length(num), num);
    call_write(FILE_STDOUT, 0, 1, ":");
    string_write_num(num, eheader->sha[5], 16);
    call_write(FILE_STDOUT, 0, string_length(num), num);
    call_write(FILE_STDOUT, 0, 1, "\n");

    call_wait();

}

static void send()
{

    char buffer[0x800];
    char tpa[] = {0xC0, 0xA8, 0x00, 0x01};
    char tha[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    struct ethernet_header eheader;
    struct arp_header aheader;

    eheader.typeHigh = 0x08;
    eheader.typeLow = 0x06;

    memory_copy(eheader.sha, eth0.mac, 6);
    memory_copy(eheader.tha, tha, 6);

    aheader.htypeHigh = 0x00;
    aheader.htypeLow = 0x01;
    aheader.ptypeHigh = 0x08;
    aheader.ptypeLow = 0x00;
    aheader.hlength = 0x06;
    aheader.plength = 0x04;
    aheader.operationHigh = 0x00;
    aheader.operationLow = 0x01;

    memory_copy(aheader.sha, eth0.mac, 6);
    memory_copy(aheader.tha, tha, 6);
    memory_copy(aheader.spa, eth0.ip, 4);
    memory_copy(aheader.tpa, tpa, 4);

    write_ethernet_header(buffer, &eheader);
    write_arp_header(buffer, &aheader);

    unsigned int id = call_open(FILE_NEW, "/module/rtl8139/data");
    call_write(id, 0, sizeof (struct ethernet_header) + sizeof (struct arp_header), buffer);
    call_close(id);

}

void main()
{

    call_attach(0x2B, receive);

    eth0.ip[0] = 0xC0;
    eth0.ip[1] = 0xA8;
    eth0.ip[2] = 0x00;
    eth0.ip[3] = 0x05;

    unsigned int id = call_open(FILE_NEW, "/module/rtl8139/mac");
    call_read(id, 0, 6, eth0.mac);
    call_close(id);

    send();

    call_wait();

}

