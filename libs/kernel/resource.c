#include <kernel.h>
#include "resource.h"

static struct list resources;
static struct resource_item res;

struct resource_item *resource_find_item(enum resource_type type)
{

    struct list_item *current;

    for (current = resources.head; current; current = current->next)
    {

        struct resource_item *item = current->data;

        if (item->id.type == type)
            return item;

    }

    return 0;

}

void resource_register_item(struct resource_item *item)
{

    list_add(&resources, &item->item);

}

void resource_init_item(struct resource_item *item, void *data, enum resource_type type, unsigned int size, const char *text)
{

    memory_clear(item, sizeof (struct resource_item));
    list_init_item(&item->item, data);

    item->id.type = type;
    item->id.size = size;
    item->id.text = text;

}

void resource_setup()
{

    list_init(&resources);
    resource_init_item(&res, &resources, RESOURCE_TYPE_ALL, 4, "root");
    resource_register_item(&res);

}

