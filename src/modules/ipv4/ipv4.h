struct ipv4_hook
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned char id;
    void (*notify)(struct ipv4_header *ipv4header, void *buffer, unsigned int count);

};

void *ipv4_writehead(void *buffer, unsigned char *sip, unsigned char *tip, unsigned int protocol, unsigned int count);
void ipv4_send(void *buffer, unsigned int count);
void ipv4_registerhook(struct ipv4_hook *hook);
void ipv4_unregisterhook(struct ipv4_hook *hook);
void ipv4_inithook(struct ipv4_hook *hook, char *name, unsigned char id, void (*notify)(struct ipv4_header *ipv4header, void *buffer, unsigned int count));
