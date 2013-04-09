#include <fudge/kernel.h>
#include "error.h"
#include "runtime.h"
#include "kernel.h"
#include "vfs.h"
#include "vfs_ramdisk.h"
#include "binary.h"
#include "binary_elf.h"
#include "syscall.h"

static struct runtime_container container;
static struct runtime_task task;

static void setup_container(struct vfs_interface *ramdisk)
{

    runtime_init_container(&container);

    container.running = &task;
    container.mounts[1].child.interface = ramdisk;
    container.mounts[1].child.id = ramdisk->rootid;

}

static void setup_task(struct vfs_interface *ramdisk, struct binary_format *format)
{

    runtime_init_task(&task, &container);

    task.registers.ip = format->copy_program(ramdisk, ramdisk->walk(ramdisk, ramdisk->rootid, 8, "bin/init"));
    task.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;

    error_assert(task.registers.ip != 0, "Init program entry point not found", __FILE__, __LINE__);

    task.descriptors[8].interface = ramdisk;
    task.descriptors[8].id = ramdisk->rootid;
    task.descriptors[9].interface = ramdisk;
    task.descriptors[9].id = ramdisk->rootid;

}

struct runtime_container *kernel_setup(unsigned int modulesc, void **modulesv)
{

    struct binary_format *elf = binary_elf_setup();
    struct vfs_interface *ramdisk = vfs_ramdisk_setup(modulesv[0]);

    error_assert(ramdisk != 0, "Ramdisk not found", __FILE__, __LINE__);
    binary_setup();
    binary_register_format(elf);
    setup_container(ramdisk);
    setup_task(ramdisk, elf);
    syscall_setup();

    return &container;

}

