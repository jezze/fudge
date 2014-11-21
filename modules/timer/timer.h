struct timer_interface
{

    struct base_interface base;
    void (*sleep)(unsigned int duration);

};

struct timer_interfacenode
{

    struct system_node base;
    struct system_node sleep;
    struct timer_interface *interface;

};

void timer_registerinterface(struct timer_interface *interface);
void timer_registerinterfacenode(struct timer_interfacenode *node);
void timer_unregisterinterface(struct timer_interface *interface);
void timer_unregisterinterfacenode(struct timer_interfacenode *node);
void timer_initinterface(struct timer_interface *interface, struct base_bus *bus, unsigned int id, void (*sleep)(unsigned int duration));
void timer_initinterfacenode(struct timer_interfacenode *node, struct timer_interface *interface);
