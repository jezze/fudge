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

    for (current = list->head; current; current = current->next)
    {

        if (current->next == 0)
        {

            current->next = item;

            break;

        }

    }

}

void list_remove(struct list *list, struct list_item *item)
{

    struct list_item *current;

    if (list->head == item)
    {

        list->head = item->next;

        return;

    }

    for (current = list->head; current; current = current->next)
    {

        if (current->next == item)
        {

            current->next = current->next->next;

            break;

        }

    }

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

