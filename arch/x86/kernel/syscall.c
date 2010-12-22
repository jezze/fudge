#include <lib/call.h>
#include <lib/elf.h>
#include <lib/file.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/mmu.h>
#include <arch/x86/kernel/pit.h>
#include <arch/x86/kernel/rtc.h>
#include <arch/x86/kernel/syscall.h>
#include <kernel/kernel.h>
#include <kernel/vfs.h>

void *syscallRoutines[SYSCALL_ROUTINES_SIZE];

struct mmu_directory syscallProgramDirectory;
struct mmu_table syscallProgramTables[3];

void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers))
{

    syscallRoutines[index] = handler;

}

void syscall_unregister_handler(unsigned char index)
{

    syscallRoutines[index] = 0;

}

void syscall_open(struct syscall_registers *registers)
{

    registers->eax = (unsigned int)file_find(vfs_get_root(), (char *)registers->esi + 1);

}

void syscall_reboot(struct syscall_registers *registers)
{

    arch_reboot();

}

void syscall_load(struct syscall_registers *registers)
{

    unsigned int address = registers->ebx;

    struct elf_header *header = (struct elf_header *)address;
    struct elf_program_header *programHeader = (struct elf_program_header *)(address + header->programHeaderOffset);

    mmu_clear_directory(&syscallProgramDirectory);
    mmu_clear_table(&syscallProgramTables[0]);
    mmu_clear_table(&syscallProgramTables[1]);
    mmu_clear_table(&syscallProgramTables[2]);

    unsigned int index = (programHeader->virtualAddress / MMU_PAGE_SIZE) / MMU_DIRECTORY_SIZE;

    mmu_add_table(&syscallProgramDirectory, 0, &syscallProgramTables[0], MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE);
    mmu_add_table(&syscallProgramDirectory, index, &syscallProgramTables[1], MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE);
    mmu_add_table(&syscallProgramDirectory, index + 1, &syscallProgramTables[2], MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE);

    mmu_map(&syscallProgramDirectory, 0x00000000, 0x00000000, 0x00400000, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);
    mmu_map(&syscallProgramDirectory, programHeader->virtualAddress, address, programHeader->memorySize, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

    mmu_set_directory(&syscallProgramDirectory);

    registers->eax = header->entry;

}

void syscall_execute(struct syscall_registers *registers)
{

    int argc = (int)registers->ecx;
    char **argv = (char **)registers->esi;

    void (*func)(int argc, char **argv) = (void (*)(int argc, char **argv))registers->ebx;

    func(argc, argv);

}

void syscall_unload(struct syscall_registers *registers)
{

    mmu_default_directory();

}

void syscall_handler(struct syscall_registers *registers)
{

    void (*handler)(struct syscall_registers *registers) = syscallRoutines[registers->eax];

    if (handler)
        handler(registers);

}

void syscall_init()
{

}

