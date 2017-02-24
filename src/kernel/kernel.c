#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "container.h"
#include "task.h"
#include "service.h"
#include "kernel.h"

static void copydescriptor(struct task *target, struct task_descriptor *tdescriptor, struct task_descriptor *sdescriptor)
{

    list_inititem(&tdescriptor->link, target);

    tdescriptor->server = (sdescriptor) ? sdescriptor->server : 0;
    tdescriptor->id = (sdescriptor) ? sdescriptor->id : 0;
    tdescriptor->offset = 0;
    tdescriptor->current = 0;

}

void kernel_copydescriptors(struct task *source, struct task *target)
{

    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        copydescriptor(target, &target->descriptors[i + 0x00], &source->descriptors[i + 0x08]);
        copydescriptor(target, &target->descriptors[i + 0x08], &source->descriptors[i + 0x08]);
        copydescriptor(target, &target->descriptors[i + 0x10], 0);
        copydescriptor(target, &target->descriptors[i + 0x18], 0);

    }

}

void kernel_multicast(struct list *links, void *buffer, unsigned int count)
{

    struct list_item *current;

    for (current = links->head; current; current = current->next)
    {

        struct task *task = current->data;

        task_write(task, buffer, count);

    }

}

unsigned int kernel_setupbinary(struct task *task, unsigned int sp)
{

    struct task_descriptor *descriptor = &task->descriptors[0x00];

    task->node.physical = descriptor->server->protocol->map(descriptor);

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

    list_inititem(&root->link, task);

    root->server = mount->parent.server;
    root->id = mount->parent.id;
    root->offset = 0;
    root->current = 0;

    list_inititem(&init->link, task);

    init->server = mount->parent.server;
    init->id = mount->parent.id;
    init->offset = 0;
    init->current = 0;

    init->server->protocol->child(init, "bin/", 4);
    init->server->protocol->child(init, "init", 4);

    container->nservers++;
    container->nmounts++;

}

void kernel_setup(void)
{

    binary_setupelf();
    service_setupcpio();

}

