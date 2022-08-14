#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct debug_interface interface;
static struct system_node root;
static struct system_node messages;
static struct system_node send;

static void interface_write(unsigned int level, char *string, char *file, unsigned int line)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    switch (level)
    {

    case DEBUG_CRITICAL:
        count += cstring_write(buffer, BUFFER_SIZE, "[CRIT] ", count);

        break;

    case DEBUG_ERROR:
        count += cstring_write(buffer, BUFFER_SIZE, "[ERRO] ", count);

        break;

    case DEBUG_WARNING:
        count += cstring_write(buffer, BUFFER_SIZE, "[WARN] ", count);

        break;

    case DEBUG_INFO:
        count += cstring_write(buffer, BUFFER_SIZE, "[INFO] ", count);

        break;

    }

    count += cstring_write(buffer, BUFFER_SIZE, string, count);

    if (file && line)
    {

        count += cstring_write(buffer, BUFFER_SIZE, " (", count);
        count += cstring_write(buffer, BUFFER_SIZE, file, count);
        count += cstring_write(buffer, BUFFER_SIZE, ":", count);
        count += cstring_writevalue(buffer, BUFFER_SIZE, line, 10, 0, count);
        count += cstring_write(buffer, BUFFER_SIZE, ")", count);

    }

    count += cstring_write(buffer, BUFFER_SIZE, "\n", count);

    kernel_notify(&messages.links, EVENT_DATA, buffer, count);

}

static unsigned int send_write(void *buffer, unsigned int count, unsigned int offset)
{

    interface_write(DEBUG_INFO, buffer, 0, 0);

    return count;

}

void module_init(void)
{

    debug_initinterface(&interface, interface_write);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "log");
    system_initnode(&messages, SYSTEM_NODETYPE_NORMAL, "messages");
    system_initnode(&send, SYSTEM_NODETYPE_NORMAL, "send");

    send.operations.write = send_write;

}

void module_register(void)
{

    debug_registerinterface(&interface);
    system_registernode(&root);
    system_addchild(&root, &messages);
    system_addchild(&root, &send);

}

void module_unregister(void)
{

    debug_unregisterinterface(&interface);
    system_unregisternode(&root);
    system_removechild(&root, &messages);
    system_removechild(&root, &send);

}

