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

    id = channel->protocol->walk(channel->backend, id, 8, "bin/init");

    if (!id)
        return 0;

    protocol = binary_find_protocol(channel, id);

    if (!protocol)
        return 0;

    return protocol->copy_program(channel, id);

}

void kernel_setup_modules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules)
{

    struct vfs_channel *channel1 = &container->channels[0x01];
    struct vfs_channel *channel2 = &container->channels[0x02];
    struct vfs_descriptor *descriptor = &task->descriptors[0x01];
    struct vfs_mount *mount1 = &container->mounts[0x01];
    struct vfs_mount *mount4 = &container->mounts[0x04];
    unsigned int entry;
    unsigned int i;

    channel1->backend = &state.vfs.backends[0];
    channel1->protocol = &state.vfs.protocols[0];

    for (i = 0; i < count; i++)
    {

        channel2->backend = &modules[i].base;
        channel2->protocol = vfs_find_protocol(channel2->backend);

        if (!channel2->protocol)
            continue;

        entry = kernel_find_init(channel2);

        if (entry)
            break;

    }

    task->registers.ip = entry;

    error_assert(task->registers.ip != 0, "Failed to locate entry point", __FILE__, __LINE__);

    descriptor->channel = channel2;
    descriptor->id = channel2->protocol->root(channel2->backend);
    mount1->parent.channel = channel2;
    mount1->parent.id = channel2->protocol->root(channel2->backend);
    mount1->child.channel = channel2;
    mount1->child.id = channel2->protocol->root(channel2->backend);
    mount4->parent.channel = channel2;
    mount4->parent.id = channel2->protocol->walk(channel2->backend, channel2->protocol->root(channel2->backend), 7, "kernel/");
    mount4->child.channel = channel1;
    mount4->child.id = channel1->protocol->root(channel1->backend);

}

void kernel_setup()
{

    scheduler_init();
    rendezvous_init();
    vfs_setup();
    vfs_init_kernel(&state.vfs.backends[0], &state.vfs.protocols[0]);
    vfs_init_cpio(&state.vfs.protocols[1]);
    vfs_init_tar(&state.vfs.protocols[2]);
    binary_setup();
    binary_init_elf(&state.binary.protocols[0]);
    resource_register_item(&state.vfs.backends[0].resource, RESOURCE_TYPE_VFSBACKEND);
    resource_register_item(&state.vfs.protocols[0].resource, RESOURCE_TYPE_VFSPROTOCOL);
    resource_register_item(&state.vfs.protocols[1].resource, RESOURCE_TYPE_VFSPROTOCOL);
    resource_register_item(&state.vfs.protocols[2].resource, RESOURCE_TYPE_VFSPROTOCOL);
    resource_register_item(&state.binary.protocols[0].resource, RESOURCE_TYPE_BINARYPROTOCOL);

}

