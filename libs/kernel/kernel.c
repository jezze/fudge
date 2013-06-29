#include <fudge/kernel.h>
#include "error.h"
#include "vfs.h"
#include "vfs_module.h"
#include "vfs_tar.h"
#include "binary.h"
#include "binary_elf.h"
#include "runtime.h"
#include "syscall.h"
#include "kernel.h"

static struct runtime_container container;
static struct runtime_task task;

static void setup(struct vfs_backend *backend, struct vfs_protocol *protocol, unsigned int ip)
{

    runtime_init_container(&container);

    container.running = &task;
    container.mounts[0x01].child.backend = backend;
    container.mounts[0x01].child.protocol = protocol;
    container.mounts[0x01].child.id = protocol->rootid;

    runtime_init_task(&task, &container);

    task.registers.ip = ip;
    task.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.descriptors[0x0E].backend = backend;
    task.descriptors[0x0E].protocol = protocol;
    task.descriptors[0x0E].id = protocol->rootid;
    task.descriptors[0x0F].backend = backend;
    task.descriptors[0x0F].protocol = protocol;
    task.descriptors[0x0F].id = protocol->rootid;

}

static void detect(struct vfs_backend *vbackend)
{

    struct vfs_protocol *vprotocol;
    struct binary_protocol *bprotocol;
    unsigned int id;
    unsigned int ip;

    vprotocol = vfs_get_protocol(vbackend);

    error_assert(vprotocol != 0, "Filesystem protocol not recognized", __FILE__, __LINE__);

    id = vprotocol->walk(vbackend, vprotocol->rootid, 8, "bin/init");

    error_assert(id != 0, "Init program not found", __FILE__, __LINE__);

    bprotocol = binary_get_protocol(vprotocol, vbackend, id);

    error_assert(bprotocol != 0, "Binary protocol not recognized", __FILE__, __LINE__);

    ip = bprotocol->copy_program(vprotocol, vbackend, id);

    error_assert(ip != 0, "Init entry not found", __FILE__, __LINE__);

    setup(vbackend, vprotocol, ip);

}

struct runtime_container *kernel_setup(unsigned int modulesc, void **modulesv)
{

    struct binary_protocol *elf = binary_elf_setup();
    struct vfs_protocol *tar = vfs_tar_setup(modulesv[0]);
    struct vfs_backend *module = vfs_module_setup(modulesc, modulesv);

    vfs_setup();
    vfs_register_protocol(tar);
    binary_setup();
    binary_register_protocol(elf);
    syscall_setup();
    detect(module);

    return &container;

}

