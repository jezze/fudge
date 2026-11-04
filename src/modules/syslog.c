#include <fudge.h>
#include <kernel.h>

static struct debug_interface debuginterface;
static struct server server;
static struct service service;

static void debuginterface_write(unsigned int level, unsigned int count, char *string, char *file, unsigned int line)
{

    struct {struct message_loginfo loginfo; char data[DEBUG_MESSAGESIZE];} message;

    message.loginfo.level = level;
    message.loginfo.count = buffer_write(message.data, DEBUG_MESSAGESIZE, string, count, 0);

    kernel_notify(&server.resource, MESSAGE_TYPE_LOGINFO, sizeof (struct message_loginfo) + message.loginfo.count, &message);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case MESSAGE_TYPE_LINK:
        return kernel_linknode(&server.resource, source);

    case MESSAGE_TYPE_UNLINK:
        return kernel_unlinknode(&server.resource, source);

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

