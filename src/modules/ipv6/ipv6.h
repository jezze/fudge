#define IPV6_ADDRSIZE                   16

struct ipv6_header
{

    unsigned char label[4];
    unsigned char length[2];
    unsigned char next;
    unsigned char hop;
    unsigned char sip[IPV6_ADDRSIZE];
    unsigned char tip[IPV6_ADDRSIZE];

};

struct ipv6_protocol
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    struct list datastates;
    struct spinlock datalock;
    unsigned char id;
    void (*notify)(struct ipv6_header *ipv6header, void *buffer, unsigned int count);

};

void ipv6_registerprotocol(struct ipv6_protocol *protocol);
void ipv6_unregisterprotocol(struct ipv6_protocol *protocol);
void ipv6_initprotocol(struct ipv6_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ipv6_header *ipv6header, void *buffer, unsigned int count));
