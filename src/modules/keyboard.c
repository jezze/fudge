#include <fudge.h>
#include <kernel.h>
#include "keyboard.h"

static struct unitsrv unitsrv;

static struct keyboard_subscriber *findsubscriber(struct keyboard_interface *interface, unsigned int source)
{

    struct mailbox *mailbox = pool_getnodemailbox(source);
    unsigned int i;

    for (i = 0; mailbox && i < KEYBOARD_SUBSCRIBERS; i++)
    {

        struct mailbox *subscriber = pool_getnodemailbox(interface->subscribers[i].target);

        if (subscriber && subscriber->itask == mailbox->itask)
            return &interface->subscribers[i];

    }

    return 0;

}

static unsigned int subscribe(struct keyboard_interface *interface, unsigned int source, struct message_link *link)
{

    unsigned int i;

    spinlock_acquire(&interface->spinlock);

    for (i = 0; i < KEYBOARD_SUBSCRIBERS; i++)
    {

        struct keyboard_subscriber *subscriber = &interface->subscribers[i];

        /* a subscriber whose task is gone has a renewed inode that no longer finds a mailbox */
        if (!pool_getnodemailbox(subscriber->target))
        {

            subscriber->target = source;
            subscriber->bit = link->bit;
            subscriber->next = interface->head;

            break;

        }

    }

    spinlock_release(&interface->spinlock);

    return (i < KEYBOARD_SUBSCRIBERS) ? MESSAGE_OK : kernel_linknode(&interface->resource, source);

}

static unsigned int unsubscribe(struct keyboard_interface *interface, unsigned int source)
{

    struct keyboard_subscriber *subscriber;

    spinlock_acquire(&interface->spinlock);

    subscriber = findsubscriber(interface, source);

    if (subscriber)
        subscriber->target = 0;

    spinlock_release(&interface->spinlock);

    return (subscriber) ? MESSAGE_OK : kernel_unlinknode(&interface->resource, source);

}

static unsigned int sendevents(struct keyboard_interface *interface, unsigned int source)
{

    char data[MESSAGE_SIZE];
    struct message_eventsresponse *response = (struct message_eventsresponse *)data;
    unsigned int offset = sizeof (struct message_eventsresponse);
    struct keyboard_subscriber *subscriber;

    response->count = 0;
    response->lost = 0;

    spinlock_acquire(&interface->spinlock);

    subscriber = findsubscriber(interface, source);

    if (subscriber)
    {

        if (interface->head - subscriber->next > KEYBOARD_RING)
        {

            response->lost = interface->head - subscriber->next - KEYBOARD_RING;
            subscriber->next = interface->head - KEYBOARD_RING;

        }

        while (subscriber->next != interface->head)
        {

            struct keyboard_entry *entry = &interface->ring[subscriber->next % KEYBOARD_RING];
            struct message_eventsentry *header = (struct message_eventsentry *)(data + offset);

            header->event = entry->event;
            header->length = sizeof (struct message_keypress);
            offset += sizeof (struct message_eventsentry);
            data[offset] = entry->scancode;
            offset += 4;

            response->count++;
            subscriber->next++;

        }

    }

    spinlock_release(&interface->spinlock);

    return kernel_placereply(source, interface->resource.inode, MESSAGE_TYPE_EVENTSRESPONSE, offset, data);

}

static void record(struct keyboard_interface *interface, unsigned int event, unsigned char scancode)
{

    struct keyboard_entry *entry;
    unsigned int i;

    spinlock_acquire(&interface->spinlock);

    entry = &interface->ring[interface->head % KEYBOARD_RING];
    entry->event = event;
    entry->scancode = scancode;
    interface->head++;

    spinlock_release(&interface->spinlock);

    for (i = 0; i < KEYBOARD_SUBSCRIBERS; i++)
    {

        struct mailbox *mailbox = pool_getnodemailbox(interface->subscribers[i].target);

        if (mailbox)
            kernel_raise(mailbox->itask, 1u << interface->subscribers[i].bit);

    }

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct keyboard_interface *interface = resource->data;

        switch (event)
        {

        case MESSAGE_TYPE_LINK:
            return (count == sizeof (struct message_link)) ? subscribe(interface, source, data) : kernel_linknode(resource, source);

        case MESSAGE_TYPE_UNLINK:
            return unsubscribe(interface, source);

        case MESSAGE_TYPE_EVENTSREQUEST:
            return sendevents(interface, source);

        }

    }

    return MESSAGE_FAILED;

}

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode)
{

    struct message_keypress keypress;

    keypress.scancode = scancode;

    record(interface, MESSAGE_TYPE_KEYPRESS, scancode);
    kernel_notify(&interface->resource, MESSAGE_TYPE_KEYPRESS, sizeof (struct message_keypress), &keypress);

}

void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode)
{

    struct message_keyrelease keyrelease;

    keyrelease.scancode = scancode;

    record(interface, MESSAGE_TYPE_KEYRELEASE, scancode);
    kernel_notify(&interface->resource, MESSAGE_TYPE_KEYRELEASE, sizeof (struct message_keyrelease), &keyrelease);

}

void keyboard_registerinterface(struct keyboard_interface *interface)
{

    resource_register(&interface->resource);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    resource_unregister(&interface->resource);

}

void keyboard_initinterface(struct keyboard_interface *interface)
{

    unsigned int i;

    resource_init(&interface->resource, RESOURCE_KEYBOARDINTERFACE, interface, operands_place);
    spinlock_init(&interface->spinlock);

    interface->head = 0;

    for (i = 0; i < KEYBOARD_SUBSCRIBERS; i++)
        interface->subscribers[i].target = 0;

}

void module_init(void)
{

    unitsrv_init(&unitsrv, RESOURCE_KEYBOARDINTERFACE, "keyboard");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

