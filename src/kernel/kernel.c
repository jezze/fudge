#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "service.h"
#include "binary.h"
#include "container.h"
#include "kernel.h"

void kernel_copysessions(struct container *container, struct task *source, struct task *target)
{

    unsigned int sid = source->id * TASK_DESCRIPTORS;
    unsigned int tid = target->id * TASK_DESCRIPTORS;
    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        container->sessions[tid + i + 0x00].channel = container->sessions[sid + i + 0x08].channel;
        container->sessions[tid + i + 0x00].id = container->sessions[sid + i + 0x08].id;
        container->sessions[tid + i + 0x00].offset = 0;
        container->sessions[tid + i + 0x08].channel = container->sessions[sid + i + 0x08].channel;
        container->sessions[tid + i + 0x08].id = container->sessions[sid + i + 0x08].id;
        container->sessions[tid + i + 0x08].offset = 0;
        container->sessions[tid + i + 0x10].channel = 0;
        container->sessions[tid + i + 0x10].id = 0;
        container->sessions[tid + i + 0x10].offset = 0;
        container->sessions[tid + i + 0x18].channel = 0;
        container->sessions[tid + i + 0x18].id = 0;
        container->sessions[tid + i + 0x18].offset = 0;

    }

}

unsigned int kernel_setupbinary(struct container *container, struct task *task, unsigned int sp)
{

    struct container_session *session = &container->sessions[task->id * TASK_DESCRIPTORS];
    struct service_channel *channel = &container->channels[session->channel];
    struct binary_format *format;

    if (!session->id)
        return 0;

    format = binary_findformat(channel->protocol, channel->backend, task, 0, session->id);

    if (!format)
        return 0;

    task_resume(task, format->findentry(channel->protocol, channel->backend, task, 0, session->id), sp);

    return task->state.registers.ip;

}

unsigned int kernel_setupramdisk(struct container *container, struct task *task, struct service_backend *backend)
{

    struct service_channel *channel = &container->channels[0x00];
    struct container_mount *mount = &container->mounts[0x00];
    struct container_session *init = &container->sessions[0x08];
    struct container_session *root = &container->sessions[0x09];

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

