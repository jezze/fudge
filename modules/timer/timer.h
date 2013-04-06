struct timer_interface
{

    struct base_driver *driver;

};

struct timer_protocol
{

    char *name;

};

void timer_register_interface(struct timer_interface *interface);
void timer_register_protocol(struct timer_protocol *protocol);
void timer_unregister_interface(struct timer_interface *interface);
void timer_unregister_protocol(struct timer_protocol *protocol);
void timer_init_interface(struct timer_interface *interface, struct base_driver *driver);
void timer_init_protocol(struct timer_protocol *protocol, char *name);
