#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "service.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

static void copydescriptor(struct task_descriptor *tdescriptor, struct task_descriptor *sdescriptor)
{

    tdescriptor->backend = (sdescriptor) ? sdescriptor->backend : 0;
    tdescriptor->protocol = (sdescriptor) ? sdescriptor->protocol : 0;
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

    if (!descriptor->state.id)
        return 0;

    if (!descriptor->protocol->map(descriptor->backend, &descriptor->state, &descriptor->node))
        return 0;

    task->format = binary_findformat(&descriptor->node);

    if (!task->format)
        return 0;

    task_resume(task, task->format->findentry(&descriptor->node), sp);

    return task->state.registers.ip;

}

unsigned int kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend)
{

    struct container_mount *mount = &container->mounts[0x00];
    struct task_descriptor *init = &task->descriptors[0x08];
    struct task_descriptor *root = &task->descriptors[0x09];

    mount->parent.backend = backend;
    mount->parent.protocol = service_findprotocol(backend);
    mount->parent.id = mount->parent.protocol->root(backend);
    mount->child.backend = mount->parent.backend;
    mount->child.protocol = mount->parent.protocol;
    mount->child.id = mount->parent.id;
    root->backend = mount->parent.backend;
    root->protocol = mount->parent.protocol;
    root->state.id = mount->parent.id;
    init->backend = mount->parent.backend;
    init->protocol = mount->parent.protocol;
    init->state.id = mount->parent.id;
    init->state.id = init->protocol->child(init->backend, &init->state, 4, "bin/");
    init->state.id = init->protocol->child(init->backend, &init->state, 4, "init");

    return init->state.id;

}

void kernel_setup(void)
{

    binary_setupelf();
    service_setupcpio();

}

