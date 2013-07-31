#include <fudge/kernel.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include "arch.h"
#include "cpu.h"
#include "mmu.h"
#include "multi.h"

#define MULTI_TASKS                     16
#define MULTI_KERNEL_BASE               0x00000000
#define MULTI_KERNEL_SIZE               0x00400000
#define MULTI_KERNEL_TABLES             3
#define MULTI_TASK_BASE                 0x00400000
#define MULTI_TASK_BASESIZE             0x00010000
#define MULTI_TASK_STACK                0x00810000
#define MULTI_TASK_STACKSIZE            0x00010000
#define MULTI_TASK_VIRTUAL              0x08048000
#define MULTI_TASK_TABLES               2

static struct multi_kernel
{

    struct mmu_directory directory;
    struct mmu_table tables[MULTI_KERNEL_TABLES];

} kernel;

static struct multi_task
{

    struct task base;
    unsigned int address;
    unsigned int stack;
    struct mmu_directory directory;
    struct mmu_table tables[MULTI_TASK_TABLES];

} tasks[MULTI_TASKS];

static struct multi_task *create_task()
{

    unsigned int i;

    for (i = 1; i < MULTI_TASKS; i++)
    {

        if (tasks[i].base.state & TASK_STATE_USED)
            continue;

        tasks[i].address = MULTI_TASK_BASE + i * MULTI_TASK_BASESIZE;
        tasks[i].stack = MULTI_TASK_STACK + i * MULTI_TASK_STACKSIZE;

        return &tasks[i];

    }

    return 0;

}

static struct multi_task *find_task()
{

    unsigned int i;

    for (i = MULTI_TASKS - 1; i > 0; i--)
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

    for (i = 1; i < MULTI_TASKS; i++)
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

    mmu_map(&task->directory, &task->tables[0], task->address, MULTI_TASK_VIRTUAL, MULTI_TASK_BASESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_map(&task->directory, &task->tables[1], task->stack, TASK_STACK - MULTI_TASK_STACKSIZE, MULTI_TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
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

    mmu_map(&kernel.directory, &kernel.tables[0], MULTI_KERNEL_BASE, MULTI_KERNEL_BASE, MULTI_KERNEL_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(&kernel.directory, &kernel.tables[1], MULTI_TASK_BASE, MULTI_TASK_VIRTUAL, MULTI_TASK_BASESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_map(&kernel.directory, &kernel.tables[2], MULTI_TASK_STACK, TASK_STACK - MULTI_TASK_STACKSIZE, MULTI_TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_set_directory(&kernel.directory);
    mmu_enable();

}

