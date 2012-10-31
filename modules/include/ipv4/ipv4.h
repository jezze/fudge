struct ipv4_protocol
{

    struct net_protocol base;
    unsigned char address[4];
    unsigned char buffer[8192];
    unsigned int count;

};

void ipv4_init_protocol(struct ipv4_protocol *protocol);
