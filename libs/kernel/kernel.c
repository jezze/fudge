#include <fudge/kernel.h>
#include "error.h"
#include "runtime.h"
#include "kernel.h"
#include "vfs.h"
#include "vfs_root.h"
#include "vfs_ramdisk.h"
#include "binary.h"
#include "binary_elf.h"
#include "syscall.h"

static struct runtime_container icontainer;
static struct runtime_task itask;

static void setup_container(void *ramdiskaddress)
{

    struct vfs_interface *root = vfs_root_setup();
    struct vfs_interface *ramdisk = vfs_ramdisk_setup(ramdiskaddress);
    unsigned int id = root->walk(root, root->rootid, 8, "ramdisk/");

    error_assert(id != 0, "Ramdisk mountpoint not found", __FILE__, __LINE__);
    runtime_init_container(&icontainer);

    icontainer.running = &itask;
    icontainer.mounts[1].child.interface = root;
    icontainer.mounts[1].child.id = root->rootid;
    icontainer.mounts[2].parent.interface = root;
    icontainer.mounts[2].parent.id = id;
    icontainer.mounts[2].child.interface = ramdisk;
    icontainer.mounts[2].child.id = ramdisk->rootid;

}

static void setup_task(struct binary_format *format)
{

    struct vfs_interface *root = icontainer.mounts[1].child.interface;
    struct vfs_interface *ramdisk = icontainer.mounts[2].child.interface;
    unsigned int id = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");

    error_assert(id != 0, "Init program not found", __FILE__, __LINE__);
    runtime_init_task(&itask, &icontainer);

    itask.registers.ip = format->copy_program(ramdisk, id);
    itask.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    itask.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;

    error_assert(itask.registers.ip != 0, "Init program entry point not found", __FILE__, __LINE__);

    itask.descriptors[8].interface = root;
    itask.descriptors[8].id = root->rootid;
    itask.descriptors[9].interface = root;
    itask.descriptors[9].id = root->rootid;

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

