#ifndef MODULES_IPV4_H
#define MODULES_IPV4_H

struct ipv4_protocol
{

    struct net_protocol base;
    unsigned char address[4];
    unsigned char buffer[8192];

};

void ipv4_protocol_init(struct ipv4_protocol *protocol);

#endif

