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

void kernel_setup_modules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules)
{

    struct vfs_channel *channel = &container->channels[0x01];
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct binary_protocol *protocol;
        unsigned int id;

        channel->backend = &modules[i].base;
        channel->protocol = vfs_find_protocol(channel->backend);

        if (!channel->protocol)
            continue;

        id = channel->protocol->root(channel->backend);

        if (!id)
            continue;

        id = channel->protocol->walk(channel->backend, id, 8, "bin/init");

        if (!id)
            continue;

        protocol = binary_find_protocol(channel, id);

        if (!protocol)
            continue;

        container->mounts[0x01].parent.channel = channel;
        container->mounts[0x01].parent.id = channel->protocol->root(channel->backend);
        container->mounts[0x01].child.channel = channel;
        container->mounts[0x01].child.id = channel->protocol->root(channel->backend);
        task->descriptors[0x01].channel = channel;
        task->descriptors[0x01].id = channel->protocol->root(channel->backend);
        task->registers.ip = protocol->copy_program(channel, id);

        error_assert(task->registers.ip != 0, "Failed to locate entry point", __FILE__, __LINE__);

        container->mounts[0x04].parent.channel = channel;
        container->mounts[0x04].parent.id = channel->protocol->walk(channel->backend, channel->protocol->root(channel->backend), 7, "kernel/");
        container->mounts[0x04].child.channel = &container->channels[0x02];
        container->mounts[0x04].child.channel->backend = &state.vfs.backends[0];
        container->mounts[0x04].child.channel->protocol = &state.vfs.protocols[0];
        container->mounts[0x04].child.id = container->mounts[0x04].child.channel->protocol->root(channel->backend);

        return;

    }

    error_panic("Failed to locate init", __FILE__, __LINE__);

}

void kernel_setup()
{

    scheduler_init();
    rendezvous_init();
    vfs_setup();
    vfs_init_kernel(&state.vfs.backends[0], &state.vfs.protocols[0]);
    vfs_register_backend(&state.vfs.backends[0]);
    vfs_register_protocol(&state.vfs.protocols[0]);
    vfs_init_cpio(&state.vfs.protocols[1]);
    vfs_register_protocol(&state.vfs.protocols[1]);
    vfs_init_tar(&state.vfs.protocols[2]);
    vfs_register_protocol(&state.vfs.protocols[2]);
    binary_setup();
    binary_init_elf(&state.binary.protocols[0]);
    binary_register_protocol(&state.binary.protocols[0]);

}

