#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct debug_interface interface;
static struct system_node root;
static struct system_node critical;
static struct system_node error;
static struct system_node warning;
static struct system_node info;
static struct list criticalstates;
static struct list errorstates;
static struct list warningstates;
static struct list infostates;

static void write(struct list *states, unsigned int level, char *string, char *file, unsigned int line)
{

    char num[FUDGE_NSIZE];

    switch (level)
    {

    case DEBUG_CRITICAL:
        kernel_multicast(states, "[CRIT] ", 7);

        break;

    case DEBUG_ERROR:
        kernel_multicast(states, "[ERRO] ", 7);

        break;

    case DEBUG_WARNING:
        kernel_multicast(states, "[WARN] ", 7);

        break;

    case DEBUG_INFO:
        kernel_multicast(states, "[INFO] ", 7);

        break;

    }

    kernel_multicast(states, string, ascii_length(string));
    kernel_multicast(states, " (", 2);
    kernel_multicast(states, file, ascii_length(file));
    kernel_multicast(states, ":", 1);
    kernel_multicast(states, num, ascii_wvalue(num, FUDGE_NSIZE, line, 10));
    kernel_multicast(states, ")\n", 2);

}

static void interface_write(unsigned int level, char *string, char *file, unsigned int line)
{

    if (level <= DEBUG_CRITICAL)
        write(&criticalstates, level, string, file, line);

    if (level <= DEBUG_ERROR)
        write(&errorstates, level, string, file, line);

    if (level <= DEBUG_WARNING)
        write(&warningstates, level, string, file, line);

    if (level <= DEBUG_INFO)
        write(&infostates, level, string, file, line);

}

static struct system_node *critical_open(struct system_node *self, struct service_state *state)
{

    list_add(&criticalstates, &state->item);

    return self;

}

static struct system_node *critical_close(struct system_node *self, struct service_state *state)
{

    list_remove(&criticalstates, &state->item);

    return self;

}

static struct system_node *error_open(struct system_node *self, struct service_state *state)
{

    list_add(&errorstates, &state->item);

    return self;

}

static struct system_node *error_close(struct system_node *self, struct service_state *state)
{

    list_remove(&errorstates, &state->item);

    return self;

}

static struct system_node *warning_open(struct system_node *self, struct service_state *state)
{

    list_add(&warningstates, &state->item);

    return self;

}

static struct system_node *warning_close(struct system_node *self, struct service_state *state)
{

    list_remove(&warningstates, &state->item);

    return self;

}

static struct system_node *info_open(struct system_node *self, struct service_state *state)
{

    list_add(&infostates, &state->item);

    return self;

}

static struct system_node *info_close(struct system_node *self, struct service_state *state)
{

    list_remove(&infostates, &state->item);

    return self;

}

void module_init(void)
{

    debug_initinterface(&interface, interface_write);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "log");
    system_initnode(&critical, SYSTEM_NODETYPE_MAILBOX, "critical");
    system_initnode(&error, SYSTEM_NODETYPE_MAILBOX, "error");
    system_initnode(&warning, SYSTEM_NODETYPE_MAILBOX, "warning");
    system_initnode(&info, SYSTEM_NODETYPE_MAILBOX, "info");

    critical.open = critical_open;
    critical.close = critical_close;
    error.open = error_open;
    error.close = error_close;
    warning.open = warning_open;
    warning.close = warning_close;
    info.open = info_open;
    info.close = info_close;

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

