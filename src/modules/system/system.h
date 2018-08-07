#define SYSTEM_NODETYPE_NORMAL          0
#define SYSTEM_NODETYPE_MAILBOX         1
#define SYSTEM_NODETYPE_GROUP           2
#define SYSTEM_NODETYPE_MULTIGROUP      3

struct system_header
{

    char id[12];
    unsigned int root;

};

struct system_operations
{

    struct system_node *(*open)(struct system_node *self, struct service_state *state);
    struct system_node *(*close)(struct system_node *self, struct service_state *state);
    unsigned int (*read)(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*seek)(struct system_node *self, struct service_state *state, unsigned int offset);

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
    struct list states;

};

void system_addchild(struct system_node *group, struct system_node *node);
void system_removechild(struct system_node *group, struct system_node *node);
void system_movechild(struct system_node *group, struct system_node *node);
void system_registernode(struct system_node *node);
void system_unregisternode(struct system_node *node);
void system_initbackend(void);
void system_initprotocol(void);
void system_initnode(struct system_node *node, unsigned int type, char *name);
