#include <fudge.h>
#include <kernel.h>

static struct debug_interface debuginterface;
static struct server server;
static struct service service;

static unsigned int islinked(unsigned int source)
{

    struct list *links = &server.resource.links;
    struct list_item *current;
    unsigned int linked = 0;

    spinlock_acquire(&links->spinlock);

    for (current = links->head; current; current = current->next)
    {

        if (pool_getlinktarget(current) == source)
            linked = 1;

    }

    spinlock_release(&links->spinlock);

    return linked;

}

static void notify(unsigned int source, unsigned int level, unsigned int count, void *data)
{

    struct {struct message_loginfo loginfo; char data[MESSAGE_SIZE - sizeof (struct message_loginfo)];} message;

    message.loginfo.level = level;
    message.loginfo.count = buffer_write(message.data, sizeof (message.data), data, count, 0);

    if (!islinked(source))
        kernel_notify(&server.resource, MESSAGE_TYPE_LOGINFO, sizeof (struct message_loginfo) + message.loginfo.count, &message);

}

static void debuginterface_write(unsigned int level, unsigned int count, char *string, char *file, unsigned int line)
{

    notify(0, level, count, string);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case MESSAGE_TYPE_LINK:
        return kernel_linknode(&server.resource, source);

    case MESSAGE_TYPE_UNLINK:
        return kernel_unlinknode(&server.resource, source);

    case MESSAGE_TYPE_DATA:
        notify(source, DEBUG_INFO, count, data);

        return MESSAGE_OK;

    case MESSAGE_TYPE_ERROR:
        notify(source, DEBUG_ERROR, count, data);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

void module_init(void)
{

    debug_initinterface(&debuginterface, debuginterface_write);
    server_init(&server, 0, operands_place, 0);
    service_init(&service, "log");

}

void module_register(void)
{

    debug_registerinterface(&debuginterface);
    service_register(&service, pool_attachnode(&server.resource));

}

void module_unregister(void)
{

    service_unregister(&service);
    pool_detachnode(&server.resource);
    debug_unregisterinterface(&debuginterface);

}

