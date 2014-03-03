struct resource
{

    struct list_item item;

};

void resource_register_container(struct resource *resource);
void resource_register_task(struct resource *resource);
void resource_init(struct resource *resource, void *data);
