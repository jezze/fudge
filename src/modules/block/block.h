struct block_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int inode;
    unsigned int (*oninfo)(unsigned int source);
    unsigned int (*onblockreadrequest)(unsigned int source, unsigned int count, unsigned int offset);
    unsigned int (*onblockwriterequest)(unsigned int source, unsigned int count, unsigned int offset);

};

void block_registerinterface(struct block_interface *interface);
void block_unregisterinterface(struct block_interface *interface);
void block_initinterface(struct block_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source), unsigned int (*onblockreadrequest)(unsigned int source, unsigned int count, unsigned int offset), unsigned int (*onblockwriterequest)(unsigned int source, unsigned int count, unsigned int offset));
