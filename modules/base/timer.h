struct base_timer_interface
{

    struct base_interface base;
    void (*sleep)(unsigned int duration);

};

struct base_timer_interfacenode
{

    struct system_node base;
    struct system_node sleep;
    struct base_timer_interface *interface;

};

void base_timer_registerinterface(struct base_timer_interface *interface);
void base_timer_registerinterfacenode(struct base_timer_interfacenode *node);
void base_timer_unregisterinterface(struct base_timer_interface *interface);
void base_timer_unregisterinterfacenode(struct base_timer_interfacenode *node);
void base_timer_initinterface(struct base_timer_interface *interface, struct base_bus *bus, unsigned int id, void (*sleep)(unsigned int duration));
void base_timer_initinterfacenode(struct base_timer_interfacenode *node, struct base_timer_interface *interface);
void base_timer_setup();
