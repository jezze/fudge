struct kbd_interface
{

    struct base_driver *driver;

};

struct kbd_protocol
{

    char *name;

};

void kbd_register_interface(struct kbd_interface *interface);
void kbd_register_protocol(struct kbd_protocol *protocol);
void kbd_unregister_interface(struct kbd_interface *interface);
void kbd_unregister_protocol(struct kbd_protocol *protocol);
void kbd_init_interface(struct kbd_interface *interface, struct base_driver *driver);
