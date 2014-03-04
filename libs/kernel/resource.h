struct resource_item
{

    struct list_item item;

};

struct resource_list
{

    struct list_item item;
    struct list list;
    char *name;

};

struct resource_list *resource_find_list(unsigned int count, char *name);
void resource_register_item(struct resource_item *item, unsigned int count, char *name);
void resource_register_list(struct resource_list *list);
void resource_init_item(struct resource_item *item, void *data);
void resource_init_list(struct resource_list *list, char *name);
