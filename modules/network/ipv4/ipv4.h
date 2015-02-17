struct ipv4_protocolnode
{

    struct system_node base;
    struct system_node data;
    struct ipv4_protocol *protocol;

};

struct ipv4_protocol
{

    struct list_item item;
    unsigned char id;
    void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer);
    struct ipv4_protocolnode node;

};

void ipv4_registerprotocol(struct ipv4_protocol *protocol);
void ipv4_unregisterprotocol(struct ipv4_protocol *protocol);
void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer));
