#include <fudge/kernel.h>
#include "error.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

#define KERNEL_BINARY_PROTOCOLS         1
#define KERNEL_VFS_PROTOCOLS            2

static struct
{

    struct task task;
    struct container container;
    struct {struct binary_protocol protocols[KERNEL_BINARY_PROTOCOLS];} binary;
    struct {struct vfs_protocol protocols[KERNEL_VFS_PROTOCOLS];} vfs;

} state;

struct container *kernel_setup(unsigned int count, struct kernel_module *modules)
{

    unsigned int i;

    vfs_setup();
    vfs_init_cpio(&state.vfs.protocols[0]);
    vfs_register_protocol(&state.vfs.protocols[0]);
    vfs_init_tar(&state.vfs.protocols[1]);
    vfs_register_protocol(&state.vfs.protocols[1]);
    binary_setup();
    binary_init_elf(&state.binary.protocols[0]);
    binary_register_protocol(&state.binary.protocols[0]);

    for (i = 0; i < count; i++)
    {

        struct vfs_session session;
        struct binary_protocol *protocol;
        unsigned int id;
        unsigned int ip;

        session.backend = &modules[i].base;
        session.protocol = vfs_get_protocol(session.backend);

        if (!session.protocol)
            continue;

        id = session.protocol->walk(session.backend, session.protocol->rootid, 8, "bin/init");

        if (!id)
            continue;

        protocol = binary_get_protocol(&session, id);

        if (!protocol)
            continue;

        ip = protocol->copy_program(&session, id);

        if (!ip)
            continue;

        task_init(&state.task, ip, STACKADDRESS_VIRTUAL, STACKADDRESS_VIRTUAL);

        state.task.descriptors[0x0E].session.backend = session.backend;
        state.task.descriptors[0x0E].session.protocol = session.protocol;
        state.task.descriptors[0x0E].id = session.protocol->rootid;
        state.task.descriptors[0x0F].session.backend = session.backend;
        state.task.descriptors[0x0F].session.protocol = session.protocol;
        state.task.descriptors[0x0F].id = session.protocol->rootid;

        container_init(&state.container, &state.task);

        state.container.mounts[0x01].parent.session.backend = session.backend;
        state.container.mounts[0x01].parent.session.protocol = session.protocol;
        state.container.mounts[0x01].parent.id = session.protocol->rootid;
        state.container.mounts[0x01].child.session.backend = session.backend;
        state.container.mounts[0x01].child.session.protocol = session.protocol;
        state.container.mounts[0x01].child.id = session.protocol->rootid;

        return &state.container;

    }

    error_panic("Failed to create container", __FILE__, __LINE__);

    return 0;

}

