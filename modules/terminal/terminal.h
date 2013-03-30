struct terminal_interface
{

    struct base_driver *driver;
    unsigned int (*read)(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct terminal_protocol
{

    char *name;

};

void terminal_register_interface(struct terminal_interface *interface);
void terminal_register_protocol(struct terminal_protocol *protocol);
void terminal_unregister_interface(struct terminal_interface *interface);
void terminal_unregister_protocol(struct terminal_protocol *protocol);
void terminal_init_interface(struct terminal_interface *interface, struct base_driver *driver, unsigned int (*read)(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer));
void terminal_init_protocol(struct terminal_protocol *protocol, char *name);
