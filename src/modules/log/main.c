#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct debug_interface interface;
static struct system_node root;
static struct system_node critical;
static struct system_node error;
static struct system_node warning;
static struct system_node info;

static void write(struct list *states, unsigned int level, char *string, char *file, unsigned int line)
{

    union event_message message;
    char num[FUDGE_NSIZE];

    event_create(&message, EVENT_DATA, EVENT_BROADCAST, EVENT_BROADCAST, 0);

    switch (level)
    {

    case DEBUG_CRITICAL:
        event_append(&message, 7, "[CRIT] ");

        break;

    case DEBUG_ERROR:
        event_append(&message, 7, "[ERRO] ");

        break;

    case DEBUG_WARNING:
        event_append(&message, 7, "[WARN] ");

        break;

    case DEBUG_INFO:
        event_append(&message, 7, "[INFO] ");

        break;

    }

    event_append(&message, ascii_length(string), string);
    event_append(&message, 2, " (");
    event_append(&message, ascii_length(file), file);
    event_append(&message, 1, ":");
    event_append(&message, ascii_wvalue(num, FUDGE_NSIZE, line, 10), num);
    event_append(&message, 2, ")\n");
    kernel_multicast(states, &message);

}

static void interface_write(unsigned int level, char *string, char *file, unsigned int line)
{

    if (level <= DEBUG_CRITICAL)
        write(&critical.states, level, string, file, line);

    if (level <= DEBUG_ERROR)
        write(&error.states, level, string, file, line);

    if (level <= DEBUG_WARNING)
        write(&warning.states, level, string, file, line);

    if (level <= DEBUG_INFO)
        write(&info.states, level, string, file, line);

}

void module_init(void)
{

    debug_initinterface(&interface, interface_write);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "log");
    system_initnode(&critical, SYSTEM_NODETYPE_NORMAL, "critical");
    system_initnode(&error, SYSTEM_NODETYPE_NORMAL, "error");
    system_initnode(&warning, SYSTEM_NODETYPE_NORMAL, "warning");
    system_initnode(&info, SYSTEM_NODETYPE_NORMAL, "info");

}

void module_register(void)
{

    resource_register(&interface.resource);
    system_registernode(&root);
    system_addchild(&root, &critical);
    system_addchild(&root, &error);
    system_addchild(&root, &warning);
    system_addchild(&root, &info);

}

void module_unregister(void)
{

    resource_unregister(&interface.resource);
    system_unregisternode(&root);
    system_removechild(&root, &critical);
    system_removechild(&root, &error);
    system_removechild(&root, &warning);
    system_removechild(&root, &info);

}

