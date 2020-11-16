#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node cores;
static struct system_node tasks;
static struct system_node mailboxes;

static unsigned int cores_read(struct service_link *link, void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    char data[FUDGE_BSIZE];
    unsigned int c = 0;

    while ((resource = resource_foreachtype(resource, RESOURCE_CORE)))
    {

        struct core *core = resource->data;
        struct list_item *item;
        char num[FUDGE_NSIZE];

        c += memory_write(data, FUDGE_BSIZE, "core ", 5, c);
        c += memory_write(data, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, core->id, 10, 0), c);
        c += memory_write(data, FUDGE_BSIZE, "\n", 1, c);

        spinlock_acquire(&core->tasks.spinlock);

        for (item = core->tasks.head; item; item = item->next)
        {

            struct task *task = item->data;

            c += memory_write(data, FUDGE_BSIZE, "    task ", 9, c);
            c += memory_write(data, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, task->id, 10, 0), c);
            c += memory_write(data, FUDGE_BSIZE, "\n", 1, c);

        }

        spinlock_release(&core->tasks.spinlock);

    }

    return memory_read(buffer, count, data, c, offset);

}

static unsigned int tasks_read(struct service_link *link, void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    char data[FUDGE_BSIZE];
    unsigned int c = 0;

    while ((resource = resource_foreachtype(resource, RESOURCE_TASK)))
    {

        struct task *task = resource->data;
        char num[FUDGE_NSIZE];

        c += memory_write(data, FUDGE_BSIZE, "task ", 5, c);
        c += memory_write(data, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, task->id, 10, 0), c);
        c += memory_write(data, FUDGE_BSIZE, "\n", 1, c);

    }

    return memory_read(buffer, count, data, c, offset);

}

static unsigned int mailboxes_read(struct service_link *link, void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    char data[FUDGE_BSIZE];
    unsigned int c = 0;

    while ((resource = resource_foreachtype(resource, RESOURCE_MAILBOX)))
    {

        struct mailbox *mailbox = resource->data;
        char num[FUDGE_NSIZE];

        c += memory_write(data, FUDGE_BSIZE, "mailbox ", 8, c);
        c += memory_write(data, FUDGE_BSIZE, "0x", 2, c);
        c += memory_write(data, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, (unsigned int)mailbox->ring.buffer, 16, 8), c);
        c += memory_write(data, FUDGE_BSIZE, "\n", 1, c);

    }

    return memory_read(buffer, count, data, c, offset);

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

