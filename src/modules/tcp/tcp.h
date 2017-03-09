struct tcp_header
{

    unsigned char sp[2];
    unsigned char tp[2];
    unsigned char seq[4];
    unsigned char ack[4];
    unsigned char flags[2];
    unsigned char window[2];
    unsigned char checksum[2];
    unsigned char urgent[2];

};

struct tcp_hook
{

    struct list_item item;
    unsigned int (*match)(unsigned int port);
    void (*notify)(struct tcp_header *header, void *buffer, unsigned int count);

};

void tcp_registerhook(struct tcp_hook *hook);
void tcp_unregisterhook(struct tcp_hook *hook);
void tcp_inithook(struct tcp_hook *hook, unsigned int (*match)(unsigned int port), void (*notify)(struct tcp_header *header, void *buffer, unsigned int count));
