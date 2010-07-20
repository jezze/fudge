#include <lib/memory.h>
#include <lib/vfs.h>
#include <kernel/arch/i386/arch.h>
#include <kernel/arch/i386/cr.h>
#include <kernel/arch/i386/isr.h>
#include <kernel/arch/i386/mmu.h>
#include <kernel/assert.h>

void mmu_handler(struct isr_registers *registers)
{

    unsigned int address = cr2_read();

    arch_puts("PAGE FAULT (");

    if (!(registers->err_code & MMU_ERROR_PRESENT))
        arch_puts("present");

    if (registers->err_code & MMU_ERROR_RW)
        arch_puts("read-only");

    if (registers->err_code & MMU_ERROR_USER)
        arch_puts("user-mode");

    if (registers->err_code & MMU_ERROR_RESERVED)
        arch_puts("reserved");

    if (registers->err_code & MMU_ERROR_FETCH)
        arch_puts("fetch");

    arch_puts(") at 0x");
    arch_puts_hex(address);
    arch_puts("\n");

    PANIC("PAGE FAULT");

}

static void mmu_init_directory(struct mmu_directory *directory)
{

    unsigned int i;

    for (i = 0; i < 1024; i++)
        directory->tables[i] = (struct mmu_table *)(0 | MMU_PAGE_DIRECTORY_FLAG_WRITEABLE);

}

static void mmu_set_directory(struct mmu_directory *directory, unsigned int base, unsigned int limit, unsigned int flags)
{

    unsigned int i;

    struct mmu_table *pageTable = (struct mmu_table *)MMU_PAGE_TABLE_ADDRESS;

    directory->tables[0] = pageTable;
    directory->tables[0] = (struct mmu_table *)((unsigned int)directory->tables[0] | flags);

    for (i = 0; i < 1024; i++, base += 0x1000)
        pageTable->entries[i] = base | 3;

}

void mmu_init()
{

    struct mmu_directory *directory = (struct mmu_directory *)MMU_PAGE_DIRECTORY_ADDRESS;

    mmu_init_directory(directory);
    mmu_set_directory(directory, 0x0, 0x400000, MMU_PAGE_DIRECTORY_FLAG_PRESENT | MMU_PAGE_DIRECTORY_FLAG_WRITEABLE);

    cr3_write((unsigned int)directory);
    cr0_write(cr0_read() | 0x80000000);

}

