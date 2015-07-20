#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

void kernel_copytask(struct task *task, struct task *next)
{

    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        next->descriptors[i + 0x00].channel = task->descriptors[i + 0x08].channel;
        next->descriptors[i + 0x00].id = task->descriptors[i + 0x08].id;
        next->descriptors[i + 0x08].channel = task->descriptors[i + 0x08].channel;
        next->descriptors[i + 0x08].id = task->descriptors[i + 0x08].id;
        next->descriptors[i + 0x10].channel = 0;
        next->descriptors[i + 0x10].id = 0;
        next->descriptors[i + 0x18].channel = 0;
        next->descriptors[i + 0x18].id = 0;

    }

}

void kernel_copyprogram(struct task *task)
{

    struct vfs_descriptor *descriptor = &task->descriptors[0x00];
    struct binary_protocol *protocol;

    if (!descriptor->id || !descriptor->channel)
        return;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return;

    protocol->copyprogram(descriptor->channel, descriptor->id);

}

void kernel_setuptask(struct task *task, unsigned int sp)
{

    struct vfs_descriptor *descriptor = &task->descriptors[0x00];
    struct binary_protocol *protocol;

    if (!descriptor->id || !descriptor->channel)
        return;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return;

    task->state.registers.ip = protocol->findentry(descriptor->channel, descriptor->id);
    task->state.registers.sp = sp;

}

unsigned int kernel_setupramdisk(struct container *container, struct task *task, struct vfs_backend *backend)
{

    struct vfs_channel *channel = &container->channels[0x00];
    struct vfs_mount *mount = &container->mounts[0x00];
    struct vfs_descriptor *init = &task->descriptors[0x08];
    struct vfs_descriptor *root = &task->descriptors[0x09];

    channel->backend = backend;
    channel->protocol = vfs_findprotocol(channel->backend);

    if (!channel->protocol)
        return 0;

    mount->parent.channel = channel;
    mount->parent.id = channel->protocol->root(channel->backend);
    mount->child.channel = mount->parent.channel;
    mount->child.id = mount->parent.id;

    if (!mount->parent.id)
        return 0;

    root->channel = mount->parent.channel;
    root->id = mount->parent.id;
    init->channel = root->channel;
    init->id = root->channel->protocol->child(root->channel->backend, root->id, 4, "bin/");

    if (!init->id)
        return 0;

    return init->id = init->channel->protocol->child(init->channel->backend, init->id, 4, "init");

}

void kernel_setup()
{

    resource_setup();
    task_setup();
    binary_setupelf();
    vfs_setupcpio();

}

