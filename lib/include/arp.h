#ifndef LIB_ARP_H
#define LIB_ARP_H

struct arp_header
{

    unsigned char htype[2];
    unsigned char ptype[2];
    unsigned char hlength;
    unsigned char plength;
    unsigned char operation[2];
    unsigned char sha[6];
    unsigned char spa[4];
    unsigned char tha[6];
    unsigned char tpa[4];

} __attribute__((packed));

#endif

