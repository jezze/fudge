#include <lib/assert.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/session.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/cr.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/mmu.h>

void mmu_handler(struct isr_registers *registers)
{

    unsigned int address = cr2_read();

    file_write(session_get_out(), "PAGE FAULT (");

    if (!(registers->err_code & MMU_ERROR_PRESENT))
        file_write(session_get_out(), "present");

    if (registers->err_code & MMU_ERROR_RW)
        file_write(session_get_out(), "read-only");

    if (registers->err_code & MMU_ERROR_USER)
        file_write(session_get_out(), "user-mode");

    if (registers->err_code & MMU_ERROR_RESERVED)
        file_write(session_get_out(), "reserved");

    if (registers->err_code & MMU_ERROR_FETCH)
        file_write(session_get_out(), "fetch");

    file_write(session_get_out(), ") at 0x");
    file_write_hex(session_get_out(), address);
    file_write(session_get_out(), "\n");

    PANIC("PAGE FAULT");

}

static void mmu_init_directory(struct mmu_directory *directory)
{

    unsigned int i;

    for (i = 0; i < 1024; i++)
        directory->tables[i] = (struct mmu_table *)(0 | MMU_DIRECTORY_FLAG_WRITEABLE);

}

static void mmu_set_directory(struct mmu_directory *directory, unsigned int base, unsigned int limit, unsigned int flags)
{

    unsigned int i;

    struct mmu_table *pageTable = (struct mmu_table *)MMU_TABLE_ADDRESS;

    directory->tables[0] = pageTable;
    directory->tables[0] = (struct mmu_table *)((unsigned int)directory->tables[0] | flags);

    for (i = 0; i < 1024; i++, base += 0x1000)
        pageTable->entries[i] = base | 3;

}

void mmu_init()
{

    struct mmu_directory *directory = (struct mmu_directory *)MMU_DIRECTORY_ADDRESS;

    mmu_init_directory(directory);
    mmu_set_directory(directory, 0x0, 0x400000, MMU_DIRECTORY_FLAG_PRESENT | MMU_DIRECTORY_FLAG_WRITEABLE);

    cr3_write((unsigned int)directory);
    cr0_write(cr0_read() | 0x80000000);

}

