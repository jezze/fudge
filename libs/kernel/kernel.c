#include <kernel.h>
#include "error.h"
#include "resource.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "scheduler.h"
#include "rendezvous.h"
#include "container.h"
#include "kernel.h"

#define KERNEL_BINARY_PROTOCOLS         1
#define KERNEL_VFS_PROTOCOLS            2

static struct
{

    struct {struct binary_protocol protocols[KERNEL_BINARY_PROTOCOLS];} binary;
    struct {struct vfs_protocol protocols[KERNEL_VFS_PROTOCOLS];} vfs;

} state;

static unsigned int kernel_find_init(struct vfs_channel *channel)
{

    unsigned int id = channel->protocol->root(channel->backend);
    struct binary_protocol *protocol;

    if (!id)
        return 0;

    id = channel->protocol->child(channel->backend, id, 4, "bin/");

    if (!id)
        return 0;

    id = channel->protocol->child(channel->backend, id, 4, "init");

    if (!id)
        return 0;

    protocol = binary_find_protocol(channel, id);

    if (!protocol)
        return 0;

    return protocol->copy_program(channel, id);

}

void kernel_setup_modules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules)
{

    struct vfs_channel *channel = &container->channels[1];
    struct vfs_descriptor *descriptor = &task->descriptors[1];
    struct vfs_mount *mount = &container->mounts[1];
    unsigned int entry = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        channel->backend = &modules[i].base;
        channel->protocol = vfs_find_protocol(channel->backend);

        if (!channel->protocol)
            continue;

        entry = kernel_find_init(channel);

        if (entry)
            break;

    }

    task->registers.ip = entry;

    error_assert(task->registers.ip != 0, "Failed to locate entry point", __FILE__, __LINE__);

    vfs_init_descriptor(descriptor, channel, channel->protocol->root(channel->backend));
    vfs_init_mount(mount, channel, channel->protocol->root(channel->backend), channel, channel->protocol->root(channel->backend));

}

void kernel_setup()
{

    resource_setup();
    scheduler_setup();
    binary_setup_elf(&state.binary.protocols[0]);
    vfs_setup_cpio(&state.vfs.protocols[0]);
    vfs_setup_tar(&state.vfs.protocols[1]);

}

