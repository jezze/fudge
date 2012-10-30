#include <runtime.h>
#include <kernel.h>
#include <vfs.h>
#include <binary.h>
#include <ramdisk.h>
#include <syscall.h>

void kernel_setup(struct runtime_task *task, unsigned int ramdiskc, void **ramdiskv)
{

    struct vfs_interface *ramdisk;
    unsigned int id;
    unsigned int entry;

    syscall_setup();
    ramdisk = ramdisk_setup(ramdiskc, ramdiskv);

    id = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");
    entry = binary_copy_program(ramdisk, id);

    runtime_init_registers(&task->registers, entry, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, 0);
    task->status.used = 1;

    runtime_init_mount(&task->mounts[1], ramdisk, 9, "/ramdisk/");

}

