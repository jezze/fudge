struct block_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int inode;
    unsigned int (*onblockrequest)(unsigned int source, unsigned int count, unsigned int sector);

};

void block_notifyblockresponse(struct block_interface *interface, void *buffer, unsigned int count);
void block_registerinterface(struct block_interface *interface);
void block_unregisterinterface(struct block_interface *interface);
void block_initinterface(struct block_interface *interface, unsigned int id, unsigned int (*onblockrequest)(unsigned int source, unsigned int count, unsigned int sector));
