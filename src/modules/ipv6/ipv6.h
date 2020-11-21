struct ipv6_hook
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned char id;
    void (*notify)(struct ipv6_header *ipv6header, void *buffer, unsigned int count);

};

void ipv6_registerhook(struct ipv6_hook *hook);
void ipv6_unregisterhook(struct ipv6_hook *hook);
void ipv6_inithook(struct ipv6_hook *hook, char *name, unsigned char id, void (*notify)(struct ipv6_header *ipv6header, void *buffer, unsigned int count));
