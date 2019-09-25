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
    count += memory_write(buffer, FUDGE_BSIZE, " (", 2, count);
    count += memory_write(buffer, FUDGE_BSIZE, file, ascii_length(file), count);
    count += memory_write(buffer, FUDGE_BSIZE, ":", 1, count);
    count += memory_write(buffer, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, line, 10, 0), count);
    count += memory_write(buffer, FUDGE_BSIZE, ")\n", 2, count);

    kernel_notify(states, EVENT_DATA, buffer, count);

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

