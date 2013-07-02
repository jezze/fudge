#include <fudge/kernel.h>
#include "error.h"
#include "vfs.h"
#include "vfs_cpio.h"
#include "vfs_module.h"
#include "vfs_tar.h"
#include "binary.h"
#include "binary_elf.h"
#include "runtime.h"
#include "syscall.h"
#include "kernel.h"

static struct runtime_container container;
static struct runtime_task task;

static void setup(struct vfs_session *session, unsigned int ip)
{

    runtime_init_container(&container);

    container.running = &task;

    container.mounts[0x01].child.session.backend = session->backend;
    container.mounts[0x01].child.session.protocol = session->protocol;
    container.mounts[0x01].child.id = session->protocol->rootid;

    runtime_init_task(&task, &container);

    task.registers.ip = ip;
    task.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.descriptors[0x0E].session.backend = session->backend;
    task.descriptors[0x0E].session.protocol = session->protocol;
    task.descriptors[0x0E].id = session->protocol->rootid;
    task.descriptors[0x0F].session.backend = session->backend;
    task.descriptors[0x0F].session.protocol = session->protocol;
    task.descriptors[0x0F].id = session->protocol->rootid;

}

static void detect(struct vfs_backend *backend)
{

    struct vfs_session session;
    struct binary_protocol *protocol;
    unsigned int id;
    unsigned int ip;

    session.backend = backend;
    session.protocol = vfs_get_protocol(session.backend);

    error_assert(session.protocol != 0, "Filesystem protocol not recognized", __FILE__, __LINE__);

    id = session.protocol->walk(session.backend, session.protocol->rootid, 8, "bin/init");

    error_assert(id != 0, "Init program not found", __FILE__, __LINE__);

    protocol = binary_get_protocol(&session, id);

    error_assert(protocol != 0, "Binary protocol not recognized", __FILE__, __LINE__);

    ip = protocol->copy_program(&session, id);

    error_assert(ip != 0, "Init entry not found", __FILE__, __LINE__);

    setup(&session, ip);

}

struct runtime_container *kernel_setup(unsigned int count, struct kernel_module *modules)
{

    struct vfs_protocol *cpio = vfs_cpio_setup((unsigned int)modules[0].base);
    struct vfs_protocol *tar = vfs_tar_setup((unsigned int)modules[0].base);
    struct vfs_backend *module = vfs_module_setup(count, modules);
    struct binary_protocol *elf = binary_elf_setup();

    vfs_setup();
    vfs_register_protocol(cpio);
    vfs_register_protocol(tar);
    binary_setup();
    binary_register_protocol(elf);
    syscall_setup();
    detect(module);

    return &container;

}

