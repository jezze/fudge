struct console_interfacenode
{

    struct system_node base;
    struct system_node ctrl;
    struct system_node in;
    struct system_node out;
    struct console_interface *interface;

};

struct console_interface
{

    struct base_interface base;
    struct ctrl_consolesettings settings;
    struct list mailboxes;
    unsigned int (*send)(unsigned int offset, unsigned int count, void *buffer);
    struct console_interfacenode node;

};

void console_notify(struct console_interface *interface, unsigned int count, void *buffer);
void console_registerinterface(struct console_interface *interface, struct base_bus *bus, unsigned int id);
void console_unregisterinterface(struct console_interface *interface);
void console_initinterface(struct console_interface *interface, struct base_driver *driver, unsigned int (*send)(unsigned int offset, unsigned int count, void *buffer));
