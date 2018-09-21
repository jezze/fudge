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

    char num[FUDGE_NSIZE];

    switch (level)
    {

    case DEBUG_CRITICAL:
        kernel_multicastdata(states, "[CRIT] ", 7);

        break;

    case DEBUG_ERROR:
        kernel_multicastdata(states, "[ERRO] ", 7);

        break;

    case DEBUG_WARNING:
        kernel_multicastdata(states, "[WARN] ", 7);

        break;

    case DEBUG_INFO:
        kernel_multicastdata(states, "[INFO] ", 7);

        break;

    }

    kernel_multicastdata(states, string, ascii_length(string));
    kernel_multicastdata(states, " (", 2);
    kernel_multicastdata(states, file, ascii_length(file));
    kernel_multicastdata(states, ":", 1);
    kernel_multicastdata(states, num, ascii_wvalue(num, FUDGE_NSIZE, line, 10));
    kernel_multicastdata(states, ")\n", 2);

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
    system_initnode(&critical, SYSTEM_NODETYPE_MAILBOX, "critical");
    system_initnode(&error, SYSTEM_NODETYPE_MAILBOX, "error");
    system_initnode(&warning, SYSTEM_NODETYPE_MAILBOX, "warning");
    system_initnode(&info, SYSTEM_NODETYPE_MAILBOX, "info");

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

