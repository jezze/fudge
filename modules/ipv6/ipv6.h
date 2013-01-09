struct ipv6_protocol
{

    struct net_protocol base;
    unsigned char address[4];
    unsigned char buffer[8192];
    unsigned int count;

};

void ipv6_init_protocol(struct ipv6_protocol *protocol);
