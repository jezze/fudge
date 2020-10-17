#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node wserver;
static struct system_node wclient;

static unsigned int multicast(struct service_state *source, struct list *targets, struct message_header *header, void *data)
{

    struct list_item *current;

    spinlock_acquire(&targets->spinlock);

    for (current = targets->head; current; current = current->next)
    {

        struct service_state *target = current->data;

        header->source = source->id;

        kernel_place(target->id, header, data);

    }

    spinlock_release(&targets->spinlock);

    return header->length;

}

static unsigned int wserver_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

static unsigned int wclient_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct message_header *header = buffer;

    return multicast(state, &wserver.states, header, header + 1);

}

static unsigned int wclient_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

void module_init(void)
{

    system_initnode(&wserver, SYSTEM_NODETYPE_NORMAL, "wserver");

    wserver.operations.seek = wserver_seek;

    system_initnode(&wclient, SYSTEM_NODETYPE_NORMAL, "wclient");

    wclient.operations.write = wclient_write;
    wclient.operations.seek = wclient_seek;

}

void module_register(void)
{

    system_registernode(&wserver);
    system_registernode(&wclient);

}

void module_unregister(void)
{

    system_unregisternode(&wserver);
    system_unregisternode(&wclient);

}

