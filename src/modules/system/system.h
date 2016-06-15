#define SYSTEM_NODETYPE_NORMAL          0
#define SYSTEM_NODETYPE_GROUP           1
#define SYSTEM_NODETYPE_MULTI           2
#define SYSTEM_NODETYPE_MERGE           4
#define SYSTEM_NODETYPE_MAILBOX         8

struct system_header
{

    char id[12];
    unsigned int root;

};

struct system_node
{

    struct list_item item;
    struct system_node *parent;
    struct list children;
    struct list links;
    unsigned int type;
    char *name;
    struct resource *resource;
    unsigned int refcount;
    unsigned int index;
    unsigned int (*read)(struct system_node *self, struct service_state *state, unsigned int count, void *buffer);
    unsigned int (*write)(struct system_node *self, struct service_state *state, unsigned int count, void *buffer);
    unsigned int (*child)(struct system_node *self, unsigned int count, char *path);

};

unsigned int system_send(unsigned int id, unsigned int count, void *buffer);
void system_multicast(struct list *list, unsigned int count, void *buffer);
void system_wakeup(struct list *list);
void system_addchild(struct system_node *group, struct system_node *node);
void system_removechild(struct system_node *group, struct system_node *node);
void system_registernode(struct system_node *node);
void system_unregisternode(struct system_node *node);
void system_initbackend(struct service_backend *backend);
void system_initprotocol(struct service_protocol *protocol);
void system_initnode(struct system_node *node, unsigned int type, char *name);
