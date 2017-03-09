struct udp_header
{

    unsigned char sp[2];
    unsigned char tp[2];
    unsigned char length[2];
    unsigned char checksum[2];

};

struct udp_hook
{

    struct list_item item;
    unsigned int (*match)(unsigned int port);
    void (*notify)(struct ipv4_header *ipv4header, struct udp_header *header, void *buffer, unsigned int count);

};

void udp_send(unsigned char *tip, void *payload, unsigned int count);
void udp_registerhook(struct udp_hook *hook);
void udp_unregisterhook(struct udp_hook *hook);
void udp_inithook(struct udp_hook *hook, unsigned int (*match)(unsigned int port), void (*notify)(struct ipv4_header *ipv4header, struct udp_header *header, void *buffer, unsigned int count));
