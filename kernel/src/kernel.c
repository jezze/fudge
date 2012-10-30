#include <memory.h>
#include <error.h>
#include <kernel.h>
#include <vfs.h>
#include <binary.h>
#include <ramdisk.h>
#include <runtime.h>
#include <syscall.h>

static struct runtime_task task;

struct runtime_task *kernel_setup(unsigned int ramdiskc, void **ramdiskv)
{

    struct vfs_interface *ramdisk;
    unsigned int id;
    unsigned int entry;

    syscall_setup();
    ramdisk = ramdisk_setup(ramdiskc, ramdiskv);

    id = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");
    entry = binary_copy_program(ramdisk, id);

    runtime_init_task(&task, entry);
    task.status.used = 1;

    runtime_init_mount(&task.mounts[1], ramdisk, 9, "/ramdisk/");

    return &task;

}

