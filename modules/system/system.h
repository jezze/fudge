#define SYSTEM_NODE_TYPE_NONE           0
#define SYSTEM_NODE_TYPE_GROUP          1
#define SYSTEM_NODE_TYPE_STREAM         2

struct system_node
{

    struct system_node *parent;
    struct system_node *sibling;
    unsigned int type;
    char *name;

};

struct system_group
{

    struct system_node node;
    struct system_node *children;

};

struct system_stream
{

    struct system_node node;
    unsigned int (*read)(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer);

};

void system_group_add(struct system_group *group, struct system_node *node);
void system_group_remove(struct system_group *group, struct system_node *node);
void system_register_node(struct system_node *node);
void system_unregister_node(struct system_node *node);
void system_init_group(struct system_group *group, char *name);
void system_init_stream(struct system_stream *stream, char *name, unsigned int (*read)(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer));
