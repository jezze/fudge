#include <fudge.h>

struct arp_header
{

    unsigned char htypeHigh;
    unsigned char htypeLow;
    unsigned char ptypeHigh;
    unsigned char ptypeLow;
    unsigned char hlength;
    unsigned char plength;
    unsigned char operationHigh;
    unsigned char operationLow;
    unsigned char sha[6];
    unsigned char spa[4];
    unsigned char tha[6];
    unsigned char tpa[4];

} __attribute__((packed));

struct frame_header
{

    unsigned char tha[6];
    unsigned char sha[6];
    unsigned char typeHigh;
    unsigned char typeLow;

} __attribute__((packed));

struct interface
{

    unsigned char mac[6];
    unsigned char ip[4];

} __attribute__((packed));

static unsigned char buffer[0x800];
struct interface eth0;

static struct frame_header *read_frame(void *data)
{

    struct frame_header *fheader = data;

    return fheader;

}

static struct arp_header *read_arp(void *data)
{

    struct frame_header *fheader = read_frame(data);

    if (!fheader)
        return 0;

    unsigned short type = (fheader->typeHigh << 8) | fheader->typeLow;

    if (type != 0x0806)
        return 0;

    struct arp_header *aheader = (void *)fheader + 14;

    return aheader;

}

void handle_network_event()
{

    unsigned int fd = file_open("/module/rtl8139/data");
    file_read(fd, 0x800, buffer);
    file_close(fd);

    struct arp_header *header = read_arp(buffer);

    if (!header)
        call_wait();

    struct frame_header fheader;

    fheader.typeHigh = 0x08;
    fheader.typeLow = 0x06;

    memory_copy(fheader.sha, eth0.mac, 6);
    memory_copy(fheader.tha, header->sha, 6);

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

    memory_copy(buffer, &fheader, sizeof (struct frame_header));
    memory_copy(buffer + sizeof (struct frame_header), &aheader, sizeof (struct arp_header));

    unsigned int fd2 = file_open("/module/rtl8139/data");
    file_write(fd2, sizeof (struct frame_header) + sizeof (struct arp_header), buffer);
    file_close(fd2);

    file_write_format(FILE_STDOUT, "Responding to ARP\n");

    call_wait();

}

void main(int argc, char *argv[])
{

    eth0.ip[0] = 0xC0;
    eth0.ip[1] = 0xA8;
    eth0.ip[2] = 0x00;
    eth0.ip[3] = 0x05;

    unsigned int fd = file_open("/module/rtl8139/mac");
    file_read(fd, 6, eth0.mac);
    file_close(fd);

    file_write_format(FILE_STDOUT, "IP: %d.%d.%d.%d\n", eth0.ip[0], eth0.ip[1], eth0.ip[2], eth0.ip[3]);
    file_write_format(FILE_STDOUT, "Mac: %x:%x:%x:%x:%x:%x\n", eth0.mac[0], eth0.mac[1], eth0.mac[2], eth0.mac[3], eth0.mac[4], eth0.mac[5]);

    call_attach(0x07, handle_network_event);
    call_wait();

}

