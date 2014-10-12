struct base_block_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);

};

struct base_block_node
{

    struct system_node base;
    struct system_node data;
    struct base_block_interface *interface;

};

struct base_block_protocol
{

    char *name;

};

void base_block_registerinterface(struct base_block_interface *interface);
void base_block_registerprotocol(struct base_block_protocol *protocol);
void base_block_registernode(struct base_block_node *node);
void base_block_unregisterinterface(struct base_block_interface *interface);
void base_block_unregisterprotocol(struct base_block_protocol *protocol);
void base_block_unregisternode(struct base_block_node *node);
void base_block_initinterface(struct base_block_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
void base_block_initprotocol(struct base_block_protocol *protocol, char *name);
void base_block_initnode(struct base_block_node *node, struct base_block_interface *interface);
void base_block_setup();
