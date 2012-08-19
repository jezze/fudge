#ifndef MODULES_NODEFS_H
#define MODULES_NODEFS_H

struct nodefs_node
{

    char *name;
    struct modules_base *module;
    unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer);

};

struct nodefs_filesystem
{

    struct modules_filesystem base;
    struct nodefs_node *nodes[128];
    unsigned int nodesCount;
    void (*register_node)(struct nodefs_filesystem *self, struct nodefs_node *node, char *name, struct modules_base *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer));
    void (*unregister_node)(struct nodefs_filesystem *self, struct nodefs_node *node);

};

void nodefs_register_node(struct nodefs_node *node, char *name, struct modules_base *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer));
void nodefs_unregister_node(struct nodefs_node *node);
void nodefs_filesystem_init(struct nodefs_filesystem *filesystem);

#endif

