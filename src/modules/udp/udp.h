struct udp_hook
{

    struct list_item item;
    unsigned int (*match)(unsigned int port);
    void (*notify)(struct ethernet_interface *interface, void *buffer, unsigned int count);

};

void udp_registerhook(struct udp_hook *hook);
void udp_unregisterhook(struct udp_hook *hook);
void udp_inithook(struct udp_hook *hook, unsigned int (*match)(unsigned int port), void (*notify)(struct ethernet_interface *interface, void *buffer, unsigned int count));
