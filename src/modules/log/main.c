#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct debug_interface interface;
static struct system_node root;
static struct system_node messages;
static struct system_node send;

static void interface_write(unsigned int level, char *string, char *file, unsigned int line)
{

    char buffer[FUDGE_BSIZE];
    char num[FUDGE_NSIZE];
    unsigned int count = 0;

    switch (level)
    {

    case DEBUG_CRITICAL:
        count += memory_write(buffer, FUDGE_BSIZE, "[CRIT] ", 7, count);

        break;

    case DEBUG_ERROR:
        count += memory_write(buffer, FUDGE_BSIZE, "[ERRO] ", 7, count);

        break;

    case DEBUG_WARNING:
        count += memory_write(buffer, FUDGE_BSIZE, "[WARN] ", 7, count);

        break;

    case DEBUG_INFO:
        count += memory_write(buffer, FUDGE_BSIZE, "[INFO] ", 7, count);

        break;

    }

    count += memory_write(buffer, FUDGE_BSIZE, string, ascii_length(string), count);

    if (file && line)
    {

        count += memory_write(buffer, FUDGE_BSIZE, " (", 2, count);
        count += memory_write(buffer, FUDGE_BSIZE, file, ascii_length(file), count);
        count += memory_write(buffer, FUDGE_BSIZE, ":", 1, count);
        count += memory_write(buffer, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, line, 10, 0), count);
        count += memory_write(buffer, FUDGE_BSIZE, ")", 1, count);

    }

    count += memory_write(buffer, FUDGE_BSIZE, "\n", 1, count);

    kernel_notify(&messages.states, EVENT_DATA, buffer, count);

}

static unsigned int send_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
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

    resource_register(&interface.resource);
    system_registernode(&root);
    system_addchild(&root, &messages);
    system_addchild(&root, &send);

}

void module_unregister(void)
{

    resource_unregister(&interface.resource);
    system_unregisternode(&root);
    system_removechild(&root, &messages);
    system_removechild(&root, &send);

}

