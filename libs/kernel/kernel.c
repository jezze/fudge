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
static struct ramdisk_filesystem ramdiskFilesystem;

static struct vfs_interface *setup_ramdisk(int ramdiskc, void **ramdiskv)
{

    unsigned int i;

    ramdisk_init_filesystem(&ramdiskFilesystem);

    for (i = 0; i < ramdiskc; i++)
        ramdisk_parse(&ramdiskFilesystem, ramdiskv[i]);

    return &ramdiskFilesystem.interface;

}

struct runtime_task *kernel_setup(unsigned int ramdiskc, void **ramdiskv)
{

    struct vfs_interface *root = vfs_setup();
    struct vfs_interface *ramdisk = setup_ramdisk(ramdiskc, ramdiskv);
    struct binary_format *elf = binary_elf_setup();
    unsigned int id;
    unsigned int entry;

    binary_setup();
    binary_register_format(elf);
    syscall_setup();
    runtime_init_task(&task);

    error_assert(ramdisk != 0, "Ramdisk not found", __FILE__, __LINE__);

    id = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");

    error_assert(id != 0, "Init program not found", __FILE__, __LINE__);

    entry = elf->copy_program(ramdisk, id);

    error_assert(entry != 0, "Init program entry point not found", __FILE__, __LINE__);

    task.registers.ip = entry;
    task.registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;
    task.registers.status = 0;
    task.mounts[1].parent.interface = 0;
    task.mounts[1].parent.id = 0;
    task.mounts[1].child.interface = root;
    task.mounts[1].child.id = root->rootid;
    task.mounts[2].parent.interface = root;
    task.mounts[2].parent.id = root->walk(root, root->rootid, 8, "ramdisk/");
    task.mounts[2].child.interface = ramdisk;
    task.mounts[2].child.id = ramdisk->rootid;
    task.descriptors[8].interface = root;
    task.descriptors[8].id = root->rootid;
    task.descriptors[9].interface = root;
    task.descriptors[9].id = root->rootid;

    return &task;

}

