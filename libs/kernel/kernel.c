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

static void setup(struct vfs_interface *vinterface, struct binary_interface *binterface, unsigned int id)
{

    runtime_init_container(&container);

    container.running = &task;
    container.mounts[0x01].child.interface = vinterface;
    container.mounts[0x01].child.id = vinterface->rootid;

    runtime_init_task(&task, &container);

    task.registers.ip = binterface->copy_program(vinterface, id);
    task.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;

    error_assert(task.registers.ip != 0, "Init program entry point not found", __FILE__, __LINE__);

    task.descriptors[0x0E].interface = vinterface;
    task.descriptors[0x0E].id = vinterface->rootid;
    task.descriptors[0x0F].interface = vinterface;
    task.descriptors[0x0F].id = vinterface->rootid;

}

static void detect()
{

    struct vfs_interface *vinterface;
    struct binary_interface *binterface;
    unsigned int id;

    vinterface = vfs_get_interface();

    error_assert(vinterface != 0, "Filesystem interface not recognized", __FILE__, __LINE__);

    id = vinterface->walk(vinterface, vinterface->rootid, 8, "bin/init");

    error_assert(id != 0, "Init program not found", __FILE__, __LINE__);

    binterface = binary_get_interface(vinterface, id);

    error_assert(binterface != 0, "Binary interface not recognized", __FILE__, __LINE__);

    setup(vinterface, binterface, id);

}

struct runtime_container *kernel_setup(unsigned int modulesc, void **modulesv)
{

    struct binary_interface *elf = binary_elf_setup();
    struct vfs_interface *tar = vfs_tar_setup(modulesv[0]);

    vfs_setup();
    vfs_register_interface(tar);
    binary_setup();
    binary_register_interface(elf);
    syscall_setup();
    detect();

    return &container;

}

