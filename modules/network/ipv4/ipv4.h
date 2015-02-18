struct ipv4_protocol
{

    struct list_item item;
    unsigned char id;
    void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer);
    struct system_node root;
    struct system_node data;

};

void ipv4_registerprotocol(struct ipv4_protocol *protocol);
void ipv4_unregisterprotocol(struct ipv4_protocol *protocol);
void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer));
