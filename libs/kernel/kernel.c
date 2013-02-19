#include <fudge/kernel.h>
#include "error.h"
#include "runtime.h"
#include "kernel.h"
#include "vfs.h"
#include "binary.h"
#include "binary_elf.h"
#include "ramdisk.h"
#include "syscall.h"

static struct runtime_container icontainer;
static struct runtime_task itask;
static struct ramdisk_filesystem iramdisk;

static void setup_ramdisk(int ramdiskc, void **ramdiskv)
{

    unsigned int i;

    ramdisk_init_filesystem(&iramdisk);

    for (i = 0; i < ramdiskc; i++)
        ramdisk_parse(&iramdisk, ramdiskv[i]);

}

static void setup_container()
{

    struct vfs_interface *root = vfs_setup();
    unsigned int rid = root->walk(root, root->rootid, 8, "ramdisk/");

    runtime_init_container(&icontainer);

    icontainer.mounts[1].child.interface = root;
    icontainer.mounts[1].child.id = root->rootid;
    icontainer.mounts[2].parent.interface = root;
    icontainer.mounts[2].parent.id = rid;
    icontainer.mounts[2].child.interface = &iramdisk.interface;
    icontainer.mounts[2].child.id = iramdisk.interface.rootid;

}

static void setup_task(struct binary_format *format)
{

    struct vfs_interface *root = icontainer.mounts[1].child.interface;
    struct vfs_interface *ramdisk = icontainer.mounts[2].child.interface;
    unsigned int rid = root->walk(root, root->rootid, 8, "ramdisk/");
    unsigned int iid = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");

    error_assert(rid != 0, "Ramdisk mountpoint not found", __FILE__, __LINE__);
    error_assert(iid != 0, "Init program not found", __FILE__, __LINE__);
    runtime_init_task(&itask, &icontainer);

    itask.registers.ip = format->copy_program(ramdisk, iid);
    itask.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    itask.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;

    error_assert(itask.registers.ip != 0, "Init program entry point not found", __FILE__, __LINE__);

    itask.descriptors[8].interface = root;
    itask.descriptors[8].id = root->rootid;
    itask.descriptors[9].interface = root;
    itask.descriptors[9].id = root->rootid;

}

struct runtime_task *kernel_setup(unsigned int ramdiskc, void **ramdiskv)
{

    struct binary_format *elf = binary_elf_setup();

    binary_setup();
    binary_register_format(elf);
    syscall_setup();
    setup_ramdisk(ramdiskc, ramdiskv);
    setup_container();
    setup_task(elf);

    return &itask;

}

