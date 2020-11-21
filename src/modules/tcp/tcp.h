struct tcp_hook
{

    struct list_item item;
    unsigned int (*match)(unsigned int port);
    void (*notify)(struct tcp_header *header, void *buffer, unsigned int count);

};

void tcp_registerhook(struct tcp_hook *hook);
void tcp_unregisterhook(struct tcp_hook *hook);
void tcp_inithook(struct tcp_hook *hook, unsigned int (*match)(unsigned int port), void (*notify)(struct tcp_header *header, void *buffer, unsigned int count));
