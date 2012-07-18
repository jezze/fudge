#include <elf.h>
#include <string.h>
#include <event.h>
#include <mmu.h>
#include <modules.h>
#include <runtime.h>
#include <syscall.h>

unsigned int syscall_attach(struct runtime_task *task, void *stack)
{

    struct syscall_attach_args *args = stack;

    if (!args->callback)
        return 0;

    return event_register_routine(args->index, task, args->callback);

}

unsigned int syscall_close(struct runtime_task *task, void *stack)
{

    struct syscall_close_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem)
        return 0;

    if (descriptor->filesystem->close)
        descriptor->filesystem->close(descriptor->filesystem, descriptor->id);

    descriptor->id = 0;

    return 1;

}

unsigned int syscall_detach(struct runtime_task *task, void *stack)
{

    struct syscall_detach_args *args = stack;

    return event_unregister_routine(args->index, task);

}

unsigned int syscall_execute(struct runtime_task *task, void *stack)
{

    struct syscall_execute_args *args = stack;
    unsigned int slot;
    unsigned int count;
    struct runtime_task *ntask;
    struct elf_header *header;
    struct elf_program_header *pheaders;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

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

    header = elf_get_header(ntask->memory.paddress);
    pheaders = (struct elf_program_header *)(ntask->memory.paddress + header->phoffset);
    ntask->memory.vaddress = pheaders[0].vaddress;

    if (!ntask->memory.vaddress)
        return 0;

    task->wait = 1;
    ntask->parent = task;
    ntask->used = 1;
    ntask->wait = 0;

    runtime_registers_init(&ntask->registers, header->entry, ntask->memory.vaddress + ntask->memory.size, ntask->memory.vaddress + ntask->memory.size);

    return slot;

}

unsigned int syscall_exit(struct runtime_task *task, void *stack)
{

    task->used = 0;
    task->wait = 1;

    if (task->parent && task->parent->id)
    {

        task->parent->wait = 0;

        return task->parent->id;

    }

    return 0;

}

unsigned int syscall_load(struct runtime_task *task, void *stack)
{

    struct syscall_load_args *args = stack;
    unsigned int physical;
    void (*init)();
    struct elf_header *header;
    struct elf_section_header *sheader;
    struct elf_section_header *symHeader;
    struct elf_symbol *symTable;
    char *strTable;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->get_physical)
        return 0;

    physical = descriptor->filesystem->get_physical(descriptor->filesystem, descriptor->id);
    header = elf_get_header(physical);

    if (!header)
        return 0;

    elf_relocate(header, physical);

    sheader = (struct elf_section_header *)(header->entry + header->shoffset);
    symHeader = elf_get_section(header, sheader, ELF_SECTION_TYPE_SYMTAB);
    symTable = (struct elf_symbol *)(header->entry + symHeader->offset);
    strTable = (char *)(header->entry + sheader[symHeader->link].offset);

    init = (void (*)())(elf_find_symbol(header, sheader, symHeader, symTable, strTable, "init"));

    if (!init)
        return 0;

    init();

    return 1;

}

unsigned int syscall_open(struct runtime_task *task, void *stack)
{

    unsigned int id;
    struct syscall_open_args *args = stack;
    struct runtime_descriptor *descriptor;
    struct modules_filesystem *filesystem;

    descriptor = task->get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    if (!args->buffer)
        return 0;

    filesystem = modules_get_filesystem(args->buffer);

    if (!filesystem)
        return 0;

    id = filesystem->walk(filesystem, descriptor->id, string_length((char *)args->buffer + string_length(filesystem->path)), (char *)args->buffer + string_length(filesystem->path));

    if (!id)
        return 0;

    runtime_descriptor_init(descriptor, id, filesystem, 0);

    if (descriptor->filesystem->open)
        descriptor->filesystem->open(descriptor->filesystem, descriptor->id);

    return args->index;

}

unsigned int syscall_read(struct runtime_task *task, void *stack)
{

    struct syscall_read_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->read)
        return 0;

    return descriptor->filesystem->read(descriptor->filesystem, descriptor->id, args->offset, args->count, args->buffer);

}

unsigned int syscall_unload(struct runtime_task *task, void *stack)
{

    struct syscall_unload_args *args = stack;
    unsigned int physical;
    void (*destroy)();
    struct elf_header *header;
    struct elf_section_header *sheader;
    struct elf_section_header *symHeader;
    struct elf_symbol *symTable;
    char *strTable;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->get_physical)
        return 0;

    physical = descriptor->filesystem->get_physical(descriptor->filesystem, descriptor->id);
    header = elf_get_header(physical);

    if (!header)
        return 0;

    sheader = (struct elf_section_header *)(header->entry + header->shoffset);
    symHeader = elf_get_section(header, sheader, ELF_SECTION_TYPE_SYMTAB);
    symTable = (struct elf_symbol *)(header->entry + symHeader->offset);
    strTable = (char *)(header->entry + sheader[symHeader->link].offset);

    destroy = (void (*)())(elf_find_symbol(header, sheader, symHeader, symTable, strTable, "destroy"));

    if (!destroy)
        return 0;

    destroy();

    return 1;

}

unsigned int syscall_wait(struct runtime_task *task, void *stack)
{

    task->event = 0;
    task->wait = 1;

    if (task->parent && task->parent->id)
    {

        task->parent->wait = 0;

        return task->parent->id;

    }

    return 0;

}

unsigned int syscall_write(struct runtime_task *task, void *stack)
{

    struct syscall_write_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->filesystem || !descriptor->filesystem->write)
        return 0;

    return descriptor->filesystem->write(descriptor->filesystem, descriptor->id, args->offset, args->count, args->buffer);

}

