#include <kernel.h>
#include "resource.h"

static struct list resources;

struct resource_list *resource_find_list(enum resource_type type)
{

    struct list_item *current;

    for (current = resources.head; current; current = current->next)
    {

        struct resource_list *list = current->data;

        if (list->id.type == type)
            return list;

    }

    return 0;

}

void resource_register_item(struct resource_item *item, enum resource_type type)
{

    struct resource_list *list = resource_find_list(type);

    if (!list)
        return;

    list_add(&list->list, &item->item);

}

void resource_register_list(struct resource_list *list)
{

    list_add(&resources, &list->item);

}

void resource_unregister_item(struct resource_item *item, enum resource_type type)
{

    struct resource_list *list = resource_find_list(type);

    if (!list)
        return;

    list_remove(&list->list, &item->item);

}

void resource_unregister_list(struct resource_list *list)
{

    list_remove(&resources, &list->item);

}

void resource_init_item(struct resource_item *item, void *data, enum resource_type type, unsigned int size, const char *text)
{

    memory_clear(item, sizeof (struct resource_item));
    list_init_item(&item->item, data);

    item->id.type = type;
    item->id.size = size;
    item->id.text = text;

}

void resource_init_list(struct resource_list *list, enum resource_type type, unsigned int size, const char *text)
{

    memory_clear(list, sizeof (struct resource_list));
    list_init_item(&list->item, list);
    list_init(&list->list);

    list->id.type = type;
    list->id.size = size;
    list->id.text = text;

}

struct list *resource_setup()
{

    list_init(&resources);

    return &resources;

}

