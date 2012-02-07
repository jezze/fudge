#include <fudge.h>

struct arp_header
{

    unsigned short htype;
    unsigned short ptype;
    unsigned char hlength;
    unsigned char plength;
    unsigned char operationHigh;
    unsigned char operationLow;
    unsigned char sha[6];
    unsigned char spa[4];
    unsigned char tha[6];
    unsigned char tpa[4];

} __attribute__((packed));

static char mac[6];
static struct arp_header response;

static void read_arp(unsigned char *data)
{

    struct arp_header *header = (struct arp_header *)data;

    file_write_format(FILE_STDOUT, "- ARP Operation: %d\n", header->operationHigh << 8 | header->operationLow);
    file_write_format(FILE_STDOUT, "- ARP SHA: %x:%x:%x:%x:%x:%x\n", header->sha[0], header->sha[1], header->sha[2], header->sha[3], header->sha[4], header->sha[5]);
    file_write_format(FILE_STDOUT, "- ARP SPA: %d.%d.%d.%d\n", header->spa[0], header->spa[1], header->spa[2], header->spa[3]);
    file_write_format(FILE_STDOUT, "- ARP THA: %x:%x:%x:%x:%x:%x\n", header->tha[0], header->tha[1], header->tha[2], header->tha[3], header->tha[4], header->tha[5]);
    file_write_format(FILE_STDOUT, "- ARP TPA: %d.%d.%d.%d\n", header->tpa[0], header->tpa[1], header->tpa[2], header->tpa[3]);

    unsigned int ip = 0x0a000205;

    memory_copy(response.sha, mac, 6);
    memory_copy(response.spa, &ip, 4);

    memory_copy(response.tha, header->sha, 6);
    memory_copy(response.tpa, header->tha, 4);

}

static void read_icmpv6(unsigned char *data)
{

    file_write_format(FILE_STDOUT, "- ICMPv6 Type: %d\n", data[0]);
    file_write_format(FILE_STDOUT, "- ICMPv6 Code: %d\n", data[1]);

}

static void read_ipv6(unsigned char *data)
{
/*
    log_write("- IPv6 Version: %d\n", data[0] >> 4);
    log_write("- IPv6 Length:  %d\n", data[4]);
    log_write("- IPv6 Next:  %d\n", data[6]);
    log_write("- IPv6 Source:  %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n", data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23]);
    log_write("- IPv6 Dest:    %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n", data[24], data[25], data[26], data[27], data[28], data[29], data[30], data[31], data[32], data[33], data[34], data[35], data[36], data[37], data[38], data[39]);
*/
    if (data[6] == 58)
        read_icmpv6(data + 40);
    else
        file_write_format(FILE_STDOUT, "- IPv6: Unknown next code %d\n", data[6]);

}

static void read_frame(unsigned char *data)
{
/*
    log_write("- Frame Dest:   %x:%x:%x:%x:%x:%x\n", data[0], data[1], data[2], data[3], data[4], data[5]);
    log_write("- Frame Source: %x:%x:%x:%x:%x:%x\n", data[6], data[7], data[8], data[9], data[10], data[11]);
    log_write("- Frame Type:   0x%x%x\n", data[12], data[13]);
*/
    unsigned short type = *(unsigned short *)(data + 12);

    if (data[12] == 0x08 && data[13] == 0x06)
        read_arp(data + 14);
    else if (data[12] == 0x86 && data[13] == 0xDD)
        read_ipv6(data + 14);
    else
        file_write_format(FILE_STDOUT, "Unknown protocol 0x%x%x\n", data[12], data[13]);

}

void handle_network_event()
{

    unsigned char buffer[0x800];
    unsigned int fd = file_open("rtl8139/data");
    unsigned int count = file_read(fd, 0x800, buffer);

    file_close(fd);
    read_frame(buffer);
    call_wait();

}

void main(int argc, char *argv[])
{

    unsigned int fd = file_open("rtl8139/mac");
    unsigned int count = file_read(fd, 6, mac);
    file_close(fd);

    file_write_format(FILE_STDOUT, "Mac: %x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    call_attach(0x07, handle_network_event);
    call_wait();

}

