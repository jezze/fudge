struct block_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    struct list datalinks;
    unsigned int id;
    unsigned int (*rdata)(void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*wdata)(void *buffer, unsigned int count, unsigned int offset);

};

void block_notify(struct block_interface *interface, void *buffer, unsigned int count);
void block_registerinterface(struct block_interface *interface, unsigned int id);
void block_unregisterinterface(struct block_interface *interface);
void block_initinterface(struct block_interface *interface, unsigned int (*rdata)(void *buffer, unsigned int count, unsigned int offset), unsigned int (*wdata)(void *buffer, unsigned int count, unsigned int offset));
