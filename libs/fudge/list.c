#include "memory.h"
#include "list.h"

void list_add(struct list *list, struct list_item *item)
{

    struct list_item *current;

    if (list->head == 0)
    {

        list->head = item;

        return;

    }

    for (current = list->head; current->next != 0; current = current->next);

    current->next = item;

}

void list_remove(struct list *list, struct list_item *item)
{

    struct list_item *current;

    if (list->head == item)
    {

        list->head = item->next;

        return;

    }

    for (current = list->head; current->next != item; current = current->next);

    current->next = current->next->next;

}

void list_init_item(struct list_item *item, void *self)
{

    memory_clear(item, sizeof (struct list_item));

    item->self = self;

}

void list_init(struct list *list)
{

    memory_clear(list, sizeof (struct list));

}

