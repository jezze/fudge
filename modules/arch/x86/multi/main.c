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

static struct runtime_task *notify_interrupt(struct runtime_task *self, unsigned int index)
{

    unsigned int i;

    for (i = MULTI_TASK_SLOTS - 1; i > 0; i--)
    {

        if (!tasks[i])
            continue;

        if (!tasks[i]->base.status.used)
        {

            tasks[i] = 0;

            continue;

        }

        if (tasks[i]->base.status.idle)
            continue;

        mmu_load_memory(&tasks[i]->directory);

        return &tasks[i]->base;

    }

    return self;

}

static unsigned int spawn(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    struct binary_format *format = binary_get_format(descriptor->interface, descriptor->id);
    struct multi_task *ntask = create_task();

    if (!descriptor || !format || !ntask)
        return 0;

    task->notify_interrupt = notify_interrupt;

    mmu_load_memory(&ntask->directory);
    memory_copy(&ntask->base, task, sizeof (struct runtime_task));

    ntask->base.registers.ip = format->copy_program(descriptor->interface, descriptor->id);
    ntask->base.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    ntask->base.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;
    ntask->base.registers.status = 0;
    ntask->base.notify_interrupt = notify_interrupt;

    return ntask->base.registers.ip;

}

void init()
{

    memory_clear(tasks, sizeof (struct multi_task *) * MULTI_TASK_SLOTS);
    syscall_set_routine(SYSCALL_INDEX_SPAWN, spawn);

}

void destroy()
{

}

