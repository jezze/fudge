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

    struct vfs_node *node = vfs_find(path);

    if (!node)
    {

        registers->eax = -1;

        return;

    }

    struct runtime_task *task = runtime_get_running_task();    

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

    struct runtime_task *task = runtime_get_running_task();
    task->load(task, path, argc, argv);

    struct mmu_header *pHeader = mmu_get_program_header();
    struct elf_header *header = elf_get_header(pHeader->address);
    struct elf_program_header *programHeader = elf_get_program_header(header);

    char **sa = pHeader->address + 0xFC00;
    void *ss = pHeader->address + 0xFD00;

    unsigned int i;
    unsigned int offset = 0;

    for (i = 0; i < argc; i++)
    {

        sa[i] = programHeader->virtualAddress + 0xFD00 + offset;

        unsigned int length = string_length(argv[i]);
        string_copy(ss + offset, argv[i]);

        offset += length + 2;

    }

    argv = programHeader->virtualAddress + 0xFC00;

    memory_set(pHeader->address + 0xFFFF, ((unsigned int)argv & 0xFF000000) >> 24, 1);
    memory_set(pHeader->address + 0xFFFE, ((unsigned int)argv & 0x00FF0000) >> 16, 1);
    memory_set(pHeader->address + 0xFFFD, ((unsigned int)argv & 0x0000FF00) >> 8, 1);
    memory_set(pHeader->address + 0xFFFC, ((unsigned int)argv & 0x000000FF) >> 0, 1);
    memory_set(pHeader->address + 0xFFFB, (argc & 0xFF000000) >> 24, 1);
    memory_set(pHeader->address + 0xFFFA, (argc & 0x00FF0000) >> 16, 1);
    memory_set(pHeader->address + 0xFFF9, (argc & 0x0000FF00) >> 8, 1);
    memory_set(pHeader->address + 0xFFF8, (argc & 0x000000FF) >> 0, 1);
    memory_set(pHeader->address + 0xFFF7, (registers->eip & 0xFF000000) >> 24, 1);
    memory_set(pHeader->address + 0xFFF6, (registers->eip & 0x00FF0000) >> 16, 1);
    memory_set(pHeader->address + 0xFFF5, (registers->eip & 0x0000FF00) >> 8, 1);
    memory_set(pHeader->address + 0xFFF4, (registers->eip & 0x000000FF) >> 0, 1);

    mmu_map(pHeader, programHeader->virtualAddress, 0x10000, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
    mmu_set_directory(&pHeader->directory);

    registers->eip = (unsigned int)header->entry;
    registers->useresp = (unsigned int)(programHeader->virtualAddress + 0xFFF4);
    registers->eax = 0;

}

static void syscall_reboot(struct syscall_registers *registers)
{

    arch_reboot();

}

void syscall_init()
{

    syscall_register_handler(SYSCALL_ROUTINE_OPEN, syscall_open);
    syscall_register_handler(SYSCALL_ROUTINE_CLOSE, syscall_close);
    syscall_register_handler(SYSCALL_ROUTINE_READ, syscall_read);
    syscall_register_handler(SYSCALL_ROUTINE_WRITE, syscall_write);
    syscall_register_handler(SYSCALL_ROUTINE_INFO, syscall_info);
    syscall_register_handler(SYSCALL_ROUTINE_EXECUTE, syscall_execute);
    syscall_register_handler(SYSCALL_ROUTINE_REBOOT, syscall_reboot);

}

