struct block_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;

};

void block_notify(struct block_interface *interface, void *buffer, unsigned int count);
void block_registerinterface(struct block_interface *interface);
void block_unregisterinterface(struct block_interface *interface);
void block_initinterface(struct block_interface *interface, unsigned int id);
