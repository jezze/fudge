#ifndef LIB_IPV6_H
#define LIB_IPV6_H

struct ipv6_header
{

    unsigned char label[4];
    unsigned char length[2];
    unsigned char next;
    unsigned char hop;
    unsigned char sip[8];
    unsigned char tip[8];

} __attribute__((packed));

#endif

