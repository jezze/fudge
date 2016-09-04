#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct debug_log log;
static struct system_node root;
static struct system_node critical;
static struct system_node error;
static struct system_node warning;
static struct system_node info;

static void write(struct system_node *node, unsigned int level, char *string, char *file, unsigned int line)
{

    char num[FUDGE_NSIZE];

    switch (level)
    {

    case DEBUG_CRITICAL:
        system_multicast(node, "[CRIT] ", 7);

        break;

    case DEBUG_ERROR:
        system_multicast(node, "[ERRO] ", 7);

        break;

    case DEBUG_WARNING:
        system_multicast(node, "[WARN] ", 7);

        break;

    case DEBUG_INFO:
        system_multicast(node, "[INFO] ", 7);

        break;

    }

    system_multicast(node, string, ascii_length(string));
    system_multicast(node, " (", 2);
    system_multicast(node, file, ascii_length(file));
    system_multicast(node, ":", 1);
    system_multicast(node, num, ascii_wvalue(num, FUDGE_NSIZE, line, 10, 0));
    system_multicast(node, ")\n", 2);

}

static void log_write(unsigned int level, char *string, char *file, unsigned int line)
{

    if (level <= DEBUG_CRITICAL)
        write(&critical, level, string, file, line);

    if (level <= DEBUG_ERROR)
        write(&error, level, string, file, line);

    if (level <= DEBUG_WARNING)
        write(&warning, level, string, file, line);

    if (level <= DEBUG_INFO)
        write(&info, level, string, file, line);

}

void module_init(void)
{

    debug_initlog(&log, log_write);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "log");
    system_initnode(&critical, SYSTEM_NODETYPE_MAILBOX, "critical");
    system_addchild(&root, &critical);
    system_initnode(&error, SYSTEM_NODETYPE_MAILBOX, "error");
    system_addchild(&root, &error);
    system_initnode(&warning, SYSTEM_NODETYPE_MAILBOX, "warning");
    system_addchild(&root, &warning);
    system_initnode(&info, SYSTEM_NODETYPE_MAILBOX, "info");
    system_addchild(&root, &info);

}

void module_register(void)
{

    resource_register(&log.resource);
    system_registernode(&root);

}

void module_unregister(void)
{

    resource_unregister(&log.resource);
    system_unregisternode(&root);

}

