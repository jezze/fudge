#ifndef LIB_ICMPV6_H
#define LIB_ICMPV6_H

struct icmpv6_header
{

    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    unsigned int data;

} __attribute__((packed));

#endif

