#include <lib/types.h>
#include <lib/memory.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/heap.h>
#include <kernel/mmu.h>

mmu_directory_t *pageDirectory = MMU_PAGE_DIRECTORY_ADDRESS;
mmu_table_t *pageTable = MMU_PAGE_TABLE_ADDRESS;
mmu_table_t *pageTable2 = 0x9E000;

void mmu_handler(registers_t *r)
{

    uint32_t address;

    address = mmu_read_cr2();

    screen_puts("PAGE FAULT (");

    if (!(r->err_code & MMU_ERROR_PRESENT))
        screen_puts("present");

    if (r->err_code & MMU_ERROR_RW)
        screen_puts("read-only");

    if (r->err_code & MMU_ERROR_USER)
        screen_puts("user-mode");

    if (r->err_code & MMU_ERROR_RESERVED)
        screen_puts("reserved");

    if (r->err_code & MMU_ERROR_FETCH)
        screen_puts("fetch");

    screen_puts(") at 0x");
    screen_puts_hex(address);
    screen_puts("\n");

    PANIC("PAGE FAULT");

}

static void mmu_init_directory(mmu_directory_t *directory)
{

    uint32_t i;

    for (i = 0; i < 1024; i++)
    {

        directory->tables[i] = 0 | MMU_PAGE_DIRECTORY_FLAG_WRITEABLE;

    }

}

static void mmu_set_directory(mmu_directory_t *directory, uint32_t base, uint32_t limit, uint32_t flags)
{

    uint32_t i;

    directory->tables[0] = pageTable;
    directory->tables[0] = directory->tables[0] | flags;

    for (i = 0; i < 1024; i++)
    {

        pageTable->entries[i] = base | 3;
        base = base + 4096;

    }

    directory->tables[1] = pageTable2;
    directory->tables[1] = directory->tables[1] | flags;

    for (i = 0; i < 1024; i++)
    {

        pageTable2->entries[i] = base | 3;
        base = base + 4096;

    }

}

void mmu_init()
{

    mmu_init_directory(pageDirectory);
    mmu_set_directory(pageDirectory, 0x0, 0x100000, MMU_PAGE_DIRECTORY_FLAG_PRESENT | MMU_PAGE_DIRECTORY_FLAG_WRITEABLE);
    mmu_flush(pageDirectory);

}

