struct resource_item
{

    struct list_item item;
    unsigned int type;
    void *data;

};

struct resource_item *resource_find_item(struct resource_item *item);
void resource_register_item(struct resource_item *item);
void resource_init_item(struct resource_item *item, unsigned int type, void *data);
void resource_setup();
