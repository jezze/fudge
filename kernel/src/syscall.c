#include <elf.h>
#include <memory.h>
#include <string.h>
#include <event.h>
#include <mmu.h>
#include <modules.h>
#include <runtime.h>
#include <syscall.h>

static unsigned int elf_get_func(struct runtime_descriptor *descriptor, char *func)
{

    unsigned int physical = descriptor->mount->filesystem->get_physical(descriptor->mount->filesystem, descriptor->id);
    struct elf_header *header = (struct elf_header *)physical;
    struct elf_section_header *sectionHeader = (struct elf_section_header *)(physical + header->shoffset);
    struct elf_section_header *symbolHeader = elf_get_section(header, sectionHeader, ELF_SECTION_TYPE_SYMTAB);
    struct elf_symbol *symbolTable = (struct elf_symbol *)(physical + symbolHeader->offset);
    char *stringTable = (char *)(physical + sectionHeader[symbolHeader->link].offset);

    return elf_find_symbol(header, sectionHeader, symbolHeader, symbolTable, stringTable, func);

}

static void elf_relocate_section(struct elf_section_header *sectionHeader, struct elf_section_header *relocateHeader, struct elf_section_header *relocateData, struct elf_relocate *relocateTable, struct elf_section_header *symbolHeader, struct elf_symbol *symbolTable, unsigned int address)
{

    unsigned int i;

    for (i = 0; i < relocateHeader->size / relocateHeader->esize; i++)
    {

        struct elf_relocate *relocateEntry = &relocateTable[i];
        unsigned char type = relocateEntry->info & 0x0F;
        unsigned char index = relocateEntry->info >> 8;
        struct elf_symbol *symbolEntry = &symbolTable[index];
        unsigned int *entry = (unsigned int *)(address + relocateData->offset + relocateEntry->offset);
        unsigned int value = *entry;
        unsigned int addend = (symbolEntry->shindex) ? address + sectionHeader[symbolEntry->shindex].offset + symbolEntry->value : 0;

        switch (type)
        {

            case 1:

                *entry = value + addend;

                break;

            case 2:

                *entry = value + addend - (unsigned int)entry;

                break;

        }

    }

}

static void elf_relocate(struct runtime_descriptor *descriptor, unsigned int address)
{

    unsigned int physical = descriptor->mount->filesystem->get_physical(descriptor->mount->filesystem, descriptor->id);
    struct elf_header *header = (struct elf_header *)physical;
    struct elf_section_header *sectionHeader = (struct elf_section_header *)(physical + header->shoffset);
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        struct elf_section_header *relocateHeader;
        struct elf_section_header *relocateData;
        struct elf_section_header *symbolHeader;
        struct elf_relocate *relocateTable;
        struct elf_symbol *symbolTable;

        if (sectionHeader[i].type != ELF_SECTION_TYPE_REL)
            continue;

        relocateHeader = &sectionHeader[i];
        relocateData = &sectionHeader[relocateHeader->info];
        symbolHeader = &sectionHeader[relocateHeader->link];
        relocateTable = (struct elf_relocate *)(physical + relocateHeader->offset);
        symbolTable = (struct elf_symbol *)(physical + symbolHeader->offset);

        elf_relocate_section(sectionHeader, relocateHeader, relocateData, relocateTable, symbolHeader, symbolTable, address);

        relocateData->address += address;

    }

}

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

    if (!descriptor || !descriptor->id)
        return 0;

    if (descriptor->mount->filesystem->close)
        descriptor->mount->filesystem->close(descriptor->mount->filesystem, descriptor->id);

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
    struct elf_header header;
    struct elf_program_header pheader;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->read)
        return 0;

    count = descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, 0, sizeof (struct elf_header), &header);

    if (!count)
        return 0;

    count = descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, header.phoffset, sizeof (struct elf_program_header), &pheader);

    if (!count)
        return 0;

    slot = runtime_get_task_slot();

    if (!slot)
        return 0;

    ntask = runtime_get_task(slot);

    runtime_task_clone(ntask, task, slot);
    ntask->memory.vaddress = pheader.vaddress;

    runtime_registers_init(&ntask->registers, header.entry, ntask->memory.vaddress + ntask->memory.size, ntask->memory.vaddress + ntask->memory.size);

    mmu_map_user_memory(ntask->id, ntask->memory.paddress, ntask->memory.vaddress, ntask->memory.size);
    mmu_load_memory(ntask->id);

    count = descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, 0, ntask->memory.size, (void *)ntask->memory.vaddress);

    if (!count)
        return 0;

    task->wait = 1;
    ntask->parent = task;
    ntask->used = 1;
    ntask->wait = 0;

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
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->get_physical)
        return 0;

    physical = descriptor->mount->filesystem->get_physical(descriptor->mount->filesystem, descriptor->id);

    if (!physical)
        return 0;

    /* Physical should be replaced with known address later on */
    elf_relocate(descriptor, physical);

    init = (void (*)())(elf_get_func(descriptor, "init"));

    if (!init)
        return 0;

    init();

    return 1;

}

unsigned int syscall_mount(struct runtime_task *task, void *stack)
{

    struct syscall_mount_args *args = stack;
    unsigned int physical;
    unsigned int slot;
    struct runtime_mount *mount;
    struct modules_filesystem *(*get_filesystem)();
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->get_physical)
        return 0;

    physical = descriptor->mount->filesystem->get_physical(descriptor->mount->filesystem, descriptor->id);

    if (!physical)
        return 0;

    slot = task->get_mount_slot(task);

    if (!slot)
        return 0;

    mount = task->get_mount(task, slot);

    if (!mount)
        return 0;

    get_filesystem = (struct modules_filesystem *(*)())(elf_get_func(descriptor, "get_filesystem"));

    if (!get_filesystem)
        return 0;

    runtime_mount_init(mount, slot, get_filesystem(), args->count, args->buffer);

    return slot;

}

unsigned int syscall_open(struct runtime_task *task, void *stack)
{

    unsigned int id;
    struct syscall_open_args *args = stack;
    struct runtime_mount *mount;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    if (!args->count)
        return 0;

    mount = task->find_mount(task, args->buffer);

    if (!mount)
        return 0;

    id = mount->filesystem->walk(mount->filesystem, descriptor->id, args->count - mount->count, (char *)args->buffer + mount->count);

    if (!id)
        return 0;

    runtime_descriptor_init(descriptor, id, mount);

    if (descriptor->mount->filesystem->open)
        descriptor->mount->filesystem->open(descriptor->mount->filesystem, descriptor->id);

    return args->index;

}

unsigned int syscall_read(struct runtime_task *task, void *stack)
{

    struct syscall_read_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->read)
        return 0;

    return descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, args->offset, args->count, args->buffer);

}

unsigned int syscall_unload(struct runtime_task *task, void *stack)
{

    struct syscall_unload_args *args = stack;
    void (*destroy)();
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->get_physical)
        return 0;

    destroy = (void (*)())(elf_get_func(descriptor, "destroy"));

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

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->write)
        return 0;

    return descriptor->mount->filesystem->write(descriptor->mount->filesystem, descriptor->id, args->offset, args->count, args->buffer);

}

