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
#define KERNEL_VFS_BACKENDS             1
#define KERNEL_VFS_PROTOCOLS            3

static struct
{

    struct {struct binary_protocol protocols[KERNEL_BINARY_PROTOCOLS];} binary;
    struct {struct vfs_backend backends[KERNEL_VFS_BACKENDS]; struct vfs_protocol protocols[KERNEL_VFS_PROTOCOLS];} vfs;

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

    struct vfs_channel *channel1 = &container->channels[1];
    struct vfs_channel *channel2 = &container->channels[2];
    struct vfs_descriptor *descriptor = &task->descriptors[1];
    struct vfs_mount *mount1 = &container->mounts[1];
    struct vfs_mount *mount4 = &container->mounts[4];
    unsigned int entry = 0;
    unsigned int i;

    channel2->backend = vfs_find_backend();
    channel2->protocol = vfs_find_protocol(channel2->backend);

    for (i = 0; i < count; i++)
    {

        channel1->backend = &modules[i].base;
        channel1->protocol = vfs_find_protocol(channel1->backend);

        if (!channel1->protocol)
            continue;

        entry = kernel_find_init(channel1);

        if (entry)
            break;

    }

    task->registers.ip = entry;

    error_assert(task->registers.ip != 0, "Failed to locate entry point", __FILE__, __LINE__);

    vfs_init_descriptor(descriptor, channel1, channel1->protocol->root(channel1->backend));
    vfs_init_mount(mount1, channel1, channel1->protocol->root(channel1->backend), channel1, channel1->protocol->root(channel1->backend));
    vfs_init_mount(mount4, channel1, channel1->protocol->root(channel1->backend), channel2, channel2->protocol->root(channel2->backend));

    mount4->parent.id = channel1->protocol->child(channel1->backend, mount4->parent.id, 7, "kernel/");

}

void kernel_setup()
{

    resource_setup();
    scheduler_setup();
    rendezvous_setup();
    binary_setup_elf(&state.binary.protocols[0]);
    vfs_setup_kernel(&state.vfs.backends[0], &state.vfs.protocols[0]);
    vfs_setup_cpio(&state.vfs.protocols[1]);
    vfs_setup_tar(&state.vfs.protocols[2]);

}

