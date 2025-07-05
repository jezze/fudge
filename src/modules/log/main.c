#include <fudge.h>
#include <kernel.h>

static struct debug_interface debuginterface;

static void debuginterface_write(unsigned int level, unsigned int count, char *string, char *file, unsigned int line)
{

    union {struct event_loginfo loginfo; char data[MESSAGE_SIZE];} message;

    message.loginfo.level = level;
    message.loginfo.count = sizeof (struct event_loginfo);

    if (file)
        message.loginfo.count += cstring_write_fmt4(message.data, MESSAGE_SIZE, message.loginfo.count, "%w (%s:%u)", string, &count, file, &line);
    else
        message.loginfo.count += cstring_write_fmt2(message.data, MESSAGE_SIZE, message.loginfo.count, "%w", string, &count);

    /*
    kernel_notify(&node, &debuginterface.resource.targets, EVENT_LOGINFO, message.loginfo.count, &message);
    */

}

void module_init(void)
{

    debug_initinterface(&debuginterface, debuginterface_write);

}

void module_register(void)
{

    debug_registerinterface(&debuginterface);

}

void module_unregister(void)
{

    debug_unregisterinterface(&debuginterface);

}

