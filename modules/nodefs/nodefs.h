struct nodefs_node
{

    char *name;
    struct base_module *module;
    struct nodefs_node *next;
    unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer);

};

void nodefs_register_node(struct nodefs_node *node, char *name, struct base_module *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer));
void nodefs_unregister_node(struct nodefs_node *node);
