#include <fudge/kernel.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include "arch.h"
#include "cpu.h"
#include "mmu.h"
#include "multi.h"

#define MULTI_TASK_BASE                 0x00300000
#define MULTI_TASK_SLOTS                32

struct multi_task
{

    struct mmu_directory directory;
    struct mmu_table tables[2];
    struct runtime_task base;

};

static struct multi_task *tasks[MULTI_TASK_SLOTS];

static struct multi_task *create_task()
{

    unsigned int i;

    for (i = 1; i < MULTI_TASK_SLOTS; i++)
    {

        if (tasks[i])
            continue;

        tasks[i] = (struct multi_task *)(MULTI_TASK_BASE + i * MMU_PAGE_SIZE * 8);

        memory_copy(&tasks[i]->directory, (void *)cpu_get_cr3(), sizeof (struct mmu_directory));
        mmu_map_memory(&tasks[i]->directory, &tasks[i]->tables[0], RUNTIME_TASKADDRESS_PHYSICAL + i * RUNTIME_TASKADDRESS_SIZE, RUNTIME_TASKADDRESS_VIRTUAL, RUNTIME_TASKADDRESS_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
        mmu_map_memory(&tasks[i]->directory, &tasks[i]->tables[1], RUNTIME_STACKADDRESS_PHYSICAL + i * RUNTIME_STACKADDRESS_SIZE, RUNTIME_STACKADDRESS_VIRTUAL - RUNTIME_STACKADDRESS_SIZE, RUNTIME_STACKADDRESS_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

        return tasks[i];

    }

    return 0;

}

static void activate(struct runtime_container *container, struct multi_task *task)
{

    mmu_load_memory(&task->directory);

    container->running = &task->base;

}

static struct multi_task *schedule()
{

    unsigned int i;

    for (i = MULTI_TASK_SLOTS - 1; i > 0; i--)
    {

        if (!tasks[i])
            continue;

        if (!(tasks[i]->base.state & RUNTIME_TASK_STATE_USED))
        {

            tasks[i] = 0;

            continue;

        }

        return tasks[i];

    }

    return 0;

}

static unsigned int spawn(struct runtime_task *task, void *stack)
{

    struct parameters {void *caller; unsigned int index;} nargs, *args = stack;
    struct multi_task *ntask = create_task();

    if (!ntask)
        return 0;

    memory_copy(&ntask->base, task, sizeof (struct runtime_task));
    memory_copy(&nargs, args, sizeof (struct parameters));

    ntask->base.registers.sp = (unsigned int)&nargs;
    ntask->base.registers.fp = (unsigned int)&nargs;

    activate(ntask->base.container, ntask);

    return syscall_raise(SYSCALL_INDEX_EXECUTE, &ntask->base);

}

void multi_notify(struct runtime_container *self)
{

    struct multi_task *task = schedule();

    if (task)
        activate(self, task);

}

void multi_setup()
{

    memory_clear(tasks, sizeof (struct multi_task *) * MULTI_TASK_SLOTS);
    syscall_set_routine(SYSCALL_INDEX_SPAWN, spawn);

}

