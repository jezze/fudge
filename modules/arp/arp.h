struct arp_protocol
{

    struct net_protocol base;

};

void arp_init_protocol(struct arp_protocol *protocol);
