struct timer_interfacenode
{

    struct system_node base;
    struct system_node sleep;
    struct timer_interface *interface;

};

struct timer_interface
{

    struct base_interface base;
    struct list mailboxes;
    struct timer_interfacenode node;

};

void timer_notify(struct timer_interface *interface, unsigned int count, void *buffer);
void timer_registerinterface(struct timer_interface *interface, unsigned int id);
void timer_unregisterinterface(struct timer_interface *interface);
void timer_initinterface(struct timer_interface *interface, struct base_driver *driver);
