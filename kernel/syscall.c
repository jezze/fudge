#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/mmu.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>

unsigned int syscall_attach(struct runtime_task *task, unsigned int index, unsigned int callback)
{

    if (!callback)
        return 0;

    return event_register_routine(index, task, callback);

}

unsigned int syscall_close(struct runtime_task *task, unsigned int index)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem)
        return 0;

    if (descriptor->filesystem->close)
        descriptor->filesystem->close(descriptor->filesystem, descriptor->id);

    runtime_descriptor_init(descriptor, 0, 0, 0);

    return 1;

}

unsigned int syscall_detach(struct runtime_task *task, unsigned int index)
{

    return event_unregister_routine(index, task);

}

unsigned int syscall_halt(struct runtime_task *task)
{

    kernel_enable_interrupts();
    kernel_halt();

    return 1;

}

unsigned int syscall_execute(struct runtime_task *task, unsigned int index)
{

    unsigned int slot;
    unsigned int count;
    struct runtime_task *ntask;
    struct elf_header *header;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->read)
        return 0;

    slot = runtime_get_task_slot();

    if (!slot)
        return 0;

    ntask = runtime_get_task(slot);

    runtime_task_clone(ntask, task, slot);
    mmu_map_user_memory(ntask->id, ntask->memory.paddress, ntask->memory.paddress, ntask->memory.size);
    mmu_load_memory(ntask->id);

    count = descriptor->filesystem->read(descriptor->filesystem, descriptor->id, 0, ntask->memory.size, (void *)ntask->memory.paddress);

    if (!count)
        return 0;

    header = elf_get_header((void *)ntask->memory.paddress);

    ntask->memory.vaddress = elf_get_virtual(header);

    if (!ntask->memory.vaddress)
        return 0;

    ntask->used = 1;
    ntask->parentid = task->id;

    runtime_registers_init(&ntask->registers, header->entry, ntask->memory.vaddress + ntask->memory.size, ntask->memory.vaddress + ntask->memory.size);
    runtime_set_running_task(ntask);

    return slot;

}

unsigned int syscall_exit(struct runtime_task *task)
{

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    task->used = 0;

    runtime_set_running_task(ptask);

    return ptask->id;

}

unsigned int syscall_load(struct runtime_task *task, unsigned int index)
{

    void *physical;
    void (*init)();
    struct elf_header *header;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->get_physical)
        return 0;

    physical = descriptor->filesystem->get_physical(descriptor->filesystem, descriptor->id);
    header = elf_get_header(physical);

    if (!header)
        return 0;

    elf_relocate(header);

    init = (void (*)())elf_get_symbol(header, "init");

    if (!init)
        return 0;

    init();

    return 1;

}

unsigned int syscall_open(struct runtime_task *task, unsigned int index, char *path)
{

    unsigned int slot;
    unsigned int id;
    struct runtime_descriptor *descriptor;
    struct modules_filesystem *filesystem;

    if (!path)
        return 0;

    slot = (index) ? index : task->get_descriptor_slot(task);

    if (!slot)
        return 0;

    descriptor = task->get_descriptor(task, slot);

    if (!descriptor)
        return 0;

    filesystem = modules_get_filesystem(path);

    if (!filesystem)
        return 0;

    id = filesystem->find(filesystem, path + string_length(filesystem->path));

    if (!id)
        return 0;

    runtime_descriptor_init(descriptor, id, filesystem, 0);

    if (descriptor->filesystem->open)
        descriptor->filesystem->open(descriptor->filesystem, descriptor->id);

    return slot;

}

unsigned int syscall_read(struct runtime_task *task, unsigned int index, unsigned int offset, unsigned int count, char *buffer)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->read)
        return 0;

    return descriptor->filesystem->read(descriptor->filesystem, descriptor->id, offset, count, buffer);

}

unsigned int syscall_reboot(struct runtime_task *task)
{

    return 1;

}

unsigned int syscall_unload(struct runtime_task *task, unsigned int index)
{

    void *physical;
    void (*destroy)();
    struct elf_header *header;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->get_physical)
        return 0;

    physical = descriptor->filesystem->get_physical(descriptor->filesystem, descriptor->id);
    header = elf_get_header(physical);

    if (!header)
        return 0;

    destroy = (void (*)())elf_get_symbol(header, "destroy");

    if (!destroy)
        return 0;

    destroy();

    return 1;

}

unsigned int syscall_wait(struct runtime_task *task)
{

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    task->event = 0;

    runtime_set_running_task(ptask);

    return ptask->id;

}

unsigned int syscall_write(struct runtime_task *task, unsigned int index, unsigned int offset, unsigned int count, char *buffer)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->write)
        return 0;

    return descriptor->filesystem->write(descriptor->filesystem, descriptor->id, offset, count, buffer);

}

