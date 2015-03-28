struct block_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;
    unsigned int (*rdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);

};

void block_notify(struct block_interface *interface, unsigned int count, void *buffer);
void block_registerinterface(struct block_interface *interface, unsigned int id);
void block_unregisterinterface(struct block_interface *interface);
void block_initinterface(struct block_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer));
