#include <kernel.h>
#include "resource.h"

static struct list lists;

struct resource_list *resource_find_list(unsigned int count, char *name)
{

    struct list_item *current;

    for (current = lists.head; current; current = current->next)
    {

        struct resource_list *list = current->data;

        if (memory_match(list->name, name, count))
            return list;

    }

    return 0;

}

void resource_register_item(struct resource_item *item, unsigned int count, char *name)
{

    struct resource_list *list = resource_find_list(count, name);

    if (!list)
        return;

    list_add(&list->list, &item->item);

}

void resource_register_list(struct resource_list *list)
{

    list_add(&lists, &list->item);

}

void resource_init_item(struct resource_item *item, void *data)
{

    memory_clear(item, sizeof (struct resource_item));
    list_init_item(&item->item, data);

}

void resource_init_list(struct resource_list *list, char *name)
{

    memory_clear(list, sizeof (struct resource_list));
    list_init_item(&list->item, list);
    list_init(&list->list);

    list->name = name;

}

