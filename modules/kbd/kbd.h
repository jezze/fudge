struct kbd_interface_node
{

    struct system_group root;

};

struct kbd_interface
{

    struct kbd_interface_node node;
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
void kbd_init_protocol(struct kbd_protocol *protocol, char *name);
