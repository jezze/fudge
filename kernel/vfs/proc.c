#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/vfs/proc.h>

static struct runtime_task *tasks;
static struct modules_filesystem filesystem;

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int length = 0;
    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_SLOTS - 1; i++)
    {

        struct runtime_task *task = &tasks[i];

        if (task->used)
        {

            length += 0;

        }

    }

    return length;

}

static unsigned int find(struct modules_filesystem *self, char *name)
{

    unsigned int length = string_length(name);

    if (!length)
        return 1;

    return 0;

}

void vfs_proc_init(struct runtime_task *t)
{

    tasks = t;

    modules_filesystem_init(&filesystem, 0x0001, "proc", 0, 0, read, 0, find, 0);
    modules_register_filesystem(&filesystem);
    filesystem.path = "/proc/";

}

