#include <memory.h>
#include <error.h>
#include <kernel.h>
#include <vfs.h>
#include <binary.h>
#include <ramdisk.h>
#include <runtime.h>

static struct ramdisk_image ramdiskImage;
static struct ramdisk_filesystem ramdiskFilesystem;

static void start(struct kernel_interface *self)
{

    struct runtime_task *task;
    struct runtime_mount *mount;
    struct runtime_descriptor *descriptor;
    unsigned int i;

    self->setup(self);

    runtime_setup();

    ramdisk_image_init(&ramdiskImage);

    for (i = 0; i < self->ramdiskc; i++)
    {

        unsigned int count = ramdiskImage.parse(&ramdiskImage, *(self->ramdiskv + i));

        error_assert(count != 0, "Ramdisk error", __FILE__, __LINE__);

    }

    ramdisk_filesystem_init(&ramdiskFilesystem, &ramdiskImage);

    task = runtime_get_task(1);
    runtime_task_init(task, 1);

    mount = runtime_get_task_mount(task, 1);
    runtime_mount_init(mount, &ramdiskFilesystem.interface, 9, "/ramdisk/");

    descriptor = runtime_get_task_descriptor(task, 1);
    runtime_descriptor_init(descriptor, mount->interface->walk(mount->interface, mount->interface->rootid, 8, "bin/init"), mount);

    task->status.used = 1;
    task->registers.ip = binary_get_entry(mount->interface, descriptor->id);
    task->registers.sp = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;
    task->registers.sb = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;

    binary_copy_program(mount->interface, descriptor->id);

    self->enter_usermode(task->registers.ip, task->registers.sp);

}

void kernel_interface_init(struct kernel_interface *interface, void (*setup)(struct kernel_interface *self), void (*enter_usermode)(unsigned int ip, unsigned int sp), unsigned int ramdiskc, void **ramdiskv)
{

    memory_clear(interface, sizeof (struct kernel_interface));

    interface->setup = setup;
    interface->enter_usermode = enter_usermode;
    interface->ramdiskc = ramdiskc;
    interface->ramdiskv = ramdiskv;
    interface->start = start;

}

