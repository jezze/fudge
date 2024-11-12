struct ethernet_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int inode;
    unsigned int (*oninfo)(unsigned int source);
    unsigned int (*ondata)(unsigned int source, void *buffer, unsigned int count);

};

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count);
void ethernet_registerinterface(struct ethernet_interface *interface);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source), unsigned int (*ondata)(unsigned int source, void *buffer, unsigned int count));
