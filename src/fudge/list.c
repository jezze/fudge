#include "memory.h"
#include "list.h"

void list_add(struct list *list, struct list_item *item)
{

    if (item->list)
        return;

    if (!list->head)
    {

        item->list = list;
        item->prev = 0;
        item->next = 0;
        list->head = item;
        list->tail = item;
        list->count++;

        return;

    }

    item->list = list;
    item->prev = list->tail;
    item->next = 0;
    list->tail->next = item;
    list->tail = item;
    list->count++;

}

void list_remove(struct list *list, struct list_item *item)
{

    if (item->list != list)
        return;

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

    if (item->next)
        item->next->prev = item->prev;

    if (item->prev)
        item->prev->next = item->next;

    item->list = 0;
    item->next = 0;
    item->prev = 0;
    list->count--;

}

void list_move(struct list *list, struct list_item *item)
{

    if (item->list)
        list_remove(item->list, item);

    list_add(list, item);

}

void list_inititem(struct list_item *item, void *data)
{

    item->list = 0;
    item->next = 0;
    item->prev = 0;
    item->data = data;

}

