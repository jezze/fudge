#include <runtime.h>
#include <kernel.h>
#include <vfs.h>
#include <binary.h>
#include <ramdisk.h>
#include <syscall.h>

void kernel_setup(struct runtime_task *task, unsigned int ramdiskc, void **ramdiskv)
{

    struct vfs_interface *root = vfs_setup();
    struct vfs_interface *ramdisk = ramdisk_setup(ramdiskc, ramdiskv);
    unsigned int id = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");
    unsigned int entry = binary_copy_program(ramdisk, id);

    syscall_setup();

    runtime_init_registers(&task->registers, entry, RUNTIME_STACK_VADDRESS_BASE, RUNTIME_STACK_VADDRESS_BASE, 0);

    runtime_init_mount(&task->mounts[1], root, root->rootid, root, root->rootid);
    runtime_init_mount(&task->mounts[2], root, root->walk(root, root->rootid, 8, "ramdisk/"), ramdisk, ramdisk->rootid);

}

