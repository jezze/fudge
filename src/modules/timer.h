struct timer_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;

};

void timer_notifytick(struct timer_interface *interface, unsigned int counter);
void timer_registerinterface(struct timer_interface *interface);
void timer_unregisterinterface(struct timer_interface *interface);
void timer_initinterface(struct timer_interface *interface, unsigned int id);
