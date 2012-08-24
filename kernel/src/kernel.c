#include <memory.h>
#include <error.h>
#include <kernel.h>
#include <ramdisk.h>
#include <runtime.h>
#include <syscall.h>
#include <vfs.h>
#include <vfs/ramdisk.h>

static struct ramdisk_image ramdiskImage;
static struct vfs_ramdisk_filesystem ramdiskFilesystem;

static void start(struct kernel_arch *self)
{

    struct syscall_execute_args eargs;
    struct runtime_mount *mount;
    struct runtime_task *task;
    unsigned int i;
    unsigned int slot;

    self->setup(self);

    runtime_setup();

    ramdisk_image_init(&ramdiskImage);

    for (i = 0; i < self->ramdiskc; i++)
    {

        unsigned int count = ramdiskImage.parse(&ramdiskImage, *(self->ramdiskv + i));

        error_assert(count != 0, "Ramdisk error", __FILE__, __LINE__);

    }

    vfs_ramdisk_filesystem_init(&ramdiskFilesystem, &ramdiskImage);

    task = runtime_get_task(1);
    runtime_task_init(task, 1);

    mount = task->get_mount(task, 2);
    runtime_mount_init(mount, 2, &ramdiskFilesystem.base, 9, "/ramdisk/");

    runtime_descriptor_init(&task->descriptors[1], mount->filesystem->walk(mount->filesystem, mount->filesystem->rootid, 8, "bin/init"), mount);

    eargs.index = 1;

    slot = syscall_execute(task, &eargs);

    task = runtime_get_task(slot);

    self->enter_usermode(task->registers.ip, task->registers.sp);

}

void kernel_arch_init(struct kernel_arch *arch, void (*setup)(struct kernel_arch *self), void (*halt)(), void (*enable_interrupts)(), void (*disable_interrupts)(), void (*enter_usermode)(unsigned int ip, unsigned int sp), unsigned int ramdiskc, void **ramdiskv)
{

    memory_clear(arch, sizeof (struct kernel_arch));

    arch->setup = setup;
    arch->halt = halt;
    arch->enable_interrupts = enable_interrupts;
    arch->disable_interrupts = disable_interrupts;
    arch->enter_usermode = enter_usermode;
    arch->ramdiskc = ramdiskc;
    arch->ramdiskv = ramdiskv;
    arch->start = start;

}

