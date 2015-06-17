#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"
#include "rendezvous.h"

static unsigned int read(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&task->mailbox.buffer, size, count, buffer);

    if (count)
        scheduler_unblockspecial(task);
    else
        scheduler_block(task);

    return count;

}

static unsigned int write(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = buffer_wcfifo(&task->mailbox.buffer, size, count, buffer);

    if (count)
        scheduler_unblockspecial(task);
    else
        scheduler_block(task);

    return count;

}

static void attach(struct task *task, struct list *mailboxes)
{

    list_add(mailboxes, &task->mailbox.item);

}

static void detach(struct task *task, struct list *mailboxes)
{

    list_remove(mailboxes, &task->mailbox.item);
    scheduler_unblockspecial(task);

}

void rendezvous_attach(struct list *mailboxes)
{

    struct task *task = scheduler_findactive();

    attach(task, mailboxes);

}

void rendezvous_detach(struct list *mailboxes)
{

    struct task *task = scheduler_findactive();

    detach(task, mailboxes);

}

void rendezvous_detachlist(struct list *mailboxes)
{

    struct list_item *current;

    for (current = mailboxes->head; current; current = current->next)
    {

        struct task *task = current->data;

        detach(task, mailboxes);

    }

}

unsigned int rendezvous_read(unsigned int size, unsigned int count, void *buffer)
{

    struct task *task = scheduler_findactive();

    return read(task, size, count, buffer);

}

unsigned int rendezvous_write(unsigned int id, unsigned int size, unsigned int count, void *buffer)
{

    struct task *task = (struct task *)id;

    return write(task, size, count, buffer);

}

void rendezvous_writelist(struct list *mailboxes, unsigned int size, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = mailboxes->head; current; current = current->next)
    {

        struct task *task = current->data;

        write(task, size, count, buffer);

    }

}

