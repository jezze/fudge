struct ethernet_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    void (*oninfo)(struct event_ethernetinfo *ethernetinfo);
    unsigned int (*ondata)(void *buffer, unsigned int count);

};

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count);
void ethernet_registerinterface(struct ethernet_interface *interface);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, void (*oninfo)(struct event_ethernetinfo *info), unsigned int (*ondata)(void *buffer, unsigned int count));
