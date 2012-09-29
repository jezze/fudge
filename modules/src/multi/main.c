#include <memory.h>
#include <vfs.h>
#include <binary.h>
#include <runtime.h>
#include <syscall.h>
#include <arch/x86/mmu.h>
#include <multi/multi.h>

static struct runtime_task tasks[MULTI_TASK_SLOTS];

static struct runtime_task *get_task(unsigned int index)
{

    if (!index || index >= MULTI_TASK_SLOTS)
        return 0;

    return &tasks[index];

}

static unsigned int get_task_slot(unsigned int parent)
{

    unsigned int i;

    for (i = parent; i < MULTI_TASK_SLOTS; i++)
    {

        if (!tasks[i].status.used)
            return i;

    }

    return 0;

}

static void schedule()
{

    unsigned int i;

    for (i = MULTI_TASK_SLOTS - 1; i > 0; i--)
    {

        if (!tasks[i].status.used)
            continue;

        if (tasks[i].status.idle)
            continue;

        runtime_set_task(&tasks[i]);

        mmu_load_memory(tasks[i].id);

        break;

    }

}

static void notify_pre_event(struct runtime_task *task, unsigned int index)
{

    unsigned int i;

    for (i = 1; i < MULTI_TASK_SLOTS - 1; i++)
    {

        if (!tasks[i].status.used)
            continue;

        if (!tasks[i].status.idle)
            continue;

        if (!tasks[i].events[index].callback)
            continue;

        tasks[i].status.idle = 0;
        runtime_init_registers(&tasks[i].registers, tasks[i].events[index].callback, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, tasks[i].registers.status);

    }

}

static void notify_post_event(struct runtime_task *task, unsigned int index)
{

    schedule();

}

void clone_task(struct runtime_task *task, struct runtime_task *from, unsigned int id, unsigned int ip)
{

    memory_copy(task, from, sizeof (struct runtime_task));

    runtime_init_registers(&task->registers, ip, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, id);

    task->id = id;
    task->notify_pre_event = notify_pre_event;
    task->notify_post_event = notify_post_event;

}

static unsigned int spawn(struct runtime_task *task, void *stack)
{

    struct multi_spawn_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    struct runtime_task *ntask;
    unsigned int id;
    unsigned int entry;

    if (!descriptor || !descriptor->interface->read)
        return 0;

    id = get_task_slot(task->id);

    if (!id)
        return 0;

    mmu_map_user_memory(id, RUNTIME_TASK_PADDRESS_BASE + id * RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE, RUNTIME_TASK_ADDRESS_SIZE);
    mmu_load_memory(id);

    entry = binary_copy_program(descriptor->interface, descriptor->id);

    if (!entry)
        return 0;

    ntask = get_task(id);
    clone_task(ntask, task, id, entry);

    schedule();

    return id;

}

void init()
{

    memory_clear(tasks, sizeof (struct runtime_task) * MULTI_TASK_SLOTS);

    syscall_set_routine(SYSCALL_INDEX_SPAWN, spawn);

}

void destroy()
{

}

