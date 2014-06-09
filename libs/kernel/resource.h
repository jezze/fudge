struct resource
{

    struct list_item item;
    unsigned int type;
    void *data;

};

struct resource *resource_find(struct resource *resource);
void resource_register(struct resource *resource);
void resource_unregister(struct resource *resource);
void resource_init(struct resource *resource, unsigned int type, void *data);
void resource_setup();
