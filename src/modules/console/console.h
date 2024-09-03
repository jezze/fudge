struct console_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int ichannel;
    unsigned int (*send)(void *buffer, unsigned int count);

};

void console_notifydata(struct console_interface *interface, unsigned char data);
void console_registerinterface(struct console_interface *interface);
void console_unregisterinterface(struct console_interface *interface);
void console_initinterface(struct console_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*send)(void *buffer, unsigned int count));
