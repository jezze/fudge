struct udp_hook
{

    struct list_item item;
    void *self;
    unsigned int port;
    void (*notify)(void *self, struct ethernet_interface *interface, void *buffer, unsigned int count);

};

void udp_registerhook(struct udp_hook *hook);
void udp_unregisterhook(struct udp_hook *hook);
void udp_inithook(struct udp_hook *hook, void *self, unsigned int port, void (*notify)(void *self, struct ethernet_interface *interface, void *buffer, unsigned int count));
