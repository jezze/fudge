#include <fudge.h>
#include "error.h"
#include "resource.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "scheduler.h"
#include "container.h"
#include "kernel.h"

static unsigned int findentry(struct vfs_channel *channel, unsigned int id)
{

    struct binary_protocol *protocol = binary_findprotocol(channel, id);

    if (!protocol)
        return 0;

    return protocol->copyprogram(channel, id);

}

void kernel_setupmodules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules)
{

    struct vfs_channel *channel = &container->channels[0];
    struct vfs_mount *mount = &container->mounts[0];
    struct vfs_descriptor *descriptor = &task->descriptors[0];
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int root;
        unsigned int init;

        channel->backend = &modules[i].base;
        channel->protocol = vfs_findprotocol(channel->backend);

        if (!channel->protocol)
            continue;

        root = channel->protocol->root(channel->backend);

        if (!root)
            continue;

        init = channel->protocol->child(channel->backend, root, 4, "bin/");

        if (!init)
            continue;

        init = channel->protocol->child(channel->backend, init, 4, "init");

        if (!init)
            continue;

        mount->parent.channel = channel;
        mount->parent.id = root;
        mount->child.channel = channel;
        mount->child.id = root;
        descriptor->channel = channel;
        descriptor->id = root;
        task->registers.ip = findentry(channel, init);

    }

    error_assert(task->registers.ip != 0, "Failed to locate entry point", __FILE__, __LINE__);

}

void kernel_setup()
{

    resource_setup();
    scheduler_setup();
    binary_setupelf();
    vfs_setupcpio();
    vfs_setuptar();

}

