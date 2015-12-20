#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"

static struct list active;
static struct list inactive;
static struct list blocked;

struct task *task_findactive(void)
{

    return (active.tail) ? active.tail->data : 0;

}

struct task_mailbox *task_findactivemailbox(void)
{

    struct task *task = task_findactive();

    return (task) ? &task->mailbox : 0;

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
        list_move(&inactive, &task->state.item);

        task->state.status = TASK_STATUS_INACTIVE;

        break;

    case TASK_STATUS_ACTIVE:
        list_move(&active, &task->state.item);

        task->state.status = TASK_STATUS_ACTIVE;

        break;

    case TASK_STATUS_BLOCKED:
        if (task->state.status != TASK_STATUS_ACTIVE)
            break;

        list_move(&blocked, &task->state.item);

        task->state.status = TASK_STATUS_BLOCKED;

        break;

    case TASK_STATUS_UNBLOCKED:
        if (task->state.status != TASK_STATUS_BLOCKED)
            break;

        list_move(&active, &task->state.item);

        task->state.status = TASK_STATUS_UNBLOCKED;

        break;

    }

}

void task_resume(struct task *task, unsigned int ip, unsigned int sp)
{

    task_setstatus(task, TASK_STATUS_ACTIVE);

    task->state.registers.ip = ip;
    task->state.registers.sp = sp;

}

void task_register(struct task *task)
{

    list_add(&inactive, &task->state.item);

}

void task_unregister(struct task *task)
{

    list_remove(&inactive, &task->state.item);

}

void task_initmailbox(struct task_mailbox *mailbox, struct task *task)
{

    list_inititem(&mailbox->item, mailbox);
    buffer_init(&mailbox->buffer, TASK_MAILBOXSIZE, mailbox->data);

    mailbox->task = task;

}

void task_init(struct task *task, unsigned int id)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    list_inititem(&task->state.item, task);
    task_initmailbox(&task->mailbox, task);

    task->id = id;

}

