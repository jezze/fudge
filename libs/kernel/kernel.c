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

static void setup_container(struct vfs_interface *interface)
{

    runtime_init_container(&container);

    container.running = &task;
    container.mounts[0x01].child.interface = interface;
    container.mounts[0x01].child.id = interface->rootid;

}

static void setup_task(struct vfs_interface *interface, struct binary_format *format)
{

    runtime_init_task(&task, &container);

    task.registers.ip = format->copy_program(interface, interface->walk(interface, interface->rootid, 8, "bin/init"));
    task.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;

    error_assert(task.registers.ip != 0, "Init program entry point not found", __FILE__, __LINE__);

    task.descriptors[0x0E].interface = interface;
    task.descriptors[0x0E].id = interface->rootid;
    task.descriptors[0x0F].interface = interface;
    task.descriptors[0x0F].id = interface->rootid;

}

struct runtime_container *kernel_setup(unsigned int modulesc, void **modulesv)
{

    struct binary_format *elf = binary_elf_setup();
    struct vfs_interface *interface = vfs_tar_setup(modulesv[0]);

    error_assert(interface != 0, "Ramdisk not found", __FILE__, __LINE__);
    binary_setup();
    binary_register_format(elf);
    setup_container(interface);
    setup_task(interface, elf);
    syscall_setup();

    return &container;

}

