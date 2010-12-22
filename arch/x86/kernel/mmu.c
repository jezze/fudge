#include <lib/file.h>
#include <lib/memory.h>
#include <lib/session.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/cr.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/mmu.h>
#include <kernel/kernel.h>

struct mmu_directory mmuKernelDirectory;
struct mmu_table mmuKernelTables[2];

void mmu_handler(struct isr_registers *registers)
{

    unsigned int address = cr2_read();

    file_write_string(session_get_out(), "PAGE FAULT (");

    if (!(registers->err_code & MMU_ERROR_PRESENT))
        file_write_string(session_get_out(), "present");

    if (registers->err_code & MMU_ERROR_RW)
        file_write_string(session_get_out(), "read-only");

    if (registers->err_code & MMU_ERROR_USER)
        file_write_string(session_get_out(), "user-mode");

    if (registers->err_code & MMU_ERROR_RESERVED)
        file_write_string(session_get_out(), "reserved");

    if (registers->err_code & MMU_ERROR_FETCH)
        file_write_string(session_get_out(), "fetch");

    file_write_string(session_get_out(), ") at 0x");
    file_write_hex(session_get_out(), address);
    file_write_string(session_get_out(), "\n");

    kernel_panic("PAGE FAULT", __FILE__, __LINE__);

}

void mmu_map(struct mmu_directory *directory, unsigned int virtualAddress, unsigned int physicalAddress, unsigned int size, unsigned int flags)
{

    unsigned int i = virtualAddress / 4096;
    unsigned int count = (size % 4096) ? (size / 4096) + 4096 : (size / 4096);

    for (; count; count--)
    {

        struct mmu_table *itable = (struct mmu_table *)((unsigned int)directory->tables[i / MMU_DIRECTORY_SIZE] & 0xFFFFF000);

        itable->entries[i % MMU_TABLE_SIZE] = physicalAddress | flags;

        virtualAddress += MMU_PAGE_SIZE;
        physicalAddress += MMU_PAGE_SIZE;
        i++;

    }

}

void mmu_set_directory(struct mmu_directory *directory)
{

    cr3_write((unsigned int)directory);

}

void mmu_default_directory()
{

    mmu_set_directory(&mmuKernelDirectory);

}

void mmu_enable()
{

    cr0_write(cr0_read() | 0x80000000);

}

void mmu_clear_directory(struct mmu_directory *directory)
{

    memory_set(directory, 0, sizeof (struct mmu_directory));

}

void mmu_clear_table(struct mmu_table *table)
{

    memory_set(table, 0, sizeof (struct mmu_table));

}

void mmu_add_table(struct mmu_directory *directory, unsigned int index, struct mmu_table *table, unsigned int flags)
{

    directory->tables[index] = (struct mmu_table *)((unsigned int)table | flags);

}

static void mmu_init_kernel_directory()
{

    mmu_clear_directory(&mmuKernelDirectory);
    mmu_clear_table(&mmuKernelTables[0]);
    mmu_clear_table(&mmuKernelTables[1]);

    mmu_add_table(&mmuKernelDirectory, 0, &mmuKernelTables[0], MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE);
    mmu_add_table(&mmuKernelDirectory, 1, &mmuKernelTables[1], MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE);

    mmu_map(&mmuKernelDirectory, 0x00000000, 0x00000000, 0x00800000, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

}

void mmu_init()
{

    mmu_init_kernel_directory();
    mmu_set_directory(&mmuKernelDirectory);
    mmu_enable();

}

