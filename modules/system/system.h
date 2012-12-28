#define SYSTEM_NODE_TYPE_NONE           0
#define SYSTEM_NODE_TYPE_GROUP          1

struct system_node
{

    unsigned int type;
    char *name;
    struct system_node *next;

};

struct system_group
{

    struct system_node base;

};

void system_register_group(struct system_group *group);
void system_unregister_group(struct system_group *group);
void system_init_node(struct system_node *node, unsigned int type, char *name);
void system_init_group(struct system_group *group, char *name);
