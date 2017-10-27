#include "memory.h"
#include "spinlock.h"
#include "list.h"

static void add(struct list *list, struct list_item *item)
{

    if (item->list)
        return;

    item->list = list;
    item->prev = list->tail;
    item->next = 0;

    if (list->head)
        list->tail->next = item;
    else
        list->head = item;

    list->tail = item;
    list->count++;

}

static void remove(struct list *list, struct list_item *item)
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

void list_add(struct list *list, struct list_item *item)
{

    spinlock_acquire(&list->spinlock);
    add(list, item);
    spinlock_release(&list->spinlock);

}

void list_remove(struct list *list, struct list_item *item)
{

    spinlock_acquire(&list->spinlock);
    remove(list, item);
    spinlock_release(&list->spinlock);

}

void list_move(struct list *list, struct list_item *item)
{

    if (item->list)
        list_remove(item->list, item);

    list_add(list, item);

}

struct list_item *list_pickhead(struct list *list)
{

    struct list_item *item;

    spinlock_acquire(&list->spinlock);

    item = list->head;

    if (item && item->list)
        remove(item->list, item);

    spinlock_release(&list->spinlock);

    return item;

}

struct list_item *list_picktail(struct list *list)
{

    struct list_item *item;

    spinlock_acquire(&list->spinlock);

    item = list->tail;

    if (item && item->list)
        remove(item->list, item);

    spinlock_release(&list->spinlock);

    return item;

}

void list_inititem(struct list_item *item, void *data)
{

    item->list = 0;
    item->next = 0;
    item->prev = 0;
    item->data = data;

}

