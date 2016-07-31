struct block_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;
    unsigned int (*rdata)(unsigned int offset, void *buffer, unsigned int count);
    unsigned int (*wdata)(unsigned int offset, void *buffer, unsigned int count);

};

void block_notify(struct block_interface *interface, void *buffer, unsigned int count);
void block_registerinterface(struct block_interface *interface, unsigned int id);
void block_unregisterinterface(struct block_interface *interface);
void block_initinterface(struct block_interface *interface, unsigned int (*rdata)(unsigned int offset, void *buffer, unsigned int count), unsigned int (*wdata)(unsigned int offset, void *buffer, unsigned int count));
