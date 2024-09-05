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
    unsigned int n = offset / sizeof (struct event_coreinfo);
    unsigned int o = offset % sizeof (struct event_coreinfo);
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_CORE)); i++)
    {

        if (i == n)
        {

            struct core *core = resource->data;
            struct event_coreinfo coreinfo;

            coreinfo.id = core->id;
            coreinfo.sp = core->sp;
            coreinfo.task = core->itask;

            return buffer_read(buffer, count, &coreinfo, sizeof (struct event_coreinfo), o);

        }

    }

    return 0;

}

static unsigned int tasks_read(void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    unsigned int n = offset / sizeof (struct event_taskinfo);
    unsigned int o = offset % sizeof (struct event_taskinfo);
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_TASK)); i++)
    {

        if (i == n)
        {

            struct task *task = resource->data;
            struct event_taskinfo taskinfo;

            taskinfo.id = task->id;
            taskinfo.state = task->state;
            taskinfo.thread_ip = task->thread.ip;
            taskinfo.thread_sp = task->thread.sp;
            taskinfo.signals_kills = task->signals.kills;
            taskinfo.signals_blocks = task->signals.blocks;

            return buffer_read(buffer, count, &taskinfo, sizeof (struct event_taskinfo), o);

        }

    }

    return 0;

}

static unsigned int mailboxes_read(void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    unsigned int n = offset / sizeof (struct event_mailboxinfo);
    unsigned int o = offset % sizeof (struct event_mailboxinfo);
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_MAILBOX)); i++)
    {

        if (i == n)
        {

            struct mailbox *mailbox = resource->data;
            struct event_mailboxinfo mailboxinfo;

            mailboxinfo.address = (unsigned int)mailbox->ring.buffer;

            return buffer_read(buffer, count, &mailboxinfo, sizeof (struct event_mailboxinfo), o);

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

