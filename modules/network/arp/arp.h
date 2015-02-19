struct arp_hook
{

    struct list_item item;
    unsigned short htype;
    unsigned short ptype;
    void (*notify)(unsigned int count, void *buffer);

};

void arp_registerhook(struct arp_hook *hook);
void arp_unregisterhook(struct arp_hook *hook);
void arp_inithook(struct arp_hook *hook, unsigned short htype, unsigned short ptype, void (*notify)(unsigned int count, void *buffer));
