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

static void mmu_init_directory(uint32_t *directory, uint32_t base, uint32_t size, uint32_t flags)
{

    uint32_t i;

    for (i = 0; i < 1024; i++)
    {

        pageTable[i] = base | 3;
        base = base + 4096;

    }

    directory[0] = pageTable;
    directory[0] = directory[0] | flags;

    for (i = 1; i < 1024; i++)
    {

        pageDirectory[i] = 0 | 2;

    }

}

void mmu_init()
{

    mmu_init_directory(pageDirectory, 0, 0x100000, MMU_PAGE_DIRECTORY_FLAG_PRESENT | MMU_PAGE_DIRECTORY_FLAG_WRITEABLE);
    mmu_flush(pageDirectory);

}

