#include <lib/elf.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>
#include <kernel/arch/x86/syscall.h>
#include <modules/elf/elf.h>

static void *syscallRoutines[SYSCALL_ROUTINES_SIZE];

void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers))
{

    syscallRoutines[index] = handler;

}

void syscall_unregister_handler(unsigned char index)
{

    syscallRoutines[index] = 0;

}

void syscall_handler(struct syscall_registers *registers)
{

    void (*handler)(struct syscall_registers *registers) = syscallRoutines[registers->eax];

    if (handler)
        handler(registers);

}

static void syscall_open(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = vfs_find(path);

    if (!node)
    {

        registers->eax = -1;

        return;

    }

    struct vfs_descriptor *descriptor = task->add_descriptor(task, node);

    if (!descriptor)
    {

        registers->eax = -1;

        return;

    }

    registers->eax = descriptor->index;

}

static void syscall_close(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;

    struct runtime_task *task = runtime_get_running_task();    

    task->remove_descriptor(task, fd);

}

static void syscall_read(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    char *buffer = (char *)registers->esi;
    unsigned int count = registers->ecx;

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!(node && node->operations.read))
    {

        registers->eax = 0;

        return;

    }

    registers->eax = node->operations.read(node, count, buffer);

}

static void syscall_write(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    char *buffer = (char *)registers->esi;
    unsigned int count = registers->ecx;

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!(node && node->operations.write))
    {

        registers->eax = 0;

        return;

    }

    registers->eax = node->operations.write(node, count, buffer);

}

static void syscall_info(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    struct file_info *info = (struct file_info *)registers->edi;
    
    struct runtime_task *task = runtime_get_running_task();

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!node)
    {

        registers->eax = 0;

        return;

    }

    info->id = node->id;
    info->length = 0;

    registers->eax = 1;

}

static void syscall_execute(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;
    unsigned int argc = registers->ecx;
    char **argv = (char **)registers->ebx;

    struct runtime_task *oldtask = runtime_get_running_task();
    oldtask->eip = (void *)registers->eip;
    oldtask->esp = (void *)registers->useresp;
    oldtask->ebp = (void *)registers->ebp;

    struct runtime_task *task = runtime_get_free_task();
    task->parentpid = oldtask->pid;

    if (!task->load(task, path, argc, argv))
    {

        registers->eax = 0;

        return;

    }

    runtime_activate(task);

    registers->eip = (unsigned int)task->eip;
    registers->useresp = (unsigned int)task->esp;
    registers->ebp = (unsigned int)task->ebp;
    registers->eax = 1;

}

static void syscall_exit(struct syscall_registers *registers)
{

    struct runtime_task *oldtask = runtime_get_running_task();
    oldtask->unload(oldtask);

    struct runtime_task *task = runtime_get_task(oldtask->parentpid);

    runtime_activate(task);

    registers->eip = (unsigned int)task->eip;
    registers->useresp = (unsigned int)task->esp;
    registers->ebp = (unsigned int)task->ebp;
    registers->eax = 1;

}

static void syscall_reboot(struct syscall_registers *registers)
{

    arch_reboot();

}

void *buffer[0x2000];
void *physical;

void doit()
{

//    file_write_format(FILE_STDOUT, "Module loaded\n");

}

static void print_symtab(struct elf_section_header *header)
{

    unsigned int i;

    for (i = 0; i < header->size / header->entrySize; i++)
    {

        struct elf_symbol *symHeader = (struct elf_symbol *)(buffer + header->offset + i * header->entrySize);

        unsigned int bind = symHeader->info >> 4;
        unsigned int type = symHeader->info & 0x0F;

        if (bind == 0x01)
        {

            file_write_format(FILE_STDERR, "  [%d] SYM - Value: 0x%x Size: %d Info: 0x%x Bind: 0x%x Type: 0x%x Shndx: 0x%x\n", i, symHeader->value, symHeader->size, symHeader->info, bind, type, symHeader->shndx);

        }

    }

}

static void print_rel(struct elf_section_header *header)
{

    unsigned int i;

    file_write_format(FILE_STDERR, "  Address 0x%x\n", *((unsigned int *)physical + 0x44));

    for (i = 0; i < header->size / header->entrySize; i++)
    {

        struct elf_relocate *rHeader = (struct elf_relocate *)(buffer + header->offset + i * header->entrySize);

        unsigned int sym = rHeader->info >> 4;
        unsigned int type = rHeader->info & 0x0F;

        file_write_format(FILE_STDERR, "  [%d] REL - Offset: 0x%x Info: 0x%x Sym: 0x%x Type: 0x%x\n", i, rHeader->offset, rHeader->info, sym, type);

        void *offset = physical + 0x40;

        unsigned int *pdoit = offset + rHeader->offset;
 
        file_write_format(FILE_STDERR, "    Before: 0x%x\n", *pdoit);

        if (sym == 0x90)
            *pdoit = (unsigned int)physical - (unsigned int)doit + rHeader->offset;
        else
            *pdoit = (unsigned int)physical - 0 + rHeader->offset;

        file_write_format(FILE_STDERR, "    Phys: 0x%x Offset: 0x%x pDoit: 0x%x Doit: 0x%x *pDoit: 0x%x\n", physical, offset, pdoit, doit, *pdoit);

    }

}

static void print_sections(struct elf_header *header)
{

    unsigned int i;

    for (i = 0; i < header->sectionHeaderCount; i++)
    {

        struct elf_section_header *sHeader = (struct elf_section_header *)(buffer + header->sectionHeaderOffset + i * header->sectionHeaderSize);

        file_write_format(FILE_STDERR, "[%d] SH - Offset: 0x%x Size:0x%x\n", i, sHeader->offset, sHeader->size);

        if (sHeader->type == 0x02)
            print_symtab(sHeader);

        if (sHeader->type == 0x09)
            print_rel(sHeader);

    }

}

static void syscall_load(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;

    struct vfs_node *node = vfs_find(path);
    physical = node->physical;

    node->operations.read(node, 0x2000, buffer);

    file_write_format(FILE_STDERR, "Physical: %d\n", physical);
    file_write_format(FILE_STDERR, "Buffer: %d\n", buffer);

    struct elf_header *header = (struct elf_header *)buffer;

//    print_sections(header);

    int *s;

    int reloc = (int)buffer + 0x40;

    s = (int *)(reloc + 0x04);
    *s = (int)&doit - reloc + 0x04;

    s = (int *)(reloc + 0x0C);
    *s = (int)&doit - reloc + 0x0C;

    void (*minit)() = (void *)reloc;
    minit();

    registers->eax = 1;

}

static void syscall_unload(struct syscall_registers *registers)
{

    registers->eax = 1;

}

void syscall_init()
{

    syscall_register_handler(SYSCALL_ROUTINE_OPEN, syscall_open);
    syscall_register_handler(SYSCALL_ROUTINE_CLOSE, syscall_close);
    syscall_register_handler(SYSCALL_ROUTINE_READ, syscall_read);
    syscall_register_handler(SYSCALL_ROUTINE_WRITE, syscall_write);
    syscall_register_handler(SYSCALL_ROUTINE_INFO, syscall_info);
    syscall_register_handler(SYSCALL_ROUTINE_EXECUTE, syscall_execute);
    syscall_register_handler(SYSCALL_ROUTINE_EXIT, syscall_exit);
    syscall_register_handler(SYSCALL_ROUTINE_LOAD, syscall_load);
    syscall_register_handler(SYSCALL_ROUTINE_UNLOAD, syscall_unload);
    syscall_register_handler(SYSCALL_ROUTINE_REBOOT, syscall_reboot);

}

