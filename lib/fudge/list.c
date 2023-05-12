#include "buffer.h"
#include "spinlock.h"
#include "list.h"

void list_add_unsafe(struct list *list, struct list_item *item)
{

    item->prev = list->tail;
    item->next = 0;

    if (list->head)
        list->tail->next = item;
    else
        list->head = item;

    list->tail = item;
    list->count++;

}

void list_add(struct list *list, struct list_item *item)
{

    spinlock_acquire(&list->spinlock);
    list_add_unsafe(list, item);
    spinlock_release(&list->spinlock);

}

void list_remove_unsafe(struct list *list, struct list_item *item)
{

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

    item->next = 0;
    item->prev = 0;
    list->count--;

}

void list_remove(struct list *list, struct list_item *item)
{

    spinlock_acquire(&list->spinlock);
    list_remove_unsafe(list, item);
    spinlock_release(&list->spinlock);

}

void list_move_unsafe(struct list *to, struct list *from, struct list_item *item)
{

    list_remove_unsafe(from, item);
    list_add_unsafe(to, item);

}

void list_move(struct list *to, struct list *from, struct list_item *item)
{

    if (to == from)
    {

        spinlock_acquire(&to->spinlock);
        list_move_unsafe(to, from, item);
        spinlock_release(&to->spinlock);

    }

    else
    {

        spinlock_acquire(&from->spinlock);
        spinlock_acquire(&to->spinlock);
        list_move_unsafe(to, from, item);
        spinlock_release(&to->spinlock);
        spinlock_release(&from->spinlock);

    }

}

struct list_item *list_pickhead(struct list *list)
{

    struct list_item *item;

    spinlock_acquire(&list->spinlock);

    item = list->head;

    if (item)
        list_remove_unsafe(list, item);

    spinlock_release(&list->spinlock);

    return item;

}

struct list_item *list_picktail(struct list *list)
{

    struct list_item *item;

    spinlock_acquire(&list->spinlock);

    item = list->tail;

    if (item)
        list_remove_unsafe(list, item);

    spinlock_release(&list->spinlock);

    return item;

}

void list_inititem(struct list_item *item, void *data)
{

    item->next = 0;
    item->prev = 0;
    item->data = data;

}

void list_init(struct list *list)
{

    spinlock_init(&list->spinlock);

    list->head = 0;
    list->tail = 0;
    list->count = 0;

}

