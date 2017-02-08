#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "service.h"
#include "container.h"
#include "task.h"

static struct list active;
static struct list inactive;
static struct list blocked;

struct task *task_findactive(void)
{

    return (active.tail) ? active.tail->data : 0;

}

struct task *task_findinactive(void)
{

    return (inactive.tail) ? inactive.tail->data : 0;

}

void task_setstatus(struct task *task, unsigned int status)
{

    switch (status)
    {

    case TASK_STATUS_INACTIVE:
        if (task->state.status == TASK_STATUS_ACTIVE)
        {

            list_move(&inactive, &task->state.item);

            task->state.status = TASK_STATUS_INACTIVE;

        }

        break;

    case TASK_STATUS_ACTIVE:
        if (task->state.status == TASK_STATUS_ACTIVE || task->state.status == TASK_STATUS_INACTIVE || task->state.status == TASK_STATUS_UNBLOCKED)
        {

            list_move(&active, &task->state.item);

            task->state.status = TASK_STATUS_ACTIVE;

        }

        break;

    case TASK_STATUS_BLOCKED:
        if (task->state.status == TASK_STATUS_ACTIVE)
        {

            list_move(&blocked, &task->state.item);

            task->state.status = TASK_STATUS_BLOCKED;

        }

        break;

    case TASK_STATUS_UNBLOCKED:
        if (task->state.status == TASK_STATUS_UNBLOCKED || task->state.status == TASK_STATUS_BLOCKED)
        {

            list_move(&active, &task->state.item);

            task->state.status = TASK_STATUS_UNBLOCKED;

        }

        break;

    }

}

void task_resume(struct task *task, unsigned int ip, unsigned int sp)
{

    task_setstatus(task, TASK_STATUS_ACTIVE);

    task->state.registers.ip = ip;
    task->state.registers.sp = sp;
    task->state.rewind = 0;

}

unsigned int task_read(struct task *task, void *buffer, unsigned int count)
{

    count = ring_read(&task->mailbox.ring, buffer, count);

    if (!count)
        task_setstatus(task, TASK_STATUS_BLOCKED);

    return count;

}

unsigned int task_write(struct task *task, void *buffer, unsigned int count)
{

    count = ring_write(&task->mailbox.ring, buffer, count);

    task_setstatus(task, TASK_STATUS_UNBLOCKED);

    return count;

}

void task_register(struct task *task)
{

    list_add(&inactive, &task->state.item);

}

void task_unregister(struct task *task)
{

    list_remove(&inactive, &task->state.item);

}

void task_init(struct task *task)
{

    unsigned int i;

    resource_init(&task->resource, RESOURCE_TASK, task);
    list_inititem(&task->state.item, task);
    ring_init(&task->mailbox.ring, TASK_MAILBOXSIZE, task->mailbox.buffer);

    for (i = 0; i < TASK_DESCRIPTORS; i++)
    {

        resource_init(&task->descriptors[i].resource, RESOURCE_TASKDESCRIPTOR, &task->descriptors[i]);
        list_inititem(&task->descriptors[i].state.link, task);

    }

}

