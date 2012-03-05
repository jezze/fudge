#include <lib/elf.h>
#include <lib/string.h>
#include <kernel/elf.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/mmu.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>

static void *routines[SYSCALL_ROUTINE_SLOTS];

unsigned int syscall_attach(struct runtime_task *task, unsigned int index, void (*routine)())
{

    if (!index)
        return 0;

    if (!routine)
        return 0;

    return event_register_routine(index, task, routine);

}

unsigned int syscall_close(struct runtime_task *task, unsigned int index)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id)
        return 0;

    if (descriptor->filesystem->close)
        descriptor->filesystem->close(descriptor->filesystem, descriptor->id);

    runtime_descriptor_init(descriptor, 0, 0, 0);

    event_raise(EVENT_SYSCALL_CLOSE);

    return 1;

}

unsigned int syscall_detach(struct runtime_task *task, unsigned int index)
{

    if (!index)
        return 0;

    return event_unregister_routine(index, task);

}

unsigned int syscall_halt(struct runtime_task *task)
{

    kernel_enable_interrupts();
    kernel_halt();

    return 1;

}

unsigned int syscall_execute(struct runtime_task *task, char *path, unsigned int argc, char **argv)
{

    if (!path)
        return 0;

    unsigned int index = runtime_get_task_slot();

    if (!index)
        return 0;

    struct runtime_task *ntask = runtime_get_task(index);

    runtime_task_init(ntask, index);

    struct vfs_filesystem *filesystem2 = vfs_find_filesystem(path);

    if (!filesystem2)
        return 0;

    unsigned int id = filesystem2->find(filesystem2, path + string_length(filesystem2->name));

    if (!id)
        return 0;

    unsigned int count = filesystem2->read(filesystem2, id, ntask->memory.size, ntask->memory.paddress);

    if (!count)
        return 0;

    ntask->memory.vaddress = elf_get_virtual(ntask->memory.paddress);

    if (!ntask->memory.vaddress)
        return 0;

    void *entry = elf_get_entry(ntask->memory.paddress);

    if (!entry)
        return 0;

    if (!ntask->load(ntask, entry, argc, argv))
        return 0;

    elf_prepare(ntask->memory.vaddress);

    runtime_activate(ntask, task);

    struct vfs_filesystem *filesystem = vfs_find_filesystem("/module/");

    if (filesystem)
    {

        runtime_descriptor_init(ntask->get_descriptor(ntask, 1), filesystem->find(filesystem, "tty/stdin"), filesystem, 0);
        runtime_descriptor_init(ntask->get_descriptor(ntask, 2), filesystem->find(filesystem, "tty/stdout"), filesystem, 0);
        runtime_descriptor_init(ntask->get_descriptor(ntask, 3), filesystem->find(filesystem, "tty/stderr"), filesystem, 0);

    }

    event_raise(EVENT_SYSCALL_EXECUTE);

    return ntask->id;

}

unsigned int syscall_exit(struct runtime_task *task)
{

    task->unload(task);

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    if (!ptask->used)
    {

        kernel_enable_interrupts();
        kernel_halt();

    }

    runtime_activate(ptask, 0);

    event_raise(EVENT_SYSCALL_EXIT);

    return ptask->id;

}

unsigned int syscall_load(struct runtime_task *task, char *path)
{

    if (!path)
        return 0;

    struct vfs_filesystem *filesystem = vfs_find_filesystem(path);

    if (!filesystem)
        return 0;

    unsigned int id = filesystem->find(filesystem, path + string_length(filesystem->name));

    if (!id)
        return 0;

    void *physical = filesystem->get_physical(filesystem, id);

    if (!physical)
        return 0;

    elf_relocate(physical);

    void (*init)() = elf_get_symbol(physical, "init");

    if (!init)
        return 0;

    init();

    event_raise(EVENT_SYSCALL_LOAD);

    return 1;

}

unsigned int syscall_open(struct runtime_task *task, char *path)
{

    if (!path)
        return 0;

    unsigned int index = task->get_descriptor_slot(task);

    if (!index)
        return 0;

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor)
        return 0;

    struct vfs_filesystem *filesystem = vfs_find_filesystem(path);

    if (!filesystem)
        return 0;

    unsigned int id = filesystem->find(filesystem, path + string_length(filesystem->name));

    if (!id)
        return 0;

    runtime_descriptor_init(descriptor, id, filesystem, 0);

    if (descriptor->filesystem->open)
        descriptor->filesystem->open(descriptor->filesystem, descriptor->id);

    event_raise(EVENT_SYSCALL_OPEN);

    return index;

}

unsigned int syscall_read(struct runtime_task *task, unsigned int index, unsigned int count, char *buffer)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor->id || !descriptor->filesystem || !descriptor->filesystem->read)
        return 0;

    unsigned int c = descriptor->filesystem->read(descriptor->filesystem, descriptor->id, count, buffer);

    event_raise(EVENT_SYSCALL_READ);

    return c;

}

unsigned int syscall_reboot(struct runtime_task *task)
{

    kernel_reboot();

    return 1;

}

unsigned int syscall_unload(struct runtime_task *task, char *path)
{

    if (!path)
        return 0;

    struct vfs_filesystem *filesystem = vfs_find_filesystem(path);

    if (!filesystem)
        return 0;

    unsigned int id = filesystem->find(filesystem, path + string_length(filesystem->name));

    if (!id)
        return 0;

    void *physical = filesystem->get_physical(filesystem, id);

    if (!physical)
        return 0;

    void (*destroy)() = elf_get_symbol(physical, "destroy");

    if (!destroy)
        return 0;

    destroy();

    event_raise(EVENT_SYSCALL_UNLOAD);

    return 1;

}

unsigned int syscall_wait(struct runtime_task *task)
{

    task->event = 0;

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    if (!ptask->used)
    {

        kernel_enable_interrupts();
        kernel_halt();

    }

    runtime_activate(ptask, 0);

    event_raise(EVENT_SYSCALL_WAIT);

    return ptask->id;

}

unsigned int syscall_write(struct runtime_task *task, unsigned int index, unsigned int count, char *buffer)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor->id || !descriptor->filesystem || !descriptor->filesystem->write)
        return 0;

    unsigned int c = descriptor->filesystem->write(descriptor->filesystem, descriptor->id, count, buffer);

    event_raise(EVENT_SYSCALL_WRITE);

    return c;

}

static unsigned int syscall_handle_attach(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    void (*routine)() = *(void **)(stack + 8);

    return syscall_attach(task, index, routine);

}

static unsigned int syscall_handle_close(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return syscall_close(task, index);

}

static unsigned int syscall_handle_detach(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return syscall_detach(task, index);

}

static unsigned int syscall_handle_halt(unsigned int stack, struct runtime_task *task)
{

    return syscall_halt(task);

}

static unsigned int syscall_handle_execute(unsigned int stack, struct runtime_task *task)
{

    char *path = *(char **)(stack + 24);
    unsigned int argc = *(unsigned int *)(stack + 28);
    char **argv = *(char ***)(stack + 32);

    return syscall_execute(task, path, argc, argv);

}

static unsigned int syscall_handle_exit(unsigned int stack, struct runtime_task *task)
{

    return syscall_exit(task);

}

static unsigned int syscall_handle_load(unsigned int stack, struct runtime_task *task)
{

    char *path = *(char **)(stack + 4);

    return syscall_load(task, path);

}

static unsigned int syscall_handle_open(unsigned int stack, struct runtime_task *task)
{

    char *path = *(char **)(stack + 4);

    return syscall_open(task, path);

}

static unsigned int syscall_handle_read(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    char *buffer = *(char **)(stack + 8);
    unsigned int count = *(unsigned int *)(stack + 12);

    return syscall_read(task, index, count, buffer);

}

static unsigned int syscall_handle_reboot(unsigned int stack, struct runtime_task *task)
{

    return syscall_reboot(task);

}

static unsigned int syscall_handle_unload(unsigned int stack, struct runtime_task *task)
{

    char *path = *(char **)(stack + 4);

    return syscall_unload(task, path);

}

static unsigned int syscall_handle_wait(unsigned int stack, struct runtime_task *task)
{

    return syscall_wait(task);

}

static unsigned int syscall_handle_write(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    char *buffer = *(char **)(stack + 8);
    unsigned int count = *(unsigned int *)(stack + 12);

    return syscall_write(task, index, count, buffer);

}

static void syscall_register_routine(unsigned char index, unsigned int (*routine)(unsigned int stack, struct runtime_task *task))
{

    routines[index] = routine;

}

unsigned int syscall_raise(unsigned int index, unsigned int stack, struct runtime_task *task)
{

    unsigned int (*routine)(unsigned int stack, struct runtime_task *task) = routines[index];

    if (!routine)
        return 0;

    return routine(stack, task);

}

void syscall_init()
{

    syscall_register_routine(SYSCALL_ROUTINE_OPEN, syscall_handle_open);
    syscall_register_routine(SYSCALL_ROUTINE_CLOSE, syscall_handle_close);
    syscall_register_routine(SYSCALL_ROUTINE_READ, syscall_handle_read);
    syscall_register_routine(SYSCALL_ROUTINE_WRITE, syscall_handle_write);
    syscall_register_routine(SYSCALL_ROUTINE_EXECUTE, syscall_handle_execute);
    syscall_register_routine(SYSCALL_ROUTINE_EXIT, syscall_handle_exit);
    syscall_register_routine(SYSCALL_ROUTINE_WAIT, syscall_handle_wait);
    syscall_register_routine(SYSCALL_ROUTINE_LOAD, syscall_handle_load);
    syscall_register_routine(SYSCALL_ROUTINE_UNLOAD, syscall_handle_unload);
    syscall_register_routine(SYSCALL_ROUTINE_HALT, syscall_handle_halt);
    syscall_register_routine(SYSCALL_ROUTINE_REBOOT, syscall_handle_reboot);
    syscall_register_routine(SYSCALL_ROUTINE_ATTACH, syscall_handle_attach);
    syscall_register_routine(SYSCALL_ROUTINE_DETACH, syscall_handle_detach);

}

