struct ethernet_interface
{

    struct resource resource;
    void (*getmac)(unsigned char address[6]);
    unsigned int (*send)(void *buffer, unsigned int count);

};

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count);
void ethernet_registerinterface(struct ethernet_interface *interface);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_initinterface(struct ethernet_interface *interface, void (*getmac)(unsigned char address[6]), unsigned int (*send)(void *buffer, unsigned int count));
