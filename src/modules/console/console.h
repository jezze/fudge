struct console_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node idata;
    struct system_node odata;
    struct list datastates;
    struct spinlock rspinlock;
    struct spinlock wspinlock;
    unsigned int id;
    struct ctrl_consolesettings settings;
    unsigned int (*send)(void *buffer, unsigned int count);

};

void console_notify(struct console_interface *interface, void *buffer, unsigned int count);
void console_registerinterface(struct console_interface *interface, unsigned int id);
void console_unregisterinterface(struct console_interface *interface);
void console_initinterface(struct console_interface *interface, unsigned int (*send)(void *buffer, unsigned int count));
