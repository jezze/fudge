enum system_nodetype
{

    SYSTEM_NODETYPE_NONE                = 0,
    SYSTEM_NODETYPE_GROUP               = 1,
    SYSTEM_NODETYPE_STREAM              = 2

};

struct system_header
{

    char id[12];
    unsigned int root;

};

struct system_node
{

    struct system_node *parent;
    struct system_node *sibling;
    enum system_nodetype type;
    char *name;
    unsigned int (*open)(struct system_node *self);
    unsigned int (*close)(struct system_node *self);
    unsigned int (*read)(struct system_node *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct system_node *self, unsigned int offset, unsigned int count, void *buffer);

};

struct system_group
{

    struct system_node node;
    struct system_node *children;

};

struct system_stream
{

    struct system_node node;

};

void system_group_add(struct system_group *group, struct system_node *node);
void system_group_remove(struct system_group *group, struct system_node *node);
void system_register_node(struct system_node *node);
void system_unregister_node(struct system_node *node);
void system_init_group(struct system_group *group, char *name);
void system_init_stream(struct system_stream *stream, char *name);
