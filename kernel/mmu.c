#include <lib/types.h>
#include <lib/memory.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/heap.h>
#include <kernel/mmu.h>

mmu_directory_t *kernel_directory = 0;
mmu_directory_t *current_directory = 0;

uint32_t *frames;
uint32_t framesNum;

static void mmu_set_bit(uint32_t address)
{

    uint32_t index = address / 32;
    uint32_t offset = address % 32;
    frames[index] |= (0x1 << offset);

}

static void mmu_unset_bit(uint32_t address)
{

    uint32_t index = address / 32;
    uint32_t offset = address % 32;
    frames[index] &= ~(0x1 << offset);

}

static uint32_t mmu_test_bit(uint32_t address)
{

    uint32_t index = address / 32;
    uint32_t offset = address % 32;

    return (frames[index] & (0x1 << offset));

}

static uint32_t mmu_find_frame()
{

    uint32_t i, j;

    for (i = 0; i < framesNum; i++)
    {

        if (frames[i] == 0xFFFFFFFF)
            continue;

        for (j = 0; j < 32; j++)
        {

            if (!(frames[i] & (0x1 << j)))
                return (i * 32 + j);

        }

    }

    return -1;

}

static void mmu_alloc_frame(mmu_page_t *page, uint8_t usermode, uint8_t writeable)
{

    if (page->frame)
        return;

    uint32_t index = mmu_find_frame();

    if (index == (uint32_t)-1)
        PANIC("No frames free");

    mmu_set_bit(index);
    page->present = 1;
    page->writeable = writeable;
    page->usermode = usermode;
    page->frame = index;

}

static void mmu_free_frame(mmu_page_t *page)
{

    uint32_t frame;

    if (!(frame = page->frame))
        return;

    mmu_unset_bit(frame);
    page->frame = 0;

}

void mmu_set_directory(mmu_directory_t *directory)
{

    current_directory = directory;

    mmu_flush(directory->tablesPhysical);

}

static mmu_page_t *mmu_get_page(uint32_t address, uint8_t make, mmu_directory_t *directory)
{

    address /= MMU_FRAME_SIZE;

    uint32_t tableIndex = address / 1024;
    uint32_t pageIndex = address % 1024;

    if (directory->tables[tableIndex])
        return &directory->tables[tableIndex]->pages[pageIndex];

    if (make)
    {

        uint32_t tmp;

        directory->tables[tableIndex] = (mmu_table_t *)kmalloc_physical_aligned(sizeof (mmu_table_t), &tmp);
        memory_set(directory->tables[tableIndex], 0, MMU_FRAME_SIZE);
        directory->tablesPhysical[tableIndex] = tmp | 0x7;

        return &directory->tables[tableIndex]->pages[pageIndex];

    }

    return 0;

}

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

static void mmu_init_frames(uint32_t size)
{

    framesNum = size / MMU_FRAME_SIZE;
    frames = (uint32_t *)kmalloc(framesNum);

    memory_set(frames, 0, framesNum);

}

static void mmu_init_kernel()
{

    kernel_directory = (mmu_directory_t *)kmalloc_aligned(sizeof (mmu_directory_t));
    memory_set(kernel_directory, 0, sizeof (mmu_directory_t));

    uint32_t i;

    for (i = 0; i < heap_address; i += MMU_FRAME_SIZE)
        mmu_alloc_frame(mmu_get_page(i, 1, kernel_directory), 1, 0);

}

void mmu_init(uint32_t size)
{

    mmu_init_frames(size);
    mmu_init_kernel();
    mmu_set_directory(kernel_directory);

}

