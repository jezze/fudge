#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"
#include "service.h"
#include "kernel.h"

static struct task tasks[KERNEL_TASKS];
static struct service_server servers[KERNEL_SERVERS];
static struct service_mount mounts[KERNEL_MOUNTS];
static struct service services[KERNEL_SERVICES];
static struct list freetasks;
static struct list blockedtasks;
static struct list unblockedtasks;
static struct list usedservers;
static struct list freeservers;
static struct list usedmounts;
static struct list freemounts;
static struct list freeservices;
static struct spinlock tasklock;

struct task *kernel_getfreetask(void)
{

    return (freetasks.tail) ? freetasks.tail->data : 0;

}

struct service_server *kernel_getfreeserver(void)
{

    return (freeservers.tail) ? freeservers.tail->data : 0;

}

struct service_mount *kernel_getfreemount(void)
{

    return (freemounts.tail) ? freemounts.tail->data : 0;

}

static unsigned int walkmount(struct service *service, struct service_node *from, struct service_node *to)
{

    if (service->server == from->server && service->id == from->id)
    {

        service->server = to->server;
        service->id = to->id;

        return 1;

    }

    return 0;

}

void kernel_walkmountparent(struct service *service)
{

    struct list_item *current;

    for (current = usedmounts.head; current; current = current->next)
    {

        struct service_mount *mount = current->data;

        if (walkmount(service, &mount->child, &mount->parent))
            break;

    }

}

void kernel_walkmountchild(struct service *service)
{

    struct list_item *current;

    for (current = usedmounts.head; current; current = current->next)
    {

        struct service_mount *mount = current->data;

        if (walkmount(service, &mount->parent, &mount->child))
            break;

    }

}

struct service *kernel_getservice(struct task *task, unsigned int service)
{

    return &services[task->id * TASK_DESCRIPTORS + (service & (TASK_DESCRIPTORS - 1))];

}

void kernel_assigntask(struct core *core, struct task *task)
{

    spinlock_hold(&tasklock);

    switch (task->state.status)
    {

    case TASK_STATUS_FREE:
        list_move(&core->tasks, &task->state.item);

        task->state.status = TASK_STATUS_ACTIVE;

        break;

    }

    spinlock_release(&tasklock);

}

void kernel_freetask(struct task *task)
{

    spinlock_hold(&tasklock);

    switch (task->state.status)
    {

    case TASK_STATUS_ACTIVE:
        list_move(&freetasks, &task->state.item);

        task->state.status = TASK_STATUS_FREE;

        break;

    }

    spinlock_release(&tasklock);

}

void kernel_blocktask(struct task *task)
{

    spinlock_hold(&tasklock);

    switch (task->state.status)
    {

    case TASK_STATUS_ACTIVE:
        list_move(&blockedtasks, &task->state.item);

        task->state.status = TASK_STATUS_BLOCKED;

        break;

    }

    spinlock_release(&tasklock);

}

void kernel_unblocktask(struct task *task)
{

    spinlock_hold(&tasklock);

    switch (task->state.status)
    {

    case TASK_STATUS_BLOCKED:
        list_move(&unblockedtasks, &task->state.item);

        task->state.status = TASK_STATUS_UNBLOCKED;

        break;

    }

    spinlock_release(&tasklock);

}

struct task *kernel_schedule(struct core *core)
{

    struct list_item *current;

    spinlock_hold(&tasklock);

    for (current = unblockedtasks.head; current; current = current->next)
    {

        struct task *task = current->data;

        list_move(&core->tasks, current);

        task->state.status = TASK_STATUS_ACTIVE;
        task->state.ip -= task->state.rewind;
        task->state.rewind = 0;

    }

    spinlock_release(&tasklock);

    return (core->tasks.tail) ? core->tasks.tail->data : 0;

}

void kernel_useserver(struct service_server *server)
{

    list_move(&usedservers, &server->item);

}

void kernel_freeserver(struct service_server *server)
{

    list_move(&freeservers, &server->item);

}

void kernel_usemount(struct service_mount *mount)
{

    list_move(&usedmounts, &mount->item);

}

void kernel_freemount(struct service_mount *mount)
{

    list_move(&freemounts, &mount->item);

}

static void copyservice(struct service *tservice, struct service *sservice, struct task *task)
{

    tservice->server = (sservice) ? sservice->server : 0;
    tservice->id = (sservice) ? sservice->id : 0;
    tservice->state.task = task;

}

void kernel_copyservices(struct task *source, struct task *target)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        copyservice(kernel_getservice(target, i + 0), kernel_getservice(source, i + 8), target);
        copyservice(kernel_getservice(target, i + 8), kernel_getservice(source, i + 8), target);
        copyservice(kernel_getservice(target, i + 16), 0, target);
        copyservice(kernel_getservice(target, i + 24), 0, target);

    }

}

unsigned int kernel_readmailbox(struct task *task, void *buffer, unsigned int count)
{

    spinlock_hold(&task->mailbox.spinlock);

    count = ring_read(&task->mailbox.ring, buffer, count);

    if (!count)
        kernel_blocktask(task);

    spinlock_release(&task->mailbox.spinlock);

    return count;

}

unsigned int kernel_writemailbox(struct task *task, void *buffer, unsigned int count)
{

    spinlock_hold(&task->mailbox.spinlock);

    count = ring_writeall(&task->mailbox.ring, buffer, count);

    if (count)
        kernel_unblocktask(task);

    spinlock_release(&task->mailbox.spinlock);

    return count;

}

void kernel_unblockall(struct list *states)
{

    struct list_item *current;

    for (current = states->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        kernel_unblocktask(state->task);

    }

}

void kernel_multicast(struct list *states, void *buffer, unsigned int count)
{

    struct list_item *current;

    for (current = states->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        kernel_writemailbox(state->task, buffer, count);

    }

}

unsigned int kernel_setupbinary(struct task *task, unsigned int sp)
{

    struct service *service = kernel_getservice(task, 0);

    task->node.physical = service->server->protocol->map(service->server->backend, &service->state, service->id);

    if (!task->node.physical)
        return 0;

    task->format = binary_findformat(&task->node);

    if (!task->format)
        return 0;

    task->state.ip = task->format->findentry(&task->node);
    task->state.sp = sp;

    return 1;

}

void kernel_setupramdisk(struct task *task, struct service_backend *backend)
{

    struct service *init = kernel_getservice(task, 8);
    struct service *root = kernel_getservice(task, 9);
    struct service_server *server = kernel_getfreeserver();
    struct service_mount *mount = kernel_getfreemount();

    server->backend = backend;
    server->protocol = service_findprotocol(1000);
    server->root = server->protocol->root(backend);
    mount->parent.server = server;
    mount->parent.id = server->root;
    mount->child.server = server;
    mount->child.id = server->root;
    root->server = server;
    root->id = server->root;
    init->server = server;
    init->id = server->root;
    init->id = server->protocol->child(server->backend, &init->state, init->id, "bin", 3);
    init->id = server->protocol->child(server->backend, &init->state, init->id, "init", 4);

    kernel_useserver(server);
    kernel_usemount(mount);

}

void kernel_setuptasks(void)
{

    unsigned int i;

    for (i = 0; i < KERNEL_TASKS; i++)
    {

        struct task *task = &tasks[i];

        task_init(task, i);
        list_add(&freetasks, &task->state.item);

    }

}

void kernel_setupservers(void)
{

    unsigned int i;

    for (i = 0; i < KERNEL_SERVERS; i++)
    {

        struct service_server *server = &servers[i];

        service_initserver(server);
        list_add(&freeservers, &server->item);

    }

}

void kernel_setupmounts(void)
{

    unsigned int i;

    for (i = 0; i < KERNEL_MOUNTS; i++)
    {

        struct service_mount *mount = &mounts[i];

        service_initmount(mount);
        list_add(&freemounts, &mount->item);

    }

}

void kernel_setupservices(void)
{

    unsigned int i;

    for (i = 0; i < KERNEL_SERVICES; i++)
    {

        struct service *service = &services[i];

        service_init(service);
        list_add(&freeservices, &service->item);

    }

}

