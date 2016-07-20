struct ipv4_arpentry
{

    unsigned char hardwareaddress[6];
    unsigned char protocoladdress[4];

};

struct ipv4_protocol
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned char id;
    void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer);

};

void ipv4_registerprotocol(struct ipv4_protocol *protocol);
void ipv4_unregisterprotocol(struct ipv4_protocol *protocol);
void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer));
