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
    char data[BUFFER_SIZE];
    unsigned int c = 0;

    while ((resource = resource_foreachtype(resource, RESOURCE_CORE)))
    {

        struct core *core = resource->data;
        struct list_item *item;

        c += cstring_write(data, BUFFER_SIZE, "core ", c);
        c += cstring_writevalue(data, BUFFER_SIZE, core->id, 10, 0, c);
        c += cstring_write(data, BUFFER_SIZE, "\n", c);

        spinlock_acquire(&core->tasks.spinlock);

        for (item = core->tasks.head; item; item = item->next)
        {

            struct task *task = item->data;

            c += cstring_write(data, BUFFER_SIZE, "    task ", c);
            c += cstring_writevalue(data, BUFFER_SIZE, task->id, 10, 0, c);
            c += cstring_write(data, BUFFER_SIZE, "\n", c);

        }

        spinlock_release(&core->tasks.spinlock);

    }

    return buffer_read(buffer, count, data, c, offset);

}

static unsigned int tasks_read(void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    char data[BUFFER_SIZE];
    unsigned int c = 0;

    while ((resource = resource_foreachtype(resource, RESOURCE_TASK)))
    {

        struct task *task = resource->data;

        c += cstring_write(data, BUFFER_SIZE, "task ", c);
        c += cstring_writevalue(data, BUFFER_SIZE, task->id, 10, 0, c);
        c += cstring_write(data, BUFFER_SIZE, "\n", c);

    }

    return buffer_read(buffer, count, data, c, offset);

}

static unsigned int mailboxes_read(void *buffer, unsigned int count, unsigned int offset)
{

    struct resource *resource = 0;
    char data[BUFFER_SIZE];
    unsigned int c = 0;

    while ((resource = resource_foreachtype(resource, RESOURCE_MAILBOX)))
    {

        struct mailbox *mailbox = resource->data;

        c += cstring_write(data, BUFFER_SIZE, "mailbox ", c);
        c += cstring_write(data, BUFFER_SIZE, "0x", c);
        c += cstring_writevalue(data, BUFFER_SIZE, (unsigned int)mailbox->ring.buffer, 16, 8, c);
        c += cstring_write(data, BUFFER_SIZE, "\n", c);

    }

    return buffer_read(buffer, count, data, c, offset);

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

