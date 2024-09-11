struct ethernet_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int ichannel;
    unsigned int (*oninfo)(unsigned int source);
    unsigned int (*ondata)(void *buffer, unsigned int count);

};

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count);
void ethernet_registerinterface(struct ethernet_interface *interface);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*oninfo)(unsigned int source), unsigned int (*ondata)(void *buffer, unsigned int count));
