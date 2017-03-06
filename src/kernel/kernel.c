#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "container.h"
#include "task.h"
#include "service.h"
#include "kernel.h"

static struct container containers[KERNEL_CONTAINERS];
static struct task tasks[KERNEL_TASKS];
static struct service services[KERNEL_SERVICES];

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

void kernel_multicast(struct list *links, void *buffer, unsigned int count)
{

    struct list_item *current;

    for (current = links->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        task_write(state->task, buffer, count);

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

    task_resume(task, task->format->findentry(&task->node), sp);

    return 1;

}

void kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend)
{

    struct container_server *server = container_getserver(container, 0);
    struct container_mount *mount = container_getmount(container, 0);
    struct service *init = kernel_getservice(task, 8);
    struct service *root = kernel_getservice(task, 9);

    server->backend = backend;
    server->protocol = service_findprotocol(backend);
    mount->parent.server = server;
    mount->parent.id = server->protocol->root(backend);
    mount->child.server = mount->parent.server;
    mount->child.id = mount->parent.id;
    root->server = mount->parent.server;
    root->state.id = mount->parent.id;
    init->server = mount->parent.server;
    init->state.id = mount->parent.id;

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
        task_register(task);

    }

    return &tasks[0];

}

void kernel_setupservices(void)
{

    unsigned int i;

    for (i = 0; i < KERNEL_TASKS; i++)
    {

        struct task *task = &tasks[i];
        unsigned int j;

        for (j = 0; j < TASK_DESCRIPTORS; j++)
            service_init(kernel_getservice(task, j), task);

    }

}

void kernel_setup(void)
{

    binary_setupelf();
    service_setupcpio();

}

