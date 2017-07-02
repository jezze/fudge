#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct debug_log log;
static struct system_node root;
static struct system_node critical;
static struct system_node error;
static struct system_node warning;
static struct system_node info;
static struct list criticallinks;
static struct list errorlinks;
static struct list warninglinks;
static struct list infolinks;

static void write(struct list *links, unsigned int level, char *string, char *file, unsigned int line)
{

    char num[FUDGE_NSIZE];

    switch (level)
    {

    case DEBUG_CRITICAL:
        kernel_multicast(links, "[CRIT] ", 7);

        break;

    case DEBUG_ERROR:
        kernel_multicast(links, "[ERRO] ", 7);

        break;

    case DEBUG_WARNING:
        kernel_multicast(links, "[WARN] ", 7);

        break;

    case DEBUG_INFO:
        kernel_multicast(links, "[INFO] ", 7);

        break;

    }

    kernel_multicast(links, string, ascii_length(string));
    kernel_multicast(links, " (", 2);
    kernel_multicast(links, file, ascii_length(file));
    kernel_multicast(links, ":", 1);
    kernel_multicast(links, num, ascii_wvalue(num, FUDGE_NSIZE, line, 10, 0));
    kernel_multicast(links, ")\n", 2);

}

static void log_write(unsigned int level, char *string, char *file, unsigned int line)
{

    if (level <= DEBUG_CRITICAL)
        write(&criticallinks, level, string, file, line);

    if (level <= DEBUG_ERROR)
        write(&errorlinks, level, string, file, line);

    if (level <= DEBUG_WARNING)
        write(&warninglinks, level, string, file, line);

    if (level <= DEBUG_INFO)
        write(&infolinks, level, string, file, line);

}

static unsigned int critical_open(struct system_node *self, struct service_state *state)
{

    list_add(&criticallinks, &state->item);

    return state->id;

}

static unsigned int critical_close(struct system_node *self, struct service_state *state)
{

    list_remove(&criticallinks, &state->item);

    return state->id;

}

static unsigned int error_open(struct system_node *self, struct service_state *state)
{

    list_add(&errorlinks, &state->item);

    return state->id;

}

static unsigned int error_close(struct system_node *self, struct service_state *state)
{

    list_remove(&errorlinks, &state->item);

    return state->id;

}

static unsigned int warning_open(struct system_node *self, struct service_state *state)
{

    list_add(&warninglinks, &state->item);

    return state->id;

}

static unsigned int warning_close(struct system_node *self, struct service_state *state)
{

    list_remove(&warninglinks, &state->item);

    return state->id;

}

static unsigned int info_open(struct system_node *self, struct service_state *state)
{

    list_add(&infolinks, &state->item);

    return state->id;

}

static unsigned int info_close(struct system_node *self, struct service_state *state)
{

    list_remove(&infolinks, &state->item);

    return state->id;

}

void module_init(void)
{

    debug_initlog(&log, log_write);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "log");
    system_initnode(&critical, SYSTEM_NODETYPE_NORMAL, "critical");
    system_initnode(&error, SYSTEM_NODETYPE_NORMAL, "error");
    system_initnode(&warning, SYSTEM_NODETYPE_NORMAL, "warning");
    system_initnode(&info, SYSTEM_NODETYPE_NORMAL, "info");

    critical.open = critical_open;
    critical.close = critical_close;
    critical.read = system_readtask;
    error.open = error_open;
    error.close = error_close;
    error.read = system_readtask;
    warning.open = warning_open;
    warning.close = warning_close;
    warning.read = system_readtask;
    info.open = info_open;
    info.close = info_close;
    info.read = system_readtask;

    system_addchild(&root, &critical);
    system_addchild(&root, &error);
    system_addchild(&root, &warning);
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

