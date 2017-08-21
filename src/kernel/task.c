#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"

void task_setstate(struct task *task, unsigned int ip, unsigned int sp)
{

    task->state.ip = ip;
    task->state.sp = sp;
    task->state.rewind = 0;

}

void task_initstate(struct task_state *state, struct task *task)
{

    list_inititem(&state->item, task);

}

void task_initmailbox(struct task_mailbox *mailbox)
{

    ring_init(&mailbox->ring, TASK_MAILBOXSIZE, mailbox->data);

}

void task_init(struct task *task, unsigned int id)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    task_initstate(&task->state, task);
    task_initmailbox(&task->mailbox);

    task->id = id;

}

