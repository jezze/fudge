#include <lib/types.h>
#include <lib/memory.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/heap.h>
#include <kernel/mmu.h>

mmu_directory_entry_t *directoryTable = 0x00100000;
mmu_page_entry_t *pageTable = 0x00200000;

void mmu_handler(registers_t *r)
{

    uint32_t address;

    __asm__ __volatile__ ("mov %%cr2, %0" : "=r" (address));

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

static void mmu_init_page_entry(mmu_page_entry_t *entry, uint32_t address)
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

static void mmu_init_directory_table()
{

    int i, j ;

    for (i = 0; i < 1024; i++)
    {

        mmu_init_directory_entry(directoryTable, pageTable + i);

        for (j = 0; j < 1024; j++)
        {

            mmu_init_page_entry(pageTable + i, i * j * 0x1000);

        }


    }

}

void mmu_init(uint32_t size)
{

    mmu_init_directory_table();
    mmu_flush(directoryTable);

}

