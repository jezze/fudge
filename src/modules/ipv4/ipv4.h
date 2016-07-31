#define IPV4_PROTOCOL                   0x0800
#define IPV4_ADDRSIZE                   4

struct ipv4_arpentry
{

    unsigned char haddress[ETHERNET_ADDRSIZE];
    unsigned char paddress[IPV4_ADDRSIZE];

};

struct ipv4_protocol
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned char id;
    void (*notify)(struct ethernet_interface *interface, void *buffer, unsigned int count);

};

unsigned int ipv4_writeheader(unsigned char *sip, unsigned char *tip, void *buffer);
void ipv4_registerprotocol(struct ipv4_protocol *protocol);
void ipv4_unregisterprotocol(struct ipv4_protocol *protocol);
void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ethernet_interface *interface, void *buffer, unsigned int count));
