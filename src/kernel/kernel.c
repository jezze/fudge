#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "mmap.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "node.h"
#include "service.h"
#include "core.h"
#include "pool.h"
#include "kernel.h"

static struct list blockedtasks;
static struct core *(*getcorecallback)(void);
static void (*assigncorecallback)(unsigned int itask);

static struct core *getcore0(void)
{

    return pool_getcore(0);

}

static void assign0(unsigned int itask)
{

    struct core *core = pool_getcore(0);

    if (core)
        pool_placetask(itask, &core->tasks);

}

static void destroytask(unsigned int itask)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        unsigned int i;

        for (i = 0; i < TASK_MAILBOXES; i++)
        {

            if (task->imailbox[i])
            {

                struct mailbox *mailbox = pool_getmailbox(task->imailbox[i]);

                if (mailbox)
                {

                    mailbox_reset(mailbox);
                    mailbox_unregister(mailbox);
                    pool_unpickmailbox(task->imailbox[i]);

                }

            }

        }

        /* DESTROY SERVICES HERE */

        task_reset(task);
        task_unregister(task);
        pool_unpicktask(itask);

    }

}

static void transition(unsigned int itask, unsigned int state)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        spinlock_acquire(&task->spinlock);

        if (task_transition(task, state))
        {

            switch (task->state)
            {

            case TASK_STATE_DEAD:
                destroytask(itask);

                break;

            case TASK_STATE_BLOCKED:
                pool_placetask(itask, &blockedtasks);

                break;

            case TASK_STATE_ASSIGNED:
                assigncorecallback(itask);

                break;

            }

        }

        spinlock_release(&task->spinlock);

    }

}

static void unblocktasks(void)
{

    struct list_item *current;
    struct list_item *next;

    spinlock_acquire(&blockedtasks.spinlock);

    for (current = blockedtasks.head; current; current = next)
    {

        unsigned int itask = pool_getitaskfromitem(current);

        next = current->next;

        if (itask)
        {

            struct task *task = pool_gettask(itask);

            if (task)
            {

                if (task->signals.kill)
                {

                    list_remove_unsafe(&blockedtasks, current);
                    transition(itask, TASK_STATE_UNBLOCKED);
                    transition(itask, TASK_STATE_DEAD);

                }

                if (task->signals.unblock)
                {

                    list_remove_unsafe(&blockedtasks, current);
                    transition(itask, TASK_STATE_UNBLOCKED);
                    transition(itask, TASK_STATE_ASSIGNED);

                }

            }

        }

    }

    spinlock_release(&blockedtasks.spinlock);

}

struct core *kernel_getcore(void)
{

    return getcorecallback();

}

unsigned int kernel_getchannelinode(unsigned int itask, unsigned int ichannel)
{

    struct task *task = pool_gettask(itask);

    if (task && ichannel < TASK_MAILBOXES)
    {

        struct mailbox *mailbox = pool_getmailbox(task->imailbox[ichannel]);

        if (!mailbox)
        {

            task->imailbox[ichannel] = pool_pickmailbox();

            if (task->imailbox[ichannel])
            {

                mailbox = pool_getmailbox(task->imailbox[ichannel]);

                mailbox_reset(mailbox);
                mailbox_register(mailbox, itask, ichannel);
                mmap_allocate((struct mmap_header *)task->mmap, MMAP_TYPE_NORMAL, mailbox->data, KERNEL_VMAILBOX + MESSAGE_CAPACITY * ichannel, MESSAGE_CAPACITY, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE);

            }

        }

        return (mailbox) ? mailbox->inode : 0;

    }

    return 0;

}

unsigned int kernel_linknode(struct list *links, unsigned int source)
{

    unsigned int inode = pool_picknode(pool_getnoderesource(source), pool_getnodeoperands(source));

    if (inode)
    {

        pool_placenode(inode, links);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

unsigned int kernel_unlinknode(struct list *links, unsigned int source)
{

    struct resource *sresource = pool_getnoderesource(source);

    if (sresource)
    {

        struct list_item *current;
        struct list_item *next;

        spinlock_acquire(&links->spinlock);

        for (current = links->head; current; current = next)
        {

            unsigned int inode = pool_getinodefromitem(current);

            next = current->next;

            if (pool_getnoderesource(inode) == sresource)
            {

                list_remove_unsafe(links, current);
                pool_unpicknode(inode);

            }

        }

        spinlock_release(&links->spinlock);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

void kernel_schedule(struct core *core)
{

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        if (task)
        {

            if (task->signals.kill)
            {

                transition(core->itask, TASK_STATE_DEAD);

                core->itask = 0;

            }

            if (task->signals.block)
            {

                transition(core->itask, TASK_STATE_BLOCKED);

                core->itask = 0;

            }

            if (core->itask)
            {

                transition(core->itask, TASK_STATE_ASSIGNED);

                core->itask = 0;

            }

        }

    }

    unblocktasks();

    core->itask = pool_picktaskfrom(&core->tasks);

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        if (task)
        {

            transition(core->itask, TASK_STATE_RUNNING);

        }

    }

}

void kernel_signal(unsigned int itask, unsigned int signal)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        spinlock_acquire(&task->spinlock);
        task_signal(task, signal);
        spinlock_release(&task->spinlock);

    }

}

unsigned int kernel_pick(unsigned int source, struct message *message)
{

    struct node_operands *operands = pool_getnodeoperands(source);

    return (operands && operands->pick) ? operands->pick(source, message) : MESSAGE_FAILED;

}

unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct node_operands *operands = pool_getnodeoperands(target);

    return (operands && operands->place) ? operands->place(target, source, event, count, data) : MESSAGE_FAILED;

}

unsigned int kernel_announce(unsigned int inode, char *name)
{

    unsigned int iservice = pool_pickservice();

    if (iservice)
    {

        struct service *service = pool_getservice(iservice);

        if (service)
            service_register(service, inode, name);

    }

    return 0;

}

void kernel_notify(struct list *links, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct list_item *current = 0;

    spinlock_acquire(&links->spinlock);

    while ((current = list_next_unsafe(links, current)))
    {

        unsigned int target = pool_getinodefromitem(current);

        if (target)
            kernel_place(source, target, event, count, data);

    }

    spinlock_release(&links->spinlock);

}

unsigned int kernel_loadtask(unsigned int itask, unsigned long ip, unsigned long sp, unsigned long address, unsigned long mmap, unsigned long code, unsigned long stack)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        task_reset(task);
        task_register(task, address, mmap, ip, sp);

        if (task->address)
        {

            struct binary_format *format = binary_findformat(task->address);

            if (format)
            {

                struct mmap_header *header = (struct mmap_header *)task->mmap;

                mmap_initheader(header);
                mmap_allocate(header, MMAP_TYPE_NORMAL, stack, KERNEL_VSTACK - TASK_STACKSIZE, TASK_STACKSIZE, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE);

                format->map(task->address, code, header);

                task->thread.ip = format->findentry(task->address);

            }

        }

        if (task->thread.ip)
        {

            unsigned int inode = kernel_getchannelinode(itask, 0);

            if (inode)
            {

                transition(itask, TASK_STATE_NEW);
                transition(itask, TASK_STATE_ASSIGNED);

                return inode;

            }

        }

    }

    return 0;

}

void kernel_setcallback(struct core *(*getcore)(void), void (*assigncore)(unsigned int itask))
{

    getcorecallback = getcore;
    assigncorecallback = assigncore;

}

void kernel_setup(void)
{

    list_init(&blockedtasks);
    kernel_setcallback(getcore0, assign0);

}

