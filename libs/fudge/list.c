#include "memory.h"
#include "list.h"

void list_add(struct list *list, struct list_item *item)
{

    if (list->head == 0)
    {

        list->head = item;
        list->tail = item;
        item->next = 0;
        item->prev = 0;

        return;

    }

    list->tail->next = item;
    item->next = 0;
    item->prev = list->tail;
    list->tail = item;

}

void list_remove(struct list *list, struct list_item *item)
{

    if (!list->head)
        return;

    if (list->head == list->tail && list->head == item)
    {

        list->head = 0;
        list->tail = 0;
        item->next = 0;
        item->prev = 0;

        return;

    }

    if (list->head == item)
    {

        list->head = item->next;
        list->head->prev = 0;

    }

    else if (list->tail == item)
    {

        list->tail = list->tail->prev;
        list->tail->next = 0;

    }

    else
    {

        struct list_item *n = item->next;
        struct list_item *p = item->prev;

        n->prev = item->prev;
        p->next = item->next;

    }

    item->next = 0;
    item->prev = 0;

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

