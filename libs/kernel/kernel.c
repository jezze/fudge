#include <fudge/kernel.h>
#include "error.h"
#include "runtime.h"
#include "kernel.h"
#include "vfs.h"
#include "vfs_ramdisk.h"
#include "binary.h"
#include "binary_elf.h"
#include "syscall.h"

static struct runtime_container icontainer;
static struct runtime_task itask;

static void setup_container(void *ramdiskaddress)
{

    struct vfs_interface *ramdisk = vfs_ramdisk_setup(ramdiskaddress);

    error_assert(ramdisk != 0, "Ramdisk not found", __FILE__, __LINE__);
    runtime_init_container(&icontainer);

    icontainer.running = &itask;
    icontainer.mounts[1].child.interface = ramdisk;
    icontainer.mounts[1].child.id = ramdisk->rootid;

}

static void setup_task(struct binary_format *format)
{

    struct vfs_interface *ramdisk = icontainer.mounts[1].child.interface;
    unsigned int id = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");

    error_assert(id != 0, "Init program not found", __FILE__, __LINE__);
    runtime_init_task(&itask, &icontainer);

    itask.registers.ip = format->copy_program(ramdisk, id);
    itask.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    itask.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;

    error_assert(itask.registers.ip != 0, "Init program entry point not found", __FILE__, __LINE__);

    itask.descriptors[8].interface = ramdisk;
    itask.descriptors[8].id = ramdisk->rootid;
    itask.descriptors[9].interface = ramdisk;
    itask.descriptors[9].id = ramdisk->rootid;

}

struct runtime_container *kernel_setup(unsigned int modulesc, void **modulesv)
{

    struct binary_format *elf = binary_elf_setup();

    binary_setup();
    binary_register_format(elf);
    setup_container(modulesv[0]);
    setup_task(elf);
    syscall_setup();

    return &icontainer;

}

