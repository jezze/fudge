struct timer_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node event1;
    struct system_node event10;
    struct system_node event100;
    struct system_node event1000;
    unsigned int id;

};

void timer_notifytick(struct timer_interface *interface);
void timer_registerinterface(struct timer_interface *interface);
void timer_unregisterinterface(struct timer_interface *interface);
void timer_initinterface(struct timer_interface *interface, unsigned int id);
