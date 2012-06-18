#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/kernel.h>
#include <kernel/event.h>
#include <kernel/mmu.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>

unsigned int syscall_attach(struct kernel_context *context, unsigned int index, unsigned int callback)
{

    if (!callback)
        return 0;

    return event_register_routine(index, context->running, callback);

}

unsigned int syscall_close(struct kernel_context *context, unsigned int index)
{

    struct runtime_descriptor *descriptor = context->running->get_descriptor(context->running, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem)
        return 0;

    if (descriptor->filesystem->close)
        descriptor->filesystem->close(descriptor->filesystem, descriptor->id);

    runtime_descriptor_init(descriptor, 0, 0, 0);

    return 1;

}

unsigned int syscall_detach(struct kernel_context *context, unsigned int index)
{

    return event_unregister_routine(index, context->running);

}

unsigned int syscall_halt(struct kernel_context *context)
{

    return 1;

}

unsigned int syscall_execute(struct kernel_context *context, unsigned int index)
{

    unsigned int slot;
    unsigned int count;
    struct runtime_task *ntask;
    struct elf_header *header;
    struct elf_program_header *pheaders;
    struct runtime_descriptor *descriptor = context->running->get_descriptor(context->running, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->read)
        return 0;

    slot = runtime_get_task_slot();

    if (!slot)
        return 0;

    ntask = runtime_get_task(slot);

    runtime_task_clone(ntask, context->running, slot);
    mmu_map_user_memory(ntask->id, ntask->memory.paddress, ntask->memory.paddress, ntask->memory.size);
    mmu_load_memory(ntask->id);

    count = descriptor->filesystem->read(descriptor->filesystem, descriptor->id, 0, ntask->memory.size, (void *)ntask->memory.paddress);

    if (!count)
        return 0;

    header = elf_get_header((void *)ntask->memory.paddress);
    pheaders = (struct elf_program_header *)(ntask->memory.paddress + header->phoffset);
    ntask->memory.vaddress = pheaders[0].vaddress;

    if (!ntask->memory.vaddress)
        return 0;

    ntask->used = 1;
    ntask->parentid = context->running->id;

    runtime_registers_init(&ntask->registers, header->entry, ntask->memory.vaddress + ntask->memory.size, ntask->memory.vaddress + ntask->memory.size);

    context->running = ntask;

    return slot;

}

unsigned int syscall_exit(struct kernel_context *context)
{

    context->running->used = 0;
    context->running = runtime_get_task(context->running->parentid);

    return context->running->id;

}

unsigned int syscall_load(struct kernel_context *context, unsigned int index)
{

    void *physical;
    void (*init)();
    struct elf_header *header;
    struct elf_section_header *sheader;
    struct elf_section_header *relHeader;
    struct elf_section_header *relData;
    struct runtime_descriptor *descriptor = context->running->get_descriptor(context->running, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->get_physical)
        return 0;

    physical = descriptor->filesystem->get_physical(descriptor->filesystem, descriptor->id);
    header = elf_get_header(physical);

    if (!header)
        return 0;

    elf_relocate(header, (unsigned int)physical);

    sheader = (struct elf_section_header *)((unsigned int)physical + header->shoffset);
    relHeader = &sheader[2];
    relData = &sheader[relHeader->info];

    init = (void (*)())(elf_get_symbol(header, "init") + (unsigned int)physical + relData->offset);

    if (!init)
        return 0;

    init();

    return 1;

}

unsigned int syscall_open(struct kernel_context *context, unsigned int index, char *path)
{

    unsigned int slot;
    unsigned int id;
    struct runtime_descriptor *descriptor;
    struct modules_filesystem *filesystem;

    if (!path)
        return 0;

    slot = (index) ? index : context->running->get_descriptor_slot(context->running);

    if (!slot)
        return 0;

    descriptor = context->running->get_descriptor(context->running, slot);

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

unsigned int syscall_read(struct kernel_context *context, unsigned int index, unsigned int offset, unsigned int count, char *buffer)
{

    struct runtime_descriptor *descriptor = context->running->get_descriptor(context->running, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->read)
        return 0;

    return descriptor->filesystem->read(descriptor->filesystem, descriptor->id, offset, count, buffer);

}

unsigned int syscall_reboot(struct kernel_context *context)
{

    return 1;

}

unsigned int syscall_unload(struct kernel_context *context, unsigned int index)
{

    void *physical;
    void (*destroy)();
    struct elf_header *header;
    struct elf_section_header *sheader;
    struct elf_section_header *relHeader;
    struct elf_section_header *relData;
    struct runtime_descriptor *descriptor = context->running->get_descriptor(context->running, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->get_physical)
        return 0;

    physical = descriptor->filesystem->get_physical(descriptor->filesystem, descriptor->id);
    header = elf_get_header(physical);

    if (!header)
        return 0;

    sheader = (struct elf_section_header *)((unsigned int)physical + header->shoffset);
    relHeader = &sheader[2];
    relData = &sheader[relHeader->info];

    destroy = (void (*)())(elf_get_symbol(header, "destroy") + (unsigned int)physical + relData->offset);

    if (!destroy)
        return 0;

    destroy();

    return 1;

}

unsigned int syscall_wait(struct kernel_context *context)
{

    context->running->event = 0;
    context->running = runtime_get_task(context->running->parentid);

    return context->running->id;

}

unsigned int syscall_write(struct kernel_context *context, unsigned int index, unsigned int offset, unsigned int count, char *buffer)
{

    struct runtime_descriptor *descriptor = context->running->get_descriptor(context->running, index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->write)
        return 0;

    return descriptor->filesystem->write(descriptor->filesystem, descriptor->id, offset, count, buffer);

}

