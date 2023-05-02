#define SYSTEM_NODETYPE_NORMAL          0
#define SYSTEM_NODETYPE_GROUP           1
#define SYSTEM_NODETYPE_MULTIGROUP      2

struct system_operations
{

    unsigned int (*create)(void *name, unsigned int length);
    unsigned int (*destroy)(void);
    unsigned int (*read)(void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*notify)(struct list *links, unsigned int source, unsigned int event, unsigned int count, void *data);

};

struct system_node
{

    char *name;
    unsigned int type;
    unsigned int index;
    struct system_node *parent;
    struct system_operations operations;
    struct list_item item;
    struct list children;
    struct list links;

};

void system_addchild(struct system_node *group, struct system_node *node);
void system_removechild(struct system_node *group, struct system_node *node);
void system_registernode(struct system_node *node);
void system_unregisternode(struct system_node *node);
void system_initnode(struct system_node *node, unsigned int type, char *name);
