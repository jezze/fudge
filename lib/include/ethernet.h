#ifndef LIB_ETHERNET_H
#define LIB_ETHERNET_H

struct ethernet_header
{

    unsigned char tha[6];
    unsigned char sha[6];
    unsigned char typeHigh;
    unsigned char typeLow;

} __attribute__((packed));

struct ethernet_interface
{

    unsigned char mac[6];
    unsigned char ip[4];

} __attribute__((packed));

#endif

