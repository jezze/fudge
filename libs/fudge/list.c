#include "memory.h"
#include "list.h"

void list_add(struct list *list, struct list_item *item)
{

    if (!list->head)
    {

        list->head = item;
        list->tail = item;

        return;

    }

    item->prev = list->tail;
    item->next = 0;
    list->tail->next = item;
    list->tail = item;

}

void list_remove(struct list *list, struct list_item *item)
{

    if (!list->head)
        return;

    if (list->head == item)
        list->head = item->next;

    if (list->tail == item)
        list->tail = item->prev;

    if (list->head)
        list->head->prev = 0;

    if (list->tail)
        list->tail->next = 0;

    item->next = 0;
    item->prev = 0;

}

void list_init_item(struct list_item *item, void *data)
{

    memory_clear(item, sizeof (struct list_item));

    item->data = data;

}

void list_init(struct list *list)
{

    memory_clear(list, sizeof (struct list));

}

