#include <fudge.h>
#include "error.h"
#include "resource.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "scheduler.h"
#include "container.h"
#include "kernel.h"

void kernel_setupmodules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules)
{

    struct vfs_channel *channel = &container->channels[0];
    struct vfs_mount *mount = &container->mounts[0];
    struct vfs_descriptor *root = &task->descriptors[0];
    struct vfs_descriptor *work = &task->descriptors[1];
    struct vfs_descriptor *init = &task->descriptors[2];
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct binary_protocol *protocol;

        channel->backend = &modules[i].base;
        channel->protocol = vfs_findprotocol(channel->backend);

        if (!channel->protocol)
            continue;

        mount->parent.channel = channel;
        mount->parent.id = channel->protocol->root(channel->backend);
        mount->child.channel = mount->parent.channel;
        mount->child.id = mount->parent.id;

        if (!mount->parent.id)
            continue;

        root->channel = mount->parent.channel;
        root->id = mount->parent.id;
        work->channel = root->channel;
        work->id = root->id;
        init->channel = work->channel;
        init->id = work->channel->protocol->child(work->channel->backend, work->id, 4, "bin/");

        if (!init->id)
            continue;

        init->id = init->channel->protocol->child(init->channel->backend, init->id, 4, "init");

        if (!init->id)
            continue;

        protocol = binary_findprotocol(init->channel, init->id);

        if (!protocol)
            continue;

        task->state.registers.ip = protocol->copyprogram(init->channel, init->id);

    }

    error_assert(task->state.registers.ip != 0, "Failed to locate entry point", __FILE__, __LINE__);

}

void kernel_setup()
{

    resource_setup();
    scheduler_setup();
    binary_setupelf();
    vfs_setupcpio();
    vfs_setuptar();

}

