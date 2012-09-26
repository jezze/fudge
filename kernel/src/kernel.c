#include <memory.h>
#include <error.h>
#include <event.h>
#include <kernel.h>
#include <vfs.h>
#include <binary.h>
#include <ramdisk.h>
#include <runtime.h>
#include <syscall.h>

static struct runtime_task task;

void kernel_register_interface(struct kernel_interface *interface)
{

    struct vfs_interface *ramdisk;
    unsigned int id;
    unsigned int entry;

    interface->setup(interface);

    syscall_setup();
    event_setup();
    ramdisk = ramdisk_setup(interface->ramdiskc, interface->ramdiskv);

    id = ramdisk->walk(ramdisk, ramdisk->rootid, 9, "bin/inits");
    entry = binary_copy_program(ramdisk, id);

    runtime_init_task(&task, 1, entry);
    task.status.used = 1;

    runtime_init_mount(&task.mounts[1], ramdisk, 9, "/ramdisk/");
    runtime_set_task(&task);

    interface->enter_usermode(task.registers.ip, task.registers.sp);

}

void kernel_init_interface(struct kernel_interface *interface, void (*setup)(struct kernel_interface *self), void (*enter_usermode)(unsigned int ip, unsigned int sp), unsigned int ramdiskc, void **ramdiskv)
{

    memory_clear(interface, sizeof (struct kernel_interface));

    interface->setup = setup;
    interface->enter_usermode = enter_usermode;
    interface->ramdiskc = ramdiskc;
    interface->ramdiskv = ramdiskv;

}

