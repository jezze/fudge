#include <lib/types.h>
#include <lib/memory.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/mmu.h>

mmu_directory_t *pageDirectory = (mmu_directory_t *)MMU_PAGE_DIRECTORY_ADDRESS;
mmu_table_t *pageTable = (mmu_table_t *)MMU_PAGE_TABLE_ADDRESS;
mmu_table_t *pageTable2 = (mmu_table_t *)0x9E000;

void mmu_handler(registers_t *r)
{

    unsigned int address = mmu_read_cr2();

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

    unsigned int i;

    for (i = 0; i < 1024; i++)
    {

        directory->tables[i] = (mmu_table_t *)(0 | MMU_PAGE_DIRECTORY_FLAG_WRITEABLE);

    }

}

void mmu_map(mmu_directory_t *directory, uint32_t base, uint32_t limit, uint32_t tableFlags, uint32_t entryFlags)
{
/*
    base /= 0x1000;
    limit /= 0x1000;

    uint32_t i;

    for (i = base; i < limit; i++)
    {

        directory->tables[i / 1024] = (mmu_table_t *)(MMU_PAGE_TABLE_ADDRESS + ((i / 1024) * 0x1000) | tableFlags);
        directory->tables[i / 1024]->entries[i % 1024] = (i * 0x1000) | entryFlags;

    }
*/
}

static void mmu_set_directory(mmu_directory_t *directory, uint32_t base, uint32_t limit, uint32_t flags)
{

    unsigned int i;

    directory->tables[0] = pageTable;
    directory->tables[0] = (mmu_table_t *)((uint32_t)directory->tables[0] | flags);

    for (i = 0; i < 1024; i++)
    {

        pageTable->entries[i] = base | 3;
        base = base + 0x1000;

    }

    directory->tables[1] = pageTable2;
    directory->tables[1] = (mmu_table_t *)((uint32_t)directory->tables[1] | flags);

    for (i = 0; i < 1024; i++)
    {

        pageTable2->entries[i] = base | 3;
        base = base + 0x1000;

    }

}

void mmu_init()
{

    mmu_init_directory(pageDirectory);
    mmu_set_directory(pageDirectory, 0x0, 0x400000, MMU_PAGE_DIRECTORY_FLAG_PRESENT | MMU_PAGE_DIRECTORY_FLAG_WRITEABLE);
    mmu_flush(pageDirectory);

}

