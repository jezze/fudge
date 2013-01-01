#define SYSTEM_NODE_TYPE_NONE           0
#define SYSTEM_NODE_TYPE_GROUP          1
#define SYSTEM_NODE_TYPE_INTEGER        2
#define SYSTEM_NODE_TYPE_STRING         3
#define SYSTEM_NODE_TYPE_STREAM         4

struct system_node
{

    unsigned int type;
    char *name;
    struct system_node *parent;
    struct system_node *next;

};

struct system_group
{

    struct system_node base;
    struct system_node *children;

};

struct system_integer
{

    struct system_node base;
    int value;

};

struct system_string
{

    struct system_node base;
    char *value;

};

struct system_stream
{

    struct system_node base;
    unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(unsigned int offset, unsigned int count, void *buffer);

};

void system_group_add(struct system_group *group, struct system_node *node);
void system_group_remove(struct system_group *group, struct system_node *node);
void system_register_node(struct system_node *node);
void system_unregister_node(struct system_node *node);
void system_init_group(struct system_group *group, char *name);
void system_init_integer(struct system_integer *integer, char *name, int value);
void system_init_string(struct system_string *string, char *name, char *value);
void system_init_stream(struct system_stream *stream, char *name, unsigned int (*read)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(unsigned int offset, unsigned int count, void *buffer));
