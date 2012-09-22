#include <memory.h>
#include <multi.h>
#include <runtime.h>

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

struct runtime_task *multi_schedule()
{

    unsigned int i;

    for (i = MULTI_TASK_SLOTS - 1; i > 0; i--)
    {

        if (tasks[i].status.used && !tasks[i].status.idle)
            return &tasks[i];

    }

    return 0;

}

struct runtime_task *multi_setup()
{

    memory_clear(tasks, sizeof (struct runtime_task) * MULTI_TASK_SLOTS);

    return tasks;

}

