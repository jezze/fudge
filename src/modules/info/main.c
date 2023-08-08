#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node cores;
static struct system_node tasks;
static struct system_node mailboxes;

static unsigned int cores_read(void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    unsigned int n = offset / sizeof (struct ctrl_core);
    unsigned int o = offset % sizeof (struct ctrl_core);
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_CORE)); i++)
    {

        if (i == n)
        {

            struct core *core = resource->data;
            struct ctrl_core ctrl;

            ctrl.id = core->id;
            ctrl.sp = core->sp;
            ctrl.task = core->itask;

            return buffer_read(buffer, count, &ctrl, sizeof (struct core), o);

        }

    }

    return 0;

}

static unsigned int tasks_read(void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    unsigned int n = offset / sizeof (struct ctrl_task);
    unsigned int o = offset % sizeof (struct ctrl_task);
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_TASK)); i++)
    {

        if (i == n)
        {

            struct task *task = resource->data;
            struct ctrl_task ctrl;

            ctrl.id = task->id;
            ctrl.state = task->state;
            ctrl.thread_ip = task->thread.ip;
            ctrl.thread_sp = task->thread.sp;
            ctrl.signals_kills = task->signals.kills;
            ctrl.signals_blocks = task->signals.blocks;

            return buffer_read(buffer, count, &ctrl, sizeof (struct task), o);

        }

    }

    return 0;

}

static unsigned int mailboxes_read(void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    unsigned int n = offset / sizeof (struct ctrl_mailbox);
    unsigned int o = offset % sizeof (struct ctrl_mailbox);
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_MAILBOX)); i++)
    {

        if (i == n)
        {

            struct mailbox *mailbox = resource->data;
            struct ctrl_mailbox ctrl;

            ctrl.address = (unsigned int)mailbox->ring.buffer;

            return buffer_read(buffer, count, &ctrl, sizeof (struct mailbox), o);

        }

    }

    return 0;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "info");
    system_initnode(&cores, SYSTEM_NODETYPE_NORMAL, "cores");
    system_initnode(&tasks, SYSTEM_NODETYPE_NORMAL, "tasks");
    system_initnode(&mailboxes, SYSTEM_NODETYPE_NORMAL, "mailboxes");

    cores.operations.read = cores_read;
    tasks.operations.read = tasks_read;
    mailboxes.operations.read = mailboxes_read;

}

void module_register(void)
{

    system_registernode(&root);
    system_addchild(&root, &cores);
    system_addchild(&root, &tasks);
    system_addchild(&root, &mailboxes);

}

void module_unregister(void)
{

    system_unregisternode(&root);
    system_removechild(&root, &cores);
    system_removechild(&root, &tasks);
    system_removechild(&root, &mailboxes);

}

