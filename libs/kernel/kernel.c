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

    struct container container;
    struct {struct binary_protocol protocols[KERNEL_BINARY_PROTOCOLS];} binary;
    struct {struct vfs_protocol protocols[KERNEL_VFS_PROTOCOLS];} vfs;

} state;

static void setup_container(struct container *container, struct vfs_session *session)
{

    container->mounts[0x01].parent.session.backend = session->backend;
    container->mounts[0x01].parent.session.protocol = session->protocol;
    container->mounts[0x01].parent.id = session->protocol->rootid;
    container->mounts[0x01].child.session.backend = session->backend;
    container->mounts[0x01].child.session.protocol = session->protocol;
    container->mounts[0x01].child.id = session->protocol->rootid;

}

static void setup_task(struct task *task, struct vfs_session *session, struct binary_protocol *protocol, unsigned int id)
{

    task->registers.ip = protocol->copy_program(session, id);

    error_assert(task->registers.ip != 0, "Failed to locate entry point", __FILE__, __LINE__);

    task->descriptors[0x0E].session.backend = session->backend;
    task->descriptors[0x0E].session.protocol = session->protocol;
    task->descriptors[0x0E].id = session->protocol->rootid;
    task->descriptors[0x0F].session.backend = session->backend;
    task->descriptors[0x0F].session.protocol = session->protocol;
    task->descriptors[0x0F].id = session->protocol->rootid;

}

void kernel_setup_modules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct vfs_session session;
        struct binary_protocol *protocol;
        unsigned int id;

        session.backend = &modules[i].base;
        session.protocol = vfs_find_protocol(session.backend);

        if (!session.protocol)
            continue;

        id = session.protocol->walk(session.backend, session.protocol->rootid, 8, "bin/init");

        if (!id)
            continue;

        protocol = binary_find_protocol(&session, id);

        if (!protocol)
            continue;

        setup_container(container, &session);
        setup_task(task, &session, protocol, id);

        return;

    }

    error_panic("Failed to locate init", __FILE__, __LINE__);

}

struct container *kernel_setup()
{

    vfs_setup();
    vfs_init_cpio(&state.vfs.protocols[0]);
    vfs_register_protocol(&state.vfs.protocols[0]);
    vfs_init_tar(&state.vfs.protocols[1]);
    vfs_register_protocol(&state.vfs.protocols[1]);
    binary_setup();
    binary_init_elf(&state.binary.protocols[0]);
    binary_register_protocol(&state.binary.protocols[0]);
    container_init(&state.container);

    return &state.container;

}

