struct timer_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node sleep;
    unsigned int id;

};

void timer_notify(struct timer_interface *interface, void *buffer, unsigned int count);
void timer_registerinterface(struct timer_interface *interface, unsigned int id);
void timer_unregisterinterface(struct timer_interface *interface);
void timer_initinterface(struct timer_interface *interface);
