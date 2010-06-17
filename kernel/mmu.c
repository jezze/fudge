#include <lib/types.h>
#include <lib/memory.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/heap.h>
#include <kernel/mmu.h>

uint32_t *pageDirectory = MMU_PAGE_DIRECTORY_ADDRESS;
uint32_t *pageTable = MMU_PAGE_TABLE_ADDRESS;

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

static void mmu_init_table_entry(mmu_table_entry_t *entry, uint32_t address)
{

    entry->present = 1;
    entry->writeable = 1;
    entry->usermode = 0;
    entry->cacheWritethrough = 0;
    entry->cacheDisabled = 0;
    entry->accessed = 0;
    entry->dirty = 0;
    entry->zero = 0;
    entry->global = 0;
    entry->unused = 0;
    entry->frame = address;

}

static void mmu_init_directory_entry(mmu_directory_entry_t *entry, uint32_t address)
{

    entry->present = 1;
    entry->writeable = 1;
    entry->usermode = 0;
    entry->cacheWritethrough = 0;
    entry->cacheDisabled = 0;
    entry->accessed = 0;
    entry->zero = 0;
    entry->large = 0;
    entry->ignored = 0;
    entry->unused = 0;
    entry->table = address;

}

static void mmu_init_directory()
{

    uint32_t address = 0;

    uint32_t i;

    for (i = 0; i < 1024; i++)
    {

        pageTable[i] = address | 3;
        address = address + 4096;

    }

    pageDirectory[0] = pageTable;
    pageDirectory[0] = pageDirectory[0] | 3;

    for (i = 1; i < 1024; i++)
    {

        pageDirectory[i] = 0 | 2;

    }

}

void mmu_init(uint32_t size)
{

    mmu_init_directory();
    mmu_flush(pageDirectory);

}

