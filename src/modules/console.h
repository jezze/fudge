struct console_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int color;
    unsigned int cursor;
    unsigned int (*ondata)(void *buffer, unsigned int count);

};

void console_notifydata(struct console_interface *interface, unsigned char data);
void console_cursorset(struct console_interface *interface, unsigned int position);
void console_cursorleft(struct console_interface *interface, unsigned int steps);
void console_cursorright(struct console_interface *interface, unsigned int steps);
void console_cursorup(struct console_interface *interface, unsigned int steps);
void console_cursordown(struct console_interface *interface, unsigned int steps);
void console_cursorhome(struct console_interface *interface);
void console_cursorend(struct console_interface *interface);
void console_registerinterface(struct console_interface *interface);
void console_unregisterinterface(struct console_interface *interface);
void console_initinterface(struct console_interface *interface, unsigned int id, unsigned int (*ondata)(void *buffer, unsigned int count));
