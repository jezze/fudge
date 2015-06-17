#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"
#include "rendezvous.h"

unsigned int rendezvous_read(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&task->mailbox.buffer, size, count, buffer);

    if (count)
        scheduler_unblockspecial(task);
    else
        scheduler_block(task);

    return count;

}

unsigned int rendezvous_write(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = buffer_wcfifo(&task->mailbox.buffer, size, count, buffer);

    if (count)
        scheduler_unblockspecial(task);
    else
        scheduler_block(task);

    return count;

}

void rendezvous_writelist(struct list *mailboxes, unsigned int size, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = mailboxes->head; current; current = current->next)
    {

        struct task *task = current->data;

        rendezvous_write(task, size, count, buffer);

    }

}

