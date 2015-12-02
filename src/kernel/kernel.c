#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

unsigned int kernel_setupramdisk(struct container *container, struct task *task, struct vfs_backend *backend)
{

    struct vfs_channel *channel = &container->channels[0x00];
    struct vfs_mount *mount = &container->mounts[0x00];
    struct task_descriptor *init = &task->descriptors[0x08];
    struct task_descriptor *root = &task->descriptors[0x09];

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

void kernel_setup(void)
{

    binary_setupelf();
    vfs_setupcpio();

}

