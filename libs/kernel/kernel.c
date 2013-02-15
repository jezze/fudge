#include <fudge/kernel.h>
#include "error.h"
#include "runtime.h"
#include "kernel.h"
#include "vfs.h"
#include "binary.h"
#include "binary_elf.h"
#include "ramdisk.h"
#include "syscall.h"

static struct runtime_task task;
static struct ramdisk_filesystem ramdisk;

static void setup_ramdisk(int ramdiskc, void **ramdiskv)
{

    unsigned int i;

    ramdisk_init_filesystem(&ramdisk);

    for (i = 0; i < ramdiskc; i++)
        ramdisk_parse(&ramdisk, ramdiskv[i]);

}

static void setup_task(struct binary_format *format)
{

    struct vfs_interface *root = vfs_setup();
    unsigned int rid = root->walk(root, root->rootid, 8, "ramdisk/");
    unsigned int iid = ramdisk.interface.walk(&ramdisk.interface, ramdisk.interface.rootid, 9, "bin/inits");

    runtime_init_task(&task);
    error_assert(rid != 0, "Ramdisk mountpoint not found", __FILE__, __LINE__);
    error_assert(iid != 0, "Init program not found", __FILE__, __LINE__);

    task.registers.ip = format->copy_program(&ramdisk.interface, iid);
    task.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;

    error_assert(task.registers.ip != 0, "Init program entry point not found", __FILE__, __LINE__);

    task.mounts[1].child.interface = root;
    task.mounts[1].child.id = root->rootid;
    task.mounts[2].parent.interface = root;
    task.mounts[2].parent.id = rid;
    task.mounts[2].child.interface = &ramdisk.interface;
    task.mounts[2].child.id = ramdisk.interface.rootid;
    task.descriptors[8].interface = root;
    task.descriptors[8].id = root->rootid;
    task.descriptors[9].interface = root;
    task.descriptors[9].id = root->rootid;

}

struct runtime_task *kernel_setup(unsigned int ramdiskc, void **ramdiskv)
{

    struct binary_format *elf = binary_elf_setup();

    binary_setup();
    binary_register_format(elf);
    syscall_setup();
    setup_ramdisk(ramdiskc, ramdiskv);
    setup_task(elf);

    return &task;

}

