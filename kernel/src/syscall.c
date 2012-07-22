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
    struct elf_section_header *sheader;
    struct elf_section_header *symHeader;
    struct elf_symbol *symTable;
    char *strTable;

    if (!header)
        return 0;

    sheader = (struct elf_section_header *)(header->entry + header->shoffset);
    symHeader = elf_get_section(header, sheader, ELF_SECTION_TYPE_SYMTAB);
    symTable = (struct elf_symbol *)(header->entry + symHeader->offset);
    strTable = (char *)(header->entry + sheader[symHeader->link].offset);

    return elf_find_symbol(header, sheader, symHeader, symTable, strTable, func);

}

static void elf_relocate_section(struct elf_section_header *sheader, struct elf_section_header *relHeader, struct elf_section_header *relData, struct elf_relocate *relTable, struct elf_section_header *symHeader, struct elf_symbol *symTable, unsigned int address)
{

    unsigned int i;

    for (i = 0; i < relHeader->size / relHeader->esize; i++)
    {

        struct elf_relocate *relEntry = &relTable[i];
        unsigned char type = relEntry->info & 0x0F;
        unsigned char index = relEntry->info >> 8;
        struct elf_symbol *symEntry = &symTable[index];
        unsigned int *entry = (unsigned int *)(address + relData->offset + relEntry->offset);
        unsigned int value = *entry;
        unsigned int addend = (symEntry->shindex) ? address + sheader[symEntry->shindex].offset + symEntry->value : 0;

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

static void elf_relocate(struct elf_header *header, unsigned int address)
{

    struct elf_section_header *sheader = (struct elf_section_header *)(address + header->shoffset);
    unsigned int i;

    header->entry = address;

    for (i = 0; i < header->shcount; i++)
    {

        struct elf_section_header *relHeader;
        struct elf_section_header *relData;
        struct elf_section_header *symHeader;
        struct elf_relocate *relTable;
        struct elf_symbol *symTable;

        if (sheader[i].type != ELF_SECTION_TYPE_REL)
            continue;

        relHeader = &sheader[i];
        relData = &sheader[relHeader->info];
        symHeader = &sheader[relHeader->link];
        relTable = (struct elf_relocate *)(header->entry + relHeader->offset);
        symTable = (struct elf_symbol *)(header->entry + symHeader->offset);

        elf_relocate_section(sheader, relHeader, relData, relTable, symHeader, symTable, address);

        relData->address += header->entry;

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
    struct elf_header *header;
    void (*init)();
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->get_physical)
        return 0;

    physical = descriptor->mount->filesystem->get_physical(descriptor->mount->filesystem, descriptor->id);

    header = (struct elf_header *)physical;

    if (!elf_validate(header))
        return 0;

    elf_relocate(header, physical);

    init = (void (*)())(elf_get_func(descriptor, "init"));

    if (!init)
        return 0;

    init();

    return 1;

}

unsigned int syscall_mount(struct runtime_task *task, void *stack)
{

    struct syscall_mount_args *args = stack;
    unsigned int slot;
    struct runtime_mount *mount;
    struct modules_filesystem *(*get_filesystem)();
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->get_physical)
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

