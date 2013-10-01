#include <fudge/kernel.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include "cpu.h"
#include "arch.h"
#include "mmu.h"
#include "multi.h"

#define MULTI_TASKS                     16
#define MULTI_KERNEL_BASE               0x00000000
#define MULTI_KERNEL_BASESIZE           0x00400000
#define MULTI_KERNEL_TABLES             1
#define MULTI_TASK_BASE                 0x00400000
#define MULTI_TASK_BASESIZE             0x00010000
#define MULTI_TASK_STACK                0x00810000
#define MULTI_TASK_STACKVIRT            0x80000000
#define MULTI_TASK_STACKSIZE            0x00010000
#define MULTI_TASK_TABLES               2

static struct multi_kernel
{

    struct mmu_table tables[MULTI_KERNEL_TABLES];

} kernel;

static struct multi_task
{

    struct task base;
    unsigned int index;
    struct mmu_directory directory;
    struct mmu_table tables[MULTI_TASK_TABLES];

} tasks[MULTI_TASKS];

static struct multi_task *find_next_task()
{

    unsigned int i;

    for (i = MULTI_TASKS - 1; i > 0; i--)
    {

        if (!(tasks[i].base.state & TASK_STATE_USED))
            continue;

        if (tasks[i].base.state & TASK_STATE_BLOCKED)
            continue;

        return &tasks[i];

    }

    return 0;

}

static struct multi_task *find_free_task()
{

    unsigned int i;

    for (i = 1; i < MULTI_TASKS; i++)
    {

        if (tasks[i].base.state & TASK_STATE_USED)
            continue;

        tasks[i].index = i;

        return &tasks[i];

    }

    return 0;

}

static void map_kernel(struct multi_task *task)
{

    memory_clear(&task->directory, sizeof (struct mmu_directory));
    mmu_map(&task->directory, &kernel.tables[0], MULTI_KERNEL_BASE, MULTI_KERNEL_BASE, MULTI_KERNEL_BASESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static void map_user(struct multi_task *task, unsigned int address)
{

    mmu_map(&task->directory, &task->tables[0], task->index * MULTI_TASK_BASESIZE + MULTI_TASK_BASE, address, MULTI_TASK_BASESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_map(&task->directory, &task->tables[1], task->index * MULTI_TASK_STACKSIZE + MULTI_TASK_STACK, MULTI_TASK_STACKVIRT - MULTI_TASK_STACKSIZE, MULTI_TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

void multi_activate(struct container *self, struct task *current)
{

    struct multi_task *task = (struct multi_task *)current;

    self->current = current;

    mmu_load(&task->directory);

}

void multi_map(struct container *self, unsigned int address)
{

    struct multi_task *task = find_next_task();

    if (!task)
        return;

    map_user(task, address);

}

struct task *multi_schedule(struct container *self)
{

    struct multi_task *task = find_next_task();

    if (!task)
        return 0;

    return &task->base;

}

static unsigned int spawn(struct container *self, struct task *task, void *stack)
{

    struct parameters {void *caller; unsigned int index;} temp, *args = stack;
    struct multi_task *parent = (struct multi_task *)task;
    struct multi_task *child = find_free_task();

    if (!child)
        return 0;

    memory_copy(&temp, args, sizeof (struct parameters));

    task_init(&child->base, 0, MULTI_TASK_STACKVIRT, MULTI_TASK_STACKVIRT);
    map_kernel(child);
    mmu_load(&child->directory);

    memory_copy(child->base.descriptors, parent->base.descriptors, sizeof (struct task_descriptor) * TASK_DESCRIPTORS);

    child->base.status = self->calls[CONTAINER_CALL_EXECUTE](self, &child->base, &temp);

    return 0;

}

struct task *multi_setup(struct container *container)
{

    struct multi_task *task = find_free_task();

    container->calls[CONTAINER_CALL_SPAWN] = spawn;

    task_init(&task->base, 0, MULTI_TASK_STACKVIRT, MULTI_TASK_STACKVIRT);
    map_kernel(task);
    mmu_load(&task->directory);
    mmu_enable();

    return &task->base;

}

