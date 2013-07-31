#include <fudge/kernel.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include "arch.h"
#include "cpu.h"
#include "mmu.h"
#include "multi.h"

#define MULTI_ENTRIES                   16

static struct multi_task
{

    struct task base;
    unsigned int address;
    unsigned int stack;
    struct mmu_directory directory;
    struct mmu_table tables[2];

} tasks[MULTI_ENTRIES];

static struct multi_task *create_task()
{

    unsigned int i;

    for (i = 1; i < MULTI_ENTRIES; i++)
    {

        if (tasks[i].base.state & TASK_STATE_USED)
            continue;

        tasks[i].address = TASKADDRESS_PHYSICAL + i * TASKADDRESS_SIZE;
        tasks[i].stack = STACKADDRESS_PHYSICAL + i * STACKADDRESS_SIZE;

        return &tasks[i];

    }

    return 0;

}

static struct multi_task *find_task()
{

    unsigned int i;

    for (i = MULTI_ENTRIES - 1; i > 0; i--)
    {

        if (!(tasks[i].base.state & TASK_STATE_USED))
            continue;

        return &tasks[i];

    }

    return 0;

}

static struct multi_task *find_task_by_directory(struct mmu_directory *directory)
{

    unsigned int i;

    for (i = 1; i < MULTI_ENTRIES; i++)
    {

        if (&tasks[i].directory == directory)
            return &tasks[i];

    }

    return 0;

}

static unsigned int spawn(struct container *self, struct task *task, void *stack)
{

    struct parameters {void *caller; unsigned int index;} nargs, *args = stack;
    struct multi_task *ntask = create_task();

    if (!ntask)
        return 0;

    memory_copy(&nargs, args, sizeof (struct parameters));
    memory_copy(&ntask->base, task, sizeof (struct task));
    memory_clear(&ntask->directory, sizeof (struct mmu_directory));
    memory_copy(&ntask->directory, mmu_get_directory(), 4);
    mmu_set_directory(&ntask->directory);

    return self->calls[CONTAINER_CALL_EXECUTE](self, &ntask->base, &nargs);

}

static void map(struct container *self, unsigned int address)
{

    struct multi_task *task = find_task_by_directory(mmu_get_directory());

    if (!task)
        return;

    mmu_map(&task->directory, &task->tables[0], task->address, TASKADDRESS_VIRTUAL, TASKADDRESS_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_map(&task->directory, &task->tables[1], task->stack, STACKADDRESS_VIRTUAL - STACKADDRESS_SIZE, STACKADDRESS_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_set_directory(&task->directory);

}

static void schedule(struct container *self)
{

    struct multi_task *task = find_task();

    if (!task)
        return;

    mmu_set_directory(&task->directory);

    self->running = &task->base;

}

void multi_setup(struct container *container)
{

    container->map = map;
    container->schedule = schedule;
    container->calls[CONTAINER_CALL_SPAWN] = spawn;

}

