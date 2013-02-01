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

    error_assert(ramdisk != 0, "Ramdisk not found", __FILE__, __LINE__);

    id = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");

    error_assert(id != 0, "Init program not found", __FILE__, __LINE__);

    entry = binary_copy_program(ramdisk, id);

    error_assert(entry != 0, "Init program entry point not found", __FILE__, __LINE__);

    runtime_init_task(&task);
    runtime_set_task_registers(&task, entry, RUNTIME_STACKADDRESS_VIRTUAL, RUNTIME_STACKADDRESS_VIRTUAL, 0);
    runtime_set_task_mount(&task, 1, 0, 0, root, root->rootid);
    runtime_set_task_mount(&task, 2, root, root->walk(root, root->rootid, 8, "ramdisk/"), ramdisk, ramdisk->rootid);
    runtime_set_task_descriptor(&task, 8, root, root->rootid);
    runtime_set_task_descriptor(&task, 9, root, root->rootid);

    return &task;

}

