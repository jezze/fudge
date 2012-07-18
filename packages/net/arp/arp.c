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

static void write_arp_header(void *buffer, struct arp_header *header)
{

    char *out = buffer;

    memory_copy(out + sizeof (struct ethernet_header), header, sizeof (struct arp_header));

}

static void receive()
{

    char num[32];
    char buffer[0x800];
    struct ethernet_header *eheader;

    call_open(3, 20, "/module/rtl8139/data");
    call_read(3, 0, 0x800, buffer);
    call_close(3);

    eheader = read_ethernet_header(buffer);

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
    unsigned char tpa[] = {0xC0, 0xA8, 0x00, 0x01};
    unsigned char tha[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    struct ethernet_header eheader;
    struct arp_header aheader;

    eheader.type[0] = 0x08;
    eheader.type[1] = 0x06;

    memory_copy(eheader.sha, eth0.mac, 6);
    memory_copy(eheader.tha, tha, 6);

    aheader.htype[0] = 0x00;
    aheader.htype[1] = 0x01;
    aheader.ptype[0] = 0x08;
    aheader.ptype[1] = 0x00;
    aheader.hlength = 0x06;
    aheader.plength = 0x04;
    aheader.operation[0] = 0x00;
    aheader.operation[1] = 0x01;

    memory_copy(aheader.sha, eth0.mac, 6);
    memory_copy(aheader.tha, tha, 6);
    memory_copy(aheader.spa, eth0.ip, 4);
    memory_copy(aheader.tpa, tpa, 4);

    write_ethernet_header(buffer, &eheader);
    write_arp_header(buffer, &aheader);

    call_open(3, 20, "/module/rtl8139/data");
    call_write(3, 0, sizeof (struct ethernet_header) + sizeof (struct arp_header), buffer);
    call_close(3);

}

void main()
{

    call_attach(0x2B, receive);

    eth0.ip[0] = 0xC0;
    eth0.ip[1] = 0xA8;
    eth0.ip[2] = 0x00;
    eth0.ip[3] = 0x05;

    call_open(3, 19, "/module/rtl8139/mac");
    call_read(3, 0, 6, eth0.mac);
    call_close(3);

    send();

    call_wait();

}

