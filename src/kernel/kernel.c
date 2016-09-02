#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "service.h"
#include "container.h"
#include "task.h"
#include "kernel.h"

static void copydescriptor(struct task_descriptor *tdescriptor, struct task_descriptor *sdescriptor)
{

    tdescriptor->server = (sdescriptor) ? sdescriptor->server : 0;
    tdescriptor->state.id = (sdescriptor) ? sdescriptor->state.id : 0;

}

void kernel_copydescriptors(struct task *source, struct task *target)
{

    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        copydescriptor(&target->descriptors[i + 0x00], &source->descriptors[i + 0x08]);
        copydescriptor(&target->descriptors[i + 0x08], &source->descriptors[i + 0x08]);
        copydescriptor(&target->descriptors[i + 0x10], 0);
        copydescriptor(&target->descriptors[i + 0x18], 0);

    }

}

unsigned int kernel_setupbinary(struct task *task, unsigned int sp)
{

    struct task_descriptor *descriptor = &task->descriptors[0x00];
    struct container_server *server = descriptor->server;
    struct service_state *state = &descriptor->state;
    struct binary_node *node = &descriptor->node;

    node->physical = server->protocol->map(server->backend, state->id);

    if (!node->physical)
        return 0;

    task->format = binary_findformat(node);

    if (!task->format)
        return 0;

    task_resume(task, task->format->findentry(node), sp);

    return 1;

}

void kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend)
{

    struct container_server *server = &container->servers[0x00];
    struct container_mount *mount = &container->mounts[0x00];
    struct task_descriptor *init = &task->descriptors[0x08];
    struct task_descriptor *root = &task->descriptors[0x09];

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

    init->server->protocol->child(init->server->backend, &init->state, "bin/", 4);
    init->server->protocol->child(init->server->backend, &init->state, "init", 4);

    container->nservers++;
    container->nmounts++;

}

void kernel_setup(void)
{

    binary_setupelf();
    service_setupcpio();

}

