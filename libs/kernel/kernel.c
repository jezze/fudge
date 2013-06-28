#include <fudge/kernel.h>
#include "error.h"
#include "runtime.h"
#include "kernel.h"
#include "vfs.h"
#include "vfs_tar.h"
#include "binary.h"
#include "binary_elf.h"
#include "syscall.h"

static struct runtime_container container;
static struct runtime_task task;

static void setup(struct vfs_protocol *vprotocol, struct binary_protocol *bprotocol, unsigned int id)
{

    runtime_init_container(&container);

    container.running = &task;
    container.mounts[0x01].child.protocol = vprotocol;
    container.mounts[0x01].child.id = vprotocol->rootid;

    runtime_init_task(&task, &container);

    task.registers.ip = bprotocol->copy_program(vprotocol, 0, id);
    task.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;

    error_assert(task.registers.ip != 0, "Init program entry point not found", __FILE__, __LINE__);

    task.descriptors[0x0E].protocol = vprotocol;
    task.descriptors[0x0E].id = vprotocol->rootid;
    task.descriptors[0x0F].protocol = vprotocol;
    task.descriptors[0x0F].id = vprotocol->rootid;

}

static void detect()
{

    struct vfs_protocol *vprotocol;
    struct binary_protocol *bprotocol;
    unsigned int id;

    vprotocol = vfs_get_protocol(0);

    error_assert(vprotocol != 0, "Filesystem protocol not recognized", __FILE__, __LINE__);

    id = vprotocol->walk(0, vprotocol->rootid, 8, "bin/init");

    error_assert(id != 0, "Init program not found", __FILE__, __LINE__);

    bprotocol = binary_get_protocol(vprotocol, 0, id);

    error_assert(bprotocol != 0, "Binary protocol not recognized", __FILE__, __LINE__);

    setup(vprotocol, bprotocol, id);

}

struct runtime_container *kernel_setup(unsigned int modulesc, void **modulesv)
{

    struct binary_protocol *elf = binary_elf_setup();
    struct vfs_protocol *tar = vfs_tar_setup(modulesv[0]);

    vfs_setup();
    vfs_register_protocol(tar);
    binary_setup();
    binary_register_protocol(elf);
    syscall_setup();
    detect();

    return &container;

}

