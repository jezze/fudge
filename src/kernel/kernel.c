#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "container.h"
#include "task.h"
#include "service.h"
#include "kernel.h"

static void copydescriptor(struct task *target, struct service_descriptor *tdescriptor, struct service_descriptor *sdescriptor)
{

    tdescriptor->server = (sdescriptor) ? sdescriptor->server : 0;
    tdescriptor->state.id = (sdescriptor) ? sdescriptor->state.id : 0;

}

void kernel_copydescriptors(struct task *source, struct task *target)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        copydescriptor(target, service_getdescriptor(target, i + 0), service_getdescriptor(source, i + 8));
        copydescriptor(target, service_getdescriptor(target, i + 8), service_getdescriptor(source, i + 8));
        copydescriptor(target, service_getdescriptor(target, i + 16), 0);
        copydescriptor(target, service_getdescriptor(target, i + 24), 0);

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

    struct service_descriptor *descriptor = service_getdescriptor(task, 0);

    task->node.physical = descriptor->server->protocol->map(descriptor->server->backend, &descriptor->state);

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

    struct container_server *server = &container->servers[0];
    struct container_mount *mount = &container->mounts[0];
    struct service_descriptor *init = service_getdescriptor(task, 8);
    struct service_descriptor *root = service_getdescriptor(task, 9);

    server->backend = backend;
    server->protocol = service_findprotocol(backend);
    mount->parent.server = server;
    mount->parent.id = server->protocol->root(backend);
    mount->child.server = server;
    mount->child.id = server->protocol->root(backend);

    root->server = mount->parent.server;
    root->state.id = mount->parent.id;
    init->server = mount->parent.server;
    init->state.id = mount->parent.id;

    server->protocol->child(server->backend, &init->state, "bin/", 4);
    server->protocol->child(server->backend, &init->state, "init", 4);

    container->nservers++;
    container->nmounts++;

}

void kernel_setup(void)
{

    binary_setupelf();
    service_setupcpio();

}

