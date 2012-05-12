#ifndef LIB_ARP_H
#define LIB_ARP_H

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

#endif

