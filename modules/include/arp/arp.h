#ifndef MODULES_ARP_H
#define MODULES_ARP_H

struct arp_protocol
{

    struct net_protocol base;
    unsigned char address[4];
    unsigned char buffer[8192];
    unsigned int count;

};

void arp_protocol_init(struct arp_protocol *protocol);

#endif

