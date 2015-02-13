struct arp_hook
{

    struct list_item item;
    unsigned short ptype;
    void (*notify)(struct ethernet_interface *interface, struct arp_message *message);

};

void arp_registerhook(struct arp_hook *hook);
void arp_unregisterhook(struct arp_hook *hook);
void arp_inithook(struct arp_hook *hook, unsigned short ptype, void (*notify)(struct ethernet_interface *interface, struct arp_message *message));
