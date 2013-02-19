#include <fudge/module.h>
#include <kernel/vfs.h>
#include <kernel/binary.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/mmu.h>
#include "multi.h"

static struct multi_task *tasks[MULTI_TASK_SLOTS];

static struct multi_task *create_task()
{

    unsigned int i;

    for (i = 1; i < MULTI_TASK_SLOTS; i++)
    {

        if (tasks[i])
            continue;

        tasks[i] = (struct multi_task *)(MULTI_TASK_BASE + i * MMU_PAGE_SIZE * 8);

        mmu_map_memory(&tasks[i]->directory, &tasks[i]->tables[0], ARCH_KERNEL_BASE, ARCH_KERNEL_BASE, ARCH_KERNEL_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);
        mmu_map_memory(&tasks[i]->directory, &tasks[i]->tables[1], RUNTIME_TASKADDRESS_PHYSICAL + i * RUNTIME_TASKADDRESS_SIZE, RUNTIME_TASKADDRESS_VIRTUAL, RUNTIME_TASKADDRESS_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
        mmu_map_memory(&tasks[i]->directory, &tasks[i]->tables[2], RUNTIME_STACKADDRESS_PHYSICAL + i * RUNTIME_STACKADDRESS_SIZE, RUNTIME_STACKADDRESS_VIRTUAL - RUNTIME_STACKADDRESS_SIZE, RUNTIME_STACKADDRESS_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

        return tasks[i];

    }

    return 0;

}

static void notify_interrupt(struct runtime_container *self, unsigned int index)
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

        mmu_load_memory(&tasks[i]->directory);

        self->running = &tasks[i]->base;

        break;

    }

}

static unsigned int spawn(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index;} nargs, *args = stack;
    struct multi_task *ntask = create_task();

    nargs.caller = 0;
    nargs.index = args->index;

    if (!ntask)
        return 0;

    task->container->notify_interrupt = notify_interrupt;

    mmu_load_memory(&ntask->directory);
    memory_copy(&ntask->base, task, sizeof (struct runtime_task));

    ntask->base.registers.sp = (unsigned int)&nargs;
    ntask->base.registers.fp = (unsigned int)&nargs;

    return syscall_raise(SYSCALL_INDEX_EXECUTE, &ntask->base);

}

void init()
{

    memory_clear(tasks, sizeof (struct multi_task *) * MULTI_TASK_SLOTS);
    syscall_set_routine(SYSCALL_INDEX_SPAWN, spawn);

}

void destroy()
{

}

