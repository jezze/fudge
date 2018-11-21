struct block_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;

};

struct block_protocol
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    void (*notify)(void *buffer, unsigned int count);

};

void block_notify(struct block_interface *interface, void *buffer, unsigned int count);
void block_registerinterface(struct block_interface *interface);
void block_registerprotocol(struct block_protocol *protocol);
void block_unregisterinterface(struct block_interface *interface);
void block_unregisterprotocol(struct block_protocol *protocol);
void block_initinterface(struct block_interface *interface, unsigned int id);
void block_initprotocol(struct block_protocol *protocol, char *name, void (*notify)(void *buffer, unsigned int count));
