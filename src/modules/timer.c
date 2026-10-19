#include <fudge.h>
#include <kernel.h>
#include "timer.h"

static struct node_operands operands;

static unsigned int getticks(struct timer_interface *interface, unsigned int ms)
{

    unsigned int ticks = ms / 1000 * interface->hz + ((ms % 1000) * interface->hz + 999) / 1000;

    return (ticks) ? ticks : 1;

}

static struct list *getslot(struct timer_interface *interface, unsigned int expires)
{

    return &interface->slots[expires % TIMER_SLOTS];

}

static struct timer_entry *findentry(struct timer_interface *interface, unsigned int target, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < TIMER_ENTRIES; i++)
    {

        struct timer_entry *entry = &interface->entries[i];

        if (entry->target == target && entry->id == id)
            return entry;

    }

    return 0;

}

static void schedule(struct timer_interface *interface, struct timer_entry *entry, unsigned int expires)
{

    struct list *from = getslot(interface, entry->expires);

    entry->expires = expires;

    list_move_unsafe(getslot(interface, expires), from, &entry->item);

}

static void release(struct timer_interface *interface, struct timer_entry *entry)
{

    list_move_unsafe(&interface->free, getslot(interface, entry->expires), &entry->item);

    entry->target = 0;

}

static unsigned int request(struct timer_interface *interface, unsigned int source, struct event_timerrequest *timerrequest)
{

    struct timer_entry *entry;
    unsigned int status = MESSAGE_OK;

    spinlock_acquire(&interface->spinlock);

    entry = findentry(interface, source, timerrequest->id);

    if (entry)
        release(interface, entry);

    if (timerrequest->ms)
    {

        struct list_item *item = interface->free.head;

        if (item)
        {

            unsigned int ticks = getticks(interface, timerrequest->ms);

            entry = item->data;
            entry->target = source;
            entry->id = timerrequest->id;
            entry->expires = interface->jiffies + ticks;
            entry->period = (timerrequest->repeat) ? ticks : 0;

            list_move_unsafe(getslot(interface, entry->expires), &interface->free, item);

        }

        else
        {

            status = MESSAGE_FAILED;

        }

    }

    spinlock_release(&interface->spinlock);

    return status;

}

static void unlink(struct timer_interface *interface, unsigned int source)
{

    unsigned int i;

    spinlock_acquire(&interface->spinlock);

    for (i = 0; i < TIMER_ENTRIES; i++)
    {

        struct timer_entry *entry = &interface->entries[i];

        if (entry->target == source)
            release(interface, entry);

    }

    spinlock_release(&interface->spinlock);

}

static void expire(struct timer_interface *interface, unsigned int counter)
{

    struct list *slot = getslot(interface, counter);
    struct list_item *current;
    struct list_item *next;

    spinlock_acquire(&interface->spinlock);

    interface->jiffies = counter;

    for (current = slot->head; current; current = next)
    {

        struct timer_entry *entry = current->data;
        struct event_timerexpire timerexpire;

        next = current->next;

        if ((int)(entry->expires - counter) > 0)
            continue;

        timerexpire.id = entry->id;

        switch (kernel_place(interface->service.inode, entry->target, EVENT_TIMEREXPIRE, sizeof (struct event_timerexpire), &timerexpire))
        {

        case MESSAGE_OK:
            if (entry->period)
                schedule(interface, entry, counter + entry->period);
            else
                release(interface, entry);

            break;

        case MESSAGE_RETRY:
            schedule(interface, entry, counter + 1);

            break;

        default:
            release(interface, entry);

            break;

        }

    }

    spinlock_release(&interface->spinlock);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct timer_interface *interface = resource->data;

        switch (event)
        {

        case EVENT_LINK:
            return kernel_linknode(&interface->service.links, source);

        case EVENT_UNLINK:
            unlink(interface, source);

            return kernel_unlinknode(&interface->service.links, source);

        case EVENT_TIMERREQUEST:
            if (count < sizeof (struct event_timerrequest))
                return MESSAGE_FAILED;

            return request(interface, source, data);

        }

    }

    return MESSAGE_FAILED;

}

void timer_notifytick(struct timer_interface *interface, unsigned int counter)
{

    struct event_timertick timertick;

    timertick.counter = counter;

    kernel_notify(&interface->service.links, interface->service.inode, EVENT_TIMERTICK, sizeof (struct event_timertick), &timertick);
    expire(interface, counter);

}

void timer_registerinterface(struct timer_interface *interface)
{

    unsigned int inode = pool_picknode(&interface->resource, &operands);

    resource_register(&interface->resource);
    service_register(&interface->service, inode, "timer");

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    unsigned int inode = interface->service.inode;

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);
    pool_unpicknode(inode);

}

void timer_initinterface(struct timer_interface *interface, unsigned int id, unsigned int hz)
{

    unsigned int i;

    resource_init(&interface->resource, RESOURCE_TIMERINTERFACE, interface);
    service_init(&interface->service);
    spinlock_init(&interface->spinlock);
    list_init(&interface->free);

    for (i = 0; i < TIMER_SLOTS; i++)
        list_init(&interface->slots[i]);

    for (i = 0; i < TIMER_ENTRIES; i++)
    {

        list_inititem(&interface->entries[i].item, &interface->entries[i]);
        list_add_unsafe(&interface->free, &interface->entries[i].item);

    }

    interface->id = id;
    interface->hz = hz;

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place, 0);

}

