#include <memory.h>
#include <vfs.h>
#include <binary.h>
#include <mmu.h>
#include <runtime.h>
#include <syscall.h>
#include <multi/multi.h>

static struct runtime_task tasks[MULTI_TASK_SLOTS];

struct runtime_task *multi_get_task(unsigned int index)
{

    if (!index || index >= MULTI_TASK_SLOTS)
        return 0;

    return &tasks[index];

}

unsigned int multi_get_task_slot(unsigned int parent)
{

    unsigned int i;

    for (i = parent; i < MULTI_TASK_SLOTS; i++)
    {

        if (!tasks[i].status.used)
            return i;

    }

    return 0;

}

void multi_clone_task(struct runtime_task *task, struct runtime_task *from, unsigned int id, unsigned int ip)
{

    memory_copy(task, from, sizeof (struct runtime_task));

    task->id = id;
    task->registers.ip = ip;
    task->registers.sp = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;
    task->registers.sb = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;

}

void multi_schedule()
{

    unsigned int i;

    for (i = MULTI_TASK_SLOTS - 1; i > 0; i--)
    {

        if (tasks[i].status.used && !tasks[i].status.idle)
        {

            runtime_set_task(&tasks[i]);

            mmu_load_user_memory(tasks[i].id);

            break;

        }

    }

}

static unsigned int exit(struct runtime_task *task, void *stack)
{

    task->status.used = 0;

    multi_schedule();

    return 1;

}

static unsigned int idle(struct runtime_task *task, void *stack)
{

    task->status.idle = 1;
    task->status.event = 0;

    multi_schedule();

    return 1;

}

static unsigned int spawn(struct runtime_task *task, void *stack)
{

    struct syscall_spawn_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    struct runtime_task *ntask;
    unsigned int id;
    unsigned int entry;

    if (!descriptor || !descriptor->interface->read)
        return 0;

    id = multi_get_task_slot(task->id);

    if (!id)
        return 0;

    mmu_map_user_memory(id, RUNTIME_TASK_PADDRESS_BASE + id * RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE, RUNTIME_TASK_ADDRESS_SIZE);
    mmu_load_user_memory(id);

    entry = binary_copy_program(descriptor->interface, descriptor->id);

    if (!entry)
        return 0;

    ntask = multi_get_task(id);
    multi_clone_task(ntask, task, id, entry);

    multi_schedule();

    return id;

}

void init()
{

    memory_clear(tasks, sizeof (struct runtime_task) * MULTI_TASK_SLOTS);

    syscall_set_routine(SYSCALL_INDEX_EXIT, exit);
    syscall_set_routine(SYSCALL_INDEX_IDLE, idle);
    syscall_set_routine(SYSCALL_INDEX_SPAWN, spawn);

}

void destroy()
{

}

