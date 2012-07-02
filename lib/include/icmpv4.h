#ifndef LIB_ICMPV4_H
#define LIB_ICMPV4_H

struct icmpv4_header
{

    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    unsigned int data;

} __attribute__((packed));

#endif

