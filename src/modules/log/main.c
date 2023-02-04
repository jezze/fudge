#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct debug_interface interface;
static struct system_node root;
static struct system_node messages;

static void interface_write(unsigned int level, char *string, char *file, unsigned int line)
{

    union {struct event_loginfo loginfo; char data[MESSAGE_SIZE];} message;

    message.loginfo.level = level;
    message.loginfo.count = sizeof (struct event_loginfo);

    if (file)
        message.loginfo.count += cstring_writefmt3(message.data, MESSAGE_SIZE, "%s (%s:%u)", message.loginfo.count, string, file, &line);
    else
        message.loginfo.count += cstring_writefmt1(message.data, MESSAGE_SIZE, "%s", message.loginfo.count, string);

    kernel_notify(&messages.links, EVENT_LOGINFO, &message, message.loginfo.count);

}

void module_init(void)
{

    debug_initinterface(&interface, interface_write);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "log");
    system_initnode(&messages, SYSTEM_NODETYPE_NORMAL, "messages");

}

void module_register(void)
{

    debug_registerinterface(&interface);
    system_registernode(&root);
    system_addchild(&root, &messages);

}

void module_unregister(void)
{

    debug_unregisterinterface(&interface);
    system_unregisternode(&root);
    system_removechild(&root, &messages);

}

