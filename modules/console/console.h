struct console_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node in;
    struct system_node out;
    unsigned int id;
    unsigned char scroll;
    unsigned int (*send)(unsigned int offset, unsigned int count, void *buffer);

};

void console_notify(struct console_interface *interface, unsigned int size, unsigned int count, void *buffer);
void console_registerinterface(struct console_interface *interface, unsigned int id);
void console_unregisterinterface(struct console_interface *interface);
void console_initinterface(struct console_interface *interface, unsigned int (*send)(unsigned int offset, unsigned int count, void *buffer));
