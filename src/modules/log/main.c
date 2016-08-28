#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "log.h"

static struct system_node root;
static struct system_node critical;
static struct system_node error;
static struct system_node warning;
static struct system_node info;

void log_notify(unsigned int level, void *buffer, unsigned int count)
{

    if (level <= LOG_CRITICAL)
        system_multicast(&critical, buffer, count);

    if (level <= LOG_ERROR)
        system_multicast(&error, buffer, count);

    if (level <= LOG_WARNING)
        system_multicast(&warning, buffer, count);

    if (level <= LOG_INFO)
        system_multicast(&info, buffer, count);

}

void module_init(void)
{

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

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

