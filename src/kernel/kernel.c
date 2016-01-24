#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "service.h"
#include "binary.h"
#include "container.h"
#include "kernel.h"

void kernel_copydescriptors(struct container *container, struct task *source, struct task *target)
{

    unsigned int sid = source->id * TASK_DESCRIPTORS;
    unsigned int tid = target->id * TASK_DESCRIPTORS;
    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        container->descriptors[tid + i + 0x00].channel = container->descriptors[sid + i + 0x08].channel;
        container->descriptors[tid + i + 0x00].id = container->descriptors[sid + i + 0x08].id;
        container->descriptors[tid + i + 0x00].offset = 0;
        container->descriptors[tid + i + 0x08].channel = container->descriptors[sid + i + 0x08].channel;
        container->descriptors[tid + i + 0x08].id = container->descriptors[sid + i + 0x08].id;
        container->descriptors[tid + i + 0x08].offset = 0;
        container->descriptors[tid + i + 0x10].channel = 0;
        container->descriptors[tid + i + 0x10].id = 0;
        container->descriptors[tid + i + 0x10].offset = 0;
        container->descriptors[tid + i + 0x18].channel = 0;
        container->descriptors[tid + i + 0x18].id = 0;
        container->descriptors[tid + i + 0x18].offset = 0;

    }

}

unsigned int kernel_setupbinary(struct container *container, struct task *task, unsigned int sp)
{

    struct container_descriptor *descriptor = &container->descriptors[task->id * TASK_DESCRIPTORS];
    struct service_channel *channel = &container->channels[descriptor->channel];
    struct binary_format *format;

    if (!descriptor->id)
        return 0;

    format = binary_findformat(channel, task, 0, descriptor->id);

    if (!format)
        return 0;

    task_resume(task, format->findentry(channel, task, 0, descriptor->id), sp);

    return task->state.registers.ip;

}

unsigned int kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend)
{

    struct service_channel *channel = &container->channels[0x00];
    struct container_mount *mount = &container->mounts[0x00];
    struct container_descriptor *init = &container->descriptors[0x08];
    struct container_descriptor *root = &container->descriptors[0x09];

    channel->backend = backend;
    channel->protocol = service_findprotocol(channel->backend);

    if (!channel->protocol)
        return 0;

    mount->parent.channel = 0x00;
    mount->parent.id = channel->protocol->root(channel->backend);
    mount->child.channel = mount->parent.channel;
    mount->child.id = mount->parent.id;

    if (!mount->parent.id)
        return 0;

    root->channel = mount->parent.channel;
    root->id = mount->parent.id;
    init->channel = 0x00;
    init->id = channel->protocol->child(channel->backend, root->id, 4, "bin/");

    if (!init->id)
        return 0;

    return init->id = channel->protocol->child(channel->backend, init->id, 4, "init");

}

void kernel_setup(void)
{

    binary_setupelf();
    service_setupcpio();

}

