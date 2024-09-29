struct console_interface
{

    struct resource resource;
    struct node node;
    unsigned int id;
    unsigned int (*ondata)(void *buffer, unsigned int count);

};

void console_notifydata(struct console_interface *interface, unsigned char data);
void console_registerinterface(struct console_interface *interface);
void console_unregisterinterface(struct console_interface *interface);
void console_initinterface(struct console_interface *interface, unsigned int id, unsigned int (*ondata)(void *buffer, unsigned int count));
