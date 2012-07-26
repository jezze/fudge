#include <memory.h>
#include <error.h>
#include <kernel.h>
#include <modules.h>
#include <ramdisk.h>
#include <runtime.h>
#include <syscall.h>
#include <vfs/ramdisk.h>
#include <vfs/sys.h>

static struct ramdisk_image ramdiskImage;
static struct vfs_sys_filesystem sysFilesystem;
static struct vfs_ramdisk_filesystem ramdiskFilesystem;

static void load_usermode(struct kernel_arch *arch)
{

    unsigned int id;
    unsigned int slot;
    struct syscall_open_args oargs;
    struct syscall_execute_args eargs;
    struct runtime_mount *mount;
    struct runtime_task *task;

    task = runtime_get_task(1);
    runtime_task_init(task, 1);
    task->parent = 0;

    mount = task->get_mount(task, 1);
    runtime_mount_init(mount, 1, &sysFilesystem.base, 1, "/");
    mount = task->get_mount(task, 2);
    runtime_mount_init(mount, 2, &ramdiskFilesystem.base, 9, "/ramdisk/");

    oargs.index = 1;
    oargs.count = 17;
    oargs.buffer = "/ramdisk/bin/init";

    id = syscall_open(task, &oargs);

    error_assert(id != 0, "Init not found", __FILE__, __LINE__);

    eargs.index = id;

    slot = syscall_execute(task, &eargs);

    task = runtime_get_task(slot);
    task->parent = 0;

    arch->enter_usermode(task->registers.ip, task->registers.sp);

}

static void start(struct kernel_arch *self)
{

    union modules_module **modules;
    unsigned int i;

    self->setup(self);

    runtime_setup();
    modules = modules_setup();

    ramdisk_image_init(&ramdiskImage);

    for (i = 0; i < self->ramdiskc; i++)
    {

        unsigned int count = ramdiskImage.parse(&ramdiskImage, *(self->ramdiskv + i));

        error_assert(count != 0, "Ramdisk error", __FILE__, __LINE__);

    }

    vfs_sys_filesystem_init(&sysFilesystem, modules);
    modules_register_filesystem(&sysFilesystem.base);

    vfs_ramdisk_filesystem_init(&ramdiskFilesystem, &ramdiskImage);
    modules_register_filesystem(&ramdiskFilesystem.base);

    load_usermode(self);

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

