#include <fudge/kernel.h>
#include "runtime.h"
#include "kernel.h"
#include "vfs.h"
#include "binary.h"
#include "ramdisk.h"
#include "syscall.h"

static struct ramdisk_filesystem ramdiskFilesystem;

static struct vfs_interface *setup_ramdisk(int ramdiskc, void **ramdiskv)
{

    unsigned int i;

    ramdisk_init_filesystem(&ramdiskFilesystem);

    for (i = 0; i < ramdiskc; i++)
        ramdisk_parse(&ramdiskFilesystem, *(ramdiskv + i));

    return &ramdiskFilesystem.interface;

}

void kernel_setup(struct runtime_task *task, unsigned int ramdiskc, void **ramdiskv)
{

    struct vfs_interface *root = vfs_setup();
    struct vfs_interface *ramdisk = setup_ramdisk(ramdiskc, ramdiskv);
    unsigned int id = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");
    unsigned int entry = binary_copy_program(ramdisk, id);

    syscall_setup();
    runtime_init_registers(&task->registers, entry, RUNTIME_STACKADDRESS_VIRTUAL, RUNTIME_STACKADDRESS_VIRTUAL, 0);
    runtime_init_mount(&task->mounts[1], 0, 0, root, root->rootid);
    runtime_init_mount(&task->mounts[2], root, root->walk(root, root->rootid, 8, "ramdisk/"), ramdisk, ramdisk->rootid);
    runtime_init_descriptor(&task->descriptors[8], root, root->rootid);
    runtime_init_descriptor(&task->descriptors[9], root, root->rootid);

}

