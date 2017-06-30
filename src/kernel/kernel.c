#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "service.h"
#include "container.h"
#include "kernel.h"

static struct container containers[KERNEL_CONTAINERS];
static struct task tasks[KERNEL_TASKS];
static struct service services[KERNEL_SERVICES];
static struct list active;
static struct list inactive;
static struct list blocked;

struct task *kernel_findactivetask(void)
{

    return (active.tail) ? active.tail->data : 0;

}

struct task *kernel_findinactivetask(void)
{

    return (inactive.tail) ? inactive.tail->data : 0;

}

void kernel_activatetask(struct task *task)
{

    switch (task->state.status)
    {

    case TASK_STATUS_ACTIVE:
    case TASK_STATUS_INACTIVE:
    case TASK_STATUS_UNBLOCKED:
        list_move(&active, &task->state.item);

        task->state.status = TASK_STATUS_ACTIVE;

        break;

    }

}

void kernel_inactivatetask(struct task *task)
{

    switch (task->state.status)
    {

    case TASK_STATUS_ACTIVE:
        list_move(&inactive, &task->state.item);

        task->state.status = TASK_STATUS_INACTIVE;

        break;

    }

}

void kernel_blocktask(struct task *task)
{

    switch (task->state.status)
    {

    case TASK_STATUS_ACTIVE:
        list_move(&blocked, &task->state.item);

        task->state.status = TASK_STATUS_BLOCKED;

        break;

    }

}

void kernel_unblocktask(struct task *task)
{

    switch (task->state.status)
    {

    case TASK_STATUS_BLOCKED:
    case TASK_STATUS_UNBLOCKED:
        list_move(&active, &task->state.item);

        task->state.status = TASK_STATUS_UNBLOCKED;

        break;

    }

}

struct service *kernel_getservice(struct task *task, unsigned int service)
{

    return &services[task->id * TASK_DESCRIPTORS + (service & (TASK_DESCRIPTORS - 1))];

}

static void copyservice(struct service *tservice, struct service *sservice)
{

    tservice->server = (sservice) ? sservice->server : 0;
    tservice->state.id = (sservice) ? sservice->state.id : 0;

}

void kernel_copyservices(struct task *source, struct task *target)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        copyservice(kernel_getservice(target, i + 0), kernel_getservice(source, i + 8));
        copyservice(kernel_getservice(target, i + 8), kernel_getservice(source, i + 8));
        copyservice(kernel_getservice(target, i + 16), 0);
        copyservice(kernel_getservice(target, i + 24), 0);

    }

}

unsigned int kernel_readtask(struct task *task, void *buffer, unsigned int count)
{

    count = ring_read(&task->mailbox, buffer, count);

    if (!count)
        kernel_blocktask(task);

    return count;

}

unsigned int kernel_writetask(struct task *task, void *buffer, unsigned int count)
{

    kernel_unblocktask(task);

    return (count < ring_avail(&task->mailbox)) ? ring_write(&task->mailbox, buffer, count) : 0;

}

void kernel_multicast(struct list *links, void *buffer, unsigned int count)
{

    struct list_item *current;

    for (current = links->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        kernel_writetask(state->task, buffer, count);

    }

}

unsigned int kernel_setupbinary(struct task *task, unsigned int sp)
{

    struct service *service = kernel_getservice(task, 0);

    task->node.physical = service->server->protocol->map(service->server->backend, &service->state);

    if (!task->node.physical)
        return 0;

    task->format = binary_findformat(&task->node);

    if (!task->format)
        return 0;

    task_setstate(task, task->format->findentry(&task->node), sp);

    return 1;

}

void kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend)
{

    struct service_server *server = container_getserver(container, 0);
    struct service *init = kernel_getservice(task, 8);
    struct service *root = kernel_getservice(task, 9);
    struct container_mount *mount = container_getmount(container, 0);

    server->backend = backend;
    server->protocol = service_findprotocol(backend);
    root->server = server;
    root->state.id = server->protocol->root(backend);
    init->server = server;
    init->state.id = server->protocol->root(backend);
    mount->parent.server = root->server;
    mount->parent.id = root->state.id;
    mount->child.server = root->server;
    mount->child.id = root->state.id;

    server->protocol->child(server->backend, &init->state, "bin/", 4);
    server->protocol->child(server->backend, &init->state, "init", 4);

    container->nservers++;
    container->nmounts++;

}

struct container *kernel_setupcontainers(void)
{

    unsigned int i;

    for (i = 0; i < KERNEL_CONTAINERS; i++)
    {

        struct container *container = &containers[i];

        container_init(container, i);

    }

    return &containers[0];

}

struct task *kernel_setuptasks(void)
{

    unsigned int i;

    for (i = 0; i < KERNEL_TASKS; i++)
    {

        struct task *task = &tasks[i];

        task_init(task, i);
        list_add(&inactive, &task->state.item);

    }

    return kernel_findinactivetask();

}

void kernel_setupservices(void)
{

    unsigned int i;

    for (i = 0; i < KERNEL_SERVICES; i++)
    {

        struct service *service = &services[i];

        service_init(service, &tasks[i / TASK_DESCRIPTORS]);

    }

}

