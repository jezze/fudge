struct block_interfacenode
{

    struct system_node base;
    struct system_node data;
    struct block_interface *interface;

};

struct block_interface
{

    struct base_interface base;
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);
    struct block_interfacenode node;

};

void block_registerinterface(struct block_interface *interface, unsigned int id);
void block_unregisterinterface(struct block_interface *interface);
void block_initinterface(struct block_interface *interface, struct base_driver *driver, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer));
