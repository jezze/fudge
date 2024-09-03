struct ethernet_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int ichannel;
    unsigned int (*ctrl)(unsigned int source);
    unsigned int (*send)(void *buffer, unsigned int count);

};

void ethernet_notify(struct ethernet_interface *interface, void *buffer, unsigned int count);
void ethernet_registerinterface(struct ethernet_interface *interface);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*ctrl)(unsigned int source), unsigned int (*send)(void *buffer, unsigned int count));
