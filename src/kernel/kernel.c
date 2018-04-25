#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"
#include "service.h"
#include "kernel.h"

static struct task tasks[KERNEL_TASKS];
static struct service_mount mounts[KERNEL_MOUNTS];
static struct service services[KERNEL_SERVICES * KERNEL_TASKS];
static struct list usedtasks;
static struct list freetasks;
static struct list blockedtasks;
static struct list usedmounts;
static struct list freemounts;
static struct list freeservices;
static struct core *(*coreget)(void);
static void (*coreassign)(struct task *task);

static unsigned int walkmount(struct service *service, struct service_node *from, struct service_node *to)
{

    if (service->backend == from->backend && service->protocol == from->protocol && service->id == from->id)
    {

        service->backend = to->backend;
        service->protocol = to->protocol;
        service->id = to->id;

        return 1;

    }

    return 0;

}

void kernel_walkmountparent(struct service *service)
{

    struct list_item *current;

    spinlock_acquire(&usedmounts.spinlock);

    for (current = usedmounts.head; current; current = current->next)
    {

        struct service_mount *mount = current->data;

        if (walkmount(service, &mount->child, &mount->parent))
            break;

    }

    spinlock_release(&usedmounts.spinlock);

}

void kernel_walkmountchild(struct service *service)
{

    struct list_item *current;

    spinlock_acquire(&usedmounts.spinlock);

    for (current = usedmounts.head; current; current = current->next)
    {

        struct service_mount *mount = current->data;

        if (walkmount(service, &mount->parent, &mount->child))
            break;

    }

    spinlock_release(&usedmounts.spinlock);

}

struct core *kernel_getcore(void)
{

    return coreget();

}

void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct task *task))
{

    coreget = get;
    coreassign = assign;

}

struct task *kernel_picktask(void)
{

    struct list_item *current = list_picktail(&freetasks);

    return (current) ? current->data : 0;

}

struct service_mount *kernel_pickmount(void)
{

    struct list_item *current = list_picktail(&freemounts);

    return (current) ? current->data : 0;

}

void kernel_usetask(struct task *task)
{

    list_add(&usedtasks, &task->item);

}

void kernel_usemount(struct service_mount *mount)
{

    list_add(&usedmounts, &mount->item);

}

void kernel_freetask(struct task *task)
{

    list_add(&freetasks, &task->item);

}

void kernel_freemount(struct service_mount *mount)
{

    list_add(&freemounts, &mount->item);

}

void kernel_freeservice(struct service *service)
{

    list_add(&freeservices, &service->item);

}

void kernel_blocktask(struct task *task)
{

    task->thread.status = TASK_STATUS_BLOCKED;

    list_add(&blockedtasks, &task->item);

}

void kernel_unblocktask(struct task *task)
{

    if (task->thread.status != TASK_STATUS_BLOCKED)
        return;

    list_remove(&blockedtasks, &task->item);

    task->thread.status = TASK_STATUS_NORMAL;
    task->thread.ip -= task->thread.rewind;
    task->thread.rewind = 0;

    list_add(&usedtasks, &task->item);

}

void kernel_schedule(struct core *core)
{

    struct list_item *current;

    if (core->task)
        list_add(&core->tasks, &core->task->item);

    while ((current = list_pickhead(&usedtasks)))
        coreassign(current->data);

    current = list_picktail(&core->tasks);

    core->task = (current) ? current->data : 0;

}

struct service *kernel_getservice(struct task *task, unsigned int service)
{

    return &services[task->id * KERNEL_SERVICES + (service & (KERNEL_SERVICES - 1))];

}

static void copyservice(struct service *tservice, struct service *sservice, struct task *task)
{

    tservice->backend = (sservice) ? sservice->backend : 0;
    tservice->protocol = (sservice) ? sservice->protocol : 0;
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

void kernel_multicast(struct list *states, void *buffer, unsigned int count)
{

    struct list_item *current;

    spinlock_acquire(&states->spinlock);

    for (current = states->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        task_writeall(state->task, buffer, count);
        kernel_unblocktask(state->task);

    }

    spinlock_release(&states->spinlock);

}

unsigned int kernel_setupbinary(struct task *task, unsigned int sp)
{

    struct service *service = kernel_getservice(task, 0);

    task->node.address = service->protocol->map(service->backend, &service->state, service->id);

    if (!task->node.address)
        return 0;

    task->format = binary_findformat(&task->node);

    if (!task->format)
        return 0;

    task->thread.ip = task->format->findentry(&task->node);
    task->thread.sp = sp;

    return 1;

}

void kernel_setupinit(struct task *task)
{

    struct service *init = kernel_getservice(task, 8);
    struct service *root = kernel_getservice(task, 9);
    struct service_mount *mount = kernel_pickmount();

    root->backend = service_findbackend(1000);
    root->protocol = service_findprotocol(1000);
    root->id = root->protocol->root(root->backend, &root->state);

    init->backend = root->backend;
    init->protocol = root->protocol;
    init->id = init->protocol->root(init->backend, &init->state);
    init->id = init->protocol->child(init->backend, &init->state, init->id, "bin", 3);
    init->id = init->protocol->child(init->backend, &init->state, init->id, "init", 4);
    mount->parent.backend = root->backend;
    mount->parent.protocol = root->protocol;
    mount->parent.id = root->id;
    mount->child.backend = root->backend;
    mount->child.protocol = root->protocol;
    mount->child.id = root->id;

    kernel_usemount(mount);

}

void kernel_setup(void)
{

    unsigned int i;

    for (i = 0; i < KERNEL_TASKS; i++)
    {

        struct task *task = &tasks[i];

        task_init(task, i);
        kernel_freetask(task);

    }

    for (i = 0; i < KERNEL_MOUNTS; i++)
    {

        struct service_mount *mount = &mounts[i];

        service_initmount(mount);
        kernel_freemount(mount);

    }

    for (i = 0; i < KERNEL_SERVICES * KERNEL_TASKS; i++)
    {

        struct service *service = &services[i];

        service_init(service);
        kernel_freeservice(service);

    }

}

