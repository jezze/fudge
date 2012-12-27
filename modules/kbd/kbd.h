struct kbd_interface
{

    struct base_driver *driver;

};

struct kbd_protocol
{

    char *name;

};

struct kbd_group
{

    struct system_group base;
    struct kbd_interface *interfaces;
    struct kbd_protocol *protocols;

};

void kbd_register_interface(struct kbd_interface *interface);
void kbd_register_protocol(struct kbd_protocol *protocol);
void kbd_unregister_interface(struct kbd_interface *interface);
void kbd_unregister_protocol(struct kbd_protocol *protocol);
void kbd_init_group(struct kbd_group *group);
void kbd_init_interface(struct kbd_interface *interface, struct base_driver *driver);
