#include <fudge.h>
#include "resource.h"
#include "mmap.h"
#include "binary.h"
#include "service.h"
#include "mailbox.h"
#include "task.h"
#include "memsrv.h"
#include "core.h"
#include "pool.h"
#include "kernel.h"

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

static void retiretask(unsigned int itask)
{

    struct task *task = pool_gettask(itask);
    unsigned int i;

    for (i = 0; i < TASK_MAILBOXES; i++)
    {

        struct mailbox *mailbox = pool_getmailbox(task->imailbox[i]);

        if (mailbox)
        {

            unsigned int inode = mailbox->resource.inode;

            mailbox_reset(mailbox);
            service_unregister(&mailbox->service);

            mailbox->resource.inode = pool_renewnode(inode);

            pool_unpickmailbox(task->imailbox[i]);
            mailbox_wake(inode);

        }

    }

    memsrv_untask(itask);
    task_reset(task);
    pool_unpicktask(itask);

}

static unsigned int transition(unsigned int itask, unsigned int from, unsigned int to)
{

    struct task *task = pool_gettask(itask);
    unsigned int changed;

    spinlock_acquire(&task->spinlock);

    changed = task->state == from;

    if (changed)
    {

        task->state = to;

        switch (to)
        {

        case TASK_STATE_BLOCKED:
            task->signals.block = 0;

            break;

        case TASK_STATE_ASSIGNED:
            if (from == TASK_STATE_BLOCKED)
                task->signals.unblock = 0;

            break;

        case TASK_STATE_RUNNING:
            task->started = 1;

            break;

        }

    }

    spinlock_release(&task->spinlock);

    if (changed && to == TASK_STATE_DEAD)
        retiretask(itask);

    return changed;

}

static void restart(struct task *task)
{

    spinlock_acquire(&task->spinlock);

    task->aborted = TASK_ABORT_DONE;
    task->thread.ip = task->start.ip;
    task->thread.sp = task->start.sp;

    spinlock_release(&task->spinlock);

}

static void unblocktasks(void)
{

    unsigned int itask;

    for (itask = 1; itask < POOL_TASKS; itask++)
    {

        struct task *task = pool_gettask(itask);

        if (task->state == TASK_STATE_BLOCKED)
        {

            if (task->signals.kill)
                transition(itask, TASK_STATE_BLOCKED, TASK_STATE_DEAD);
            else if (task->signals.unblock && transition(itask, TASK_STATE_BLOCKED, TASK_STATE_ASSIGNED))
                assigncorecallback(itask);

        }

    }

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

                mailbox_register(mailbox, itask, ichannel);
                mmap_allocate((struct mmap_header *)task->mmap, MMAP_TYPE_NORMAL, mailbox->data, KERNEL_VMAILBOX + MESSAGE_CAPACITY * ichannel, MESSAGE_CAPACITY, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE);

            }

        }

        return (mailbox) ? mailbox->resource.inode : 0;

    }

    return 0;

}

unsigned int kernel_linknode(struct resource *resource, unsigned int source)
{

    return (pool_getnoderesource(source) && pool_link(&resource->links, source)) ? MESSAGE_OK : MESSAGE_FAILED;

}

unsigned int kernel_unlinknode(struct resource *resource, unsigned int source)
{

    struct list *links = &resource->links;
    struct list_item *current;
    struct list_item *next;

    spinlock_acquire(&links->spinlock);

    for (current = links->head; current; current = next)
    {

        next = current->next;

        if (pool_getlinktarget(current) == source)
        {

            list_remove_unsafe(links, current);
            pool_unlink(current);

        }

    }

    spinlock_release(&links->spinlock);

    return MESSAGE_OK;

}

void kernel_schedule(struct core *core)
{

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        if (task->signals.kill)
            transition(core->itask, TASK_STATE_RUNNING, TASK_STATE_DEAD);
        else if (task->signals.block)
            transition(core->itask, TASK_STATE_RUNNING, TASK_STATE_BLOCKED);
        else if (transition(core->itask, TASK_STATE_RUNNING, TASK_STATE_ASSIGNED))
            pool_placetask(core->itask, &core->tasks);

        core->itask = 0;

    }

    unblocktasks();

    while ((core->itask = pool_picktaskfrom(&core->tasks)))
    {

        struct task *task = pool_gettask(core->itask);

        transition(core->itask, TASK_STATE_ASSIGNED, TASK_STATE_RUNNING);

        if (!task->signals.kill)
        {

            if (task->aborted == TASK_ABORT_PENDING)
                restart(task);

            break;

        }

        transition(core->itask, TASK_STATE_RUNNING, TASK_STATE_DEAD);

    }

}

unsigned int kernel_kill(unsigned int itask)
{

    struct task *task = pool_gettask(itask);
    unsigned int status = 0;

    if (task)
    {

        spinlock_acquire(&task->spinlock);

        if (task->aborted || !task->started)
        {

            task_signal(task, TASK_SIGNAL_KILL);

        }

        else
        {

            task->aborted = TASK_ABORT_PENDING;

            task_signal(task, TASK_SIGNAL_UNBLOCK);

            status = 1;

        }

        spinlock_release(&task->spinlock);

    }

    return status;

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

static struct resource_operands *getoperands(unsigned int inode)
{

    struct resource *resource = pool_getnoderesource(inode);

    return (resource) ? resource->operands : 0;

}

unsigned int kernel_pick(unsigned int source, struct message *messages, unsigned int count, unsigned int release, unsigned int flags)
{

    struct resource_operands *operands = getoperands(source);

    return (operands && operands->pick) ? operands->pick(source, messages, count, release, flags) : MESSAGE_FAILED;

}

unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct resource_operands *operands = getoperands(target);

    return (operands && operands->place) ? operands->place(target, source, event, count, data) : MESSAGE_FAILED;

}

unsigned long kernel_locate(unsigned int target, unsigned int id)
{

    struct resource_operands *operands = getoperands(target);

    return (operands && operands->locate) ? operands->locate(id) : 0;

}

unsigned int kernel_announce(unsigned int inode, char *name)
{

    struct mailbox *mailbox = mailbox_get(inode);

    if (mailbox)
    {

        service_unregister(&mailbox->service);
        service_init(&mailbox->service, name);
        service_register(&mailbox->service, inode);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

void kernel_notify(struct resource *resource, unsigned int event, unsigned int count, void *data)
{

    struct list *links = &resource->links;
    struct list_item *current;
    struct list_item *next;

    spinlock_acquire(&links->spinlock);

    for (current = links->head; current; current = next)
    {

        unsigned int target = pool_getlinktarget(current);

        next = current->next;

        if (pool_getnoderesource(target))
        {

            kernel_place(resource->inode, target, event, count, data);

        }

        else
        {

            list_remove_unsafe(links, current);
            pool_unlink(current);

        }

    }

    spinlock_release(&links->spinlock);

}

static unsigned int loadbinary(struct mmap_header *header, unsigned long base, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < header->nentries; i++)
    {

        struct mmap_entry *entry = &header->entries[i];

        if (entry->type == MMAP_TYPE_BINARY)
        {

            unsigned char *target = (unsigned char *)(entry->paddress + (entry->vaddress & 0xFFF));

            if (id)
            {

                if (memsrv_read(id, target, entry->fsize, entry->foffset) != entry->fsize)
                    return 0;

            }

            else
            {

                buffer_copy(target, (void *)(base + entry->foffset), entry->fsize);

            }

            buffer_clear(target + entry->fsize, entry->msize - entry->fsize);

            entry->type = MMAP_TYPE_NORMAL;

        }

    }

    return 1;

}

unsigned int kernel_loadtask(unsigned int itask, unsigned long base, unsigned int id, unsigned long mmap, unsigned long code, unsigned long stack, unsigned long buffer)
{

    struct task *task = pool_gettask(itask);
    struct binary_format *format = binary_findformat(base);
    unsigned int inode = 0;

    if (task && format)
    {

        struct mmap_header *header = (struct mmap_header *)mmap;

        task_reset(task);
        mmap_initheader(header);
        mmap_allocate(header, MMAP_TYPE_NORMAL, stack, KERNEL_VSTACK - TASK_STACKSIZE, TASK_STACKSIZE, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE);
        mmap_allocate(header, MMAP_TYPE_NORMAL, buffer, KERNEL_VBUFFER, TASK_BUFFERSIZE, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE);

        if (format->map(base, MEMSRV_PAGESIZE, code, TASK_CODESIZE, header) && loadbinary(header, base, id))
            task_register(task, mmap, buffer, format->findentry(base), KERNEL_VSTACK);

        inode = (task->thread.ip) ? kernel_getchannelinode(itask, 0) : 0;

        if (!inode)
            task_reset(task);

    }

    return inode;

}

unsigned long kernel_getbuffer(unsigned int inode)
{

    struct mailbox *mailbox = mailbox_get(inode);

    if (mailbox)
    {

        struct task *task = pool_gettask(mailbox->itask);

        if (task)
            return task->buffer;

    }

    return 0;

}

void kernel_starttask(unsigned int itask)
{

    if (transition(itask, TASK_STATE_DEAD, TASK_STATE_ASSIGNED))
        assigncorecallback(itask);

}

void kernel_setcallback(struct core *(*getcore)(void), void (*assigncore)(unsigned int itask))
{

    getcorecallback = getcore;
    assigncorecallback = assigncore;

}

void kernel_setup(void)
{

    kernel_setcallback(getcore0, assign0);

}

