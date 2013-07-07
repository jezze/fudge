#include <fudge/kernel.h>
#include "error.h"
#include "vfs.h"
#include "binary.h"
#include "runtime.h"
#include "syscall.h"
#include "kernel.h"

static struct runtime_task task;
static struct runtime_container container;

static void setup_task(struct vfs_session *session, unsigned int ip)
{

    runtime_init_task(&task, ip, RUNTIME_STACKADDRESS_VIRTUAL, RUNTIME_STACKADDRESS_VIRTUAL);

    task.descriptors[0x0E].session.backend = session->backend;
    task.descriptors[0x0E].session.protocol = session->protocol;
    task.descriptors[0x0E].id = session->protocol->rootid;
    task.descriptors[0x0F].session.backend = session->backend;
    task.descriptors[0x0F].session.protocol = session->protocol;
    task.descriptors[0x0F].id = session->protocol->rootid;

}

static void setup_container(struct vfs_session *session)
{

    runtime_init_container(&container, &task);

    container.mounts[0x01].parent.session.backend = session->backend;
    container.mounts[0x01].parent.session.protocol = session->protocol;
    container.mounts[0x01].parent.id = session->protocol->rootid;
    container.mounts[0x01].child.session.backend = session->backend;
    container.mounts[0x01].child.session.protocol = session->protocol;
    container.mounts[0x01].child.id = session->protocol->rootid;

}

struct runtime_container *kernel_setup(unsigned int count, struct kernel_module *modules)
{

    struct vfs_protocol *cpio = vfs_cpio_setup();
    struct vfs_protocol *tar = vfs_tar_setup();
    struct binary_protocol *elf = binary_elf_setup();
    unsigned int i;

    vfs_setup();
    vfs_register_protocol(cpio);
    vfs_register_protocol(tar);
    binary_setup();
    binary_register_protocol(elf);

    for (i = 0; i < count; i++)
    {

        struct vfs_session session;
        struct binary_protocol *protocol;
        unsigned int id;
        unsigned int ip;

        session.backend = &modules[i].base;
        session.protocol = vfs_get_protocol(session.backend);

        error_assert(session.protocol != 0, "Filesystem protocol not recognized", __FILE__, __LINE__);

        id = session.protocol->walk(session.backend, session.protocol->rootid, 8, "bin/init");

        error_assert(id != 0, "Init program not found", __FILE__, __LINE__);

        protocol = binary_get_protocol(&session, id);

        error_assert(protocol != 0, "Binary protocol not recognized", __FILE__, __LINE__);

        ip = protocol->copy_program(&session, id);

        error_assert(ip != 0, "Init entry not found", __FILE__, __LINE__);

        setup_task(&session, ip);
        setup_container(&session);

    }

    syscall_setup(&container);

    return &container;

}

