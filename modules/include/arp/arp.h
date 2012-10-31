struct arp_protocol
{

    struct net_protocol base;
    unsigned char address[4];
    unsigned char buffer[8192];
    unsigned int count;

};

void arp_init_protocol(struct arp_protocol *protocol);
