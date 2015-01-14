struct timer_interfacenode
{

    struct system_node base;
    struct system_node sleep;
    struct timer_interface *interface;

};

struct timer_interface
{

    struct base_interface base;
    struct timer_interfacenode node;
    void (*sleep)(unsigned int duration);

};

void timer_registerinterface(struct timer_interface *interface, struct base_bus *bus, unsigned int id);
void timer_unregisterinterface(struct timer_interface *interface);
void timer_initinterface(struct timer_interface *interface, struct base_driver *driver, void (*sleep)(unsigned int duration));
