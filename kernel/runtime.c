#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/arch/x86/mmu.h>
#include <modules/elf/elf.h>
#include <kernel/runtime.h>

struct runtime_task runtimeTasks[8];

struct runtime_task *runtime_get_running_task()
{

    return &runtimeTasks[0];

}

static struct vfs_descriptor *runtime_add_descriptor(struct runtime_task *task, struct vfs_node *node)
{

    unsigned int i;

    for (i = 0; i < 16; i++)
    {

        if (!task->descriptors[i].node)
        {

            task->descriptors[i].index = i;
            task->descriptors[i].node = node;
            task->descriptors[i].permissions = 0;

            return &task->descriptors[i];

        }

    }

    return 0;

}

static struct vfs_descriptor *runtime_get_descriptor(struct runtime_task *task, unsigned int index)
{

    return &task->descriptors[index];

}

static void runtime_remove_descriptor(struct runtime_task *task, unsigned int index)
{

    memory_set((void *)&task->descriptors[index], 0, sizeof (struct vfs_descriptor));

}

void runtime_init()
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        runtimeTasks[i].running = 0;
        runtimeTasks[i].add_descriptor = runtime_add_descriptor;
        runtimeTasks[i].get_descriptor = runtime_get_descriptor;
        runtimeTasks[i].remove_descriptor = runtime_remove_descriptor;

    }

}

