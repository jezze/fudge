struct console_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    unsigned int id;
    struct ctrl_consolesettings settings;
    unsigned int (*send)(unsigned int count, void *buffer);

};

void console_notify(struct console_interface *interface, unsigned int count, void *buffer);
void console_registerinterface(struct console_interface *interface, unsigned int id);
void console_unregisterinterface(struct console_interface *interface);
void console_initinterface(struct console_interface *interface, unsigned int (*send)(unsigned int count, void *buffer));
