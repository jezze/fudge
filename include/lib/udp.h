#ifndef LIB_UDP_H
#define LIB_UDP_H

struct udp_header
{

    unsigned short sp;
    unsigned short tp;
    unsigned short length;
    unsigned short checksum;

} __attribute__((packed));

#endif

