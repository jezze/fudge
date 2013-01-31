struct kbd_interface
{

    struct base_driver *driver;
    unsigned int (*read)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct kbd_interface_node
{

    struct system_group root;
    struct kbd_interface *interface;

};

struct kbd_protocol
{

    char *name;

};

struct kbd_protocol_node
{

    struct system_group root;
    struct kbd_protocol *protocol;

};

void kbd_register_interface(struct kbd_interface *interface);
void kbd_register_protocol(struct kbd_protocol *protocol);
void kbd_unregister_interface(struct kbd_interface *interface);
void kbd_unregister_protocol(struct kbd_protocol *protocol);
void kbd_init_interface(struct kbd_interface *interface, struct base_driver *driver, unsigned int (*read)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct kbd_interface *self, unsigned int offset, unsigned int count, void *buffer));
void kbd_init_protocol(struct kbd_protocol *protocol, char *name);
