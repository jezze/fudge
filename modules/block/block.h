struct block_interface_node
{

    struct system_group root;

};

struct block_interface
{

    struct block_interface_node node;
    struct base_driver *driver;
    unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct block_protocol
{

    char *name;

};

void block_register_interface(struct block_interface *interface);
void block_register_protocol(struct block_protocol *protocol);
void block_unregister_interface(struct block_interface *interface);
void block_unregister_protocol(struct block_protocol *protocol);
void block_init_interface(struct block_interface *interface, struct base_driver *driver, unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer));
void block_init_protocol(struct block_protocol *protocol, char *name);
