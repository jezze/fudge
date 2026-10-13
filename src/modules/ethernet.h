struct ethernet_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    void (*getmac)(unsigned char address[6]);
    unsigned int (*send)(void *buffer, unsigned int count);

};

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count);
void ethernet_registerinterface(struct ethernet_interface *interface);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, void (*getmac)(unsigned char address[6]), unsigned int (*send)(void *buffer, unsigned int count));
