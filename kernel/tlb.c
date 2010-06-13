#include <lib/types.h>
#include <lib/memory.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/heap.h>
#include <kernel/tlb.h>

tlb_directory_t *kernel_directory = 0;
tlb_directory_t *current_directory = 0;

uint32_t *frames;
uint32_t framesNum;

static void tlb_set_frame(uint32_t address)
{

    uint32_t frame = address / TLB_FRAME_SIZE;
    uint32_t index = frame / 32;
    uint32_t off = frame % 32;
    frames[index] |= (0x1 << off);

}

static void tlb_unset_frame(uint32_t address)
{

    uint32_t frame = address / TLB_FRAME_SIZE;
    uint32_t index = frame / 32;
    uint32_t off = frame % 32;
    frames[index] &= ~(0x1 << off);

}

static uint32_t tlb_test_frame(uint32_t address)
{

    uint32_t frame = address / TLB_FRAME_SIZE;
    uint32_t index = frame / 32;
    uint32_t off = frame % 32;

    return (frames[index] & (0x1 << off));

}

static uint32_t tlb_find_frame()
{

    uint32_t i, j;

    for (i = 0; i < (framesNum / 32); i++)
    {

        if (frames[i] != 0xFFFFFFFF)
        {

            for (j = 0; j < 32; j++)
            {

                if (!(frames[i] & (0x1 << j)))
                    return (i * 32 + j);

            }

        }

    }

    return -1;

}

static void tlb_alloc_frame(tlb_page_t *page, uint8_t kernel, uint8_t writeable)
{

    if (page->frame)
        return;

    uint32_t index = tlb_find_frame();

    if (index == (uint32_t)-1)
        PANIC("No frames free");

    tlb_set_frame(index * TLB_FRAME_SIZE);
    page->present = 1;
    page->rw = (writeable) ? 1 : 0;
    page->user = (kernel) ? 0 : 1;
    page->frame = index;

}

static void tlb_free_frame(tlb_page_t *page)
{

    uint32_t frame;

    if (!(frame = page->frame))
        return;

    tlb_unset_frame(frame);
    page->frame = 0;

}

void tlb_set_directory(tlb_directory_t *directory)
{

    current_directory = directory;

    tlb_flush(directory->tablesPhysical);

}

static tlb_page_t *tlb_get_page(uint32_t address, uint8_t make, tlb_directory_t *directory)
{

    address /= TLB_FRAME_SIZE;

    uint32_t index = address / 1024;

    if (directory->tables[index])
        return &directory->tables[index]->pages[address % 1024];

    if (make)
    {

        uint32_t tmp;

        directory->tables[index] = (tlb_table_t *)kmalloc_physical_aligned(sizeof (tlb_table_t), &tmp);
        memory_set(directory->tables[index], 0, TLB_FRAME_SIZE);
        directory->tablesPhysical[index] = tmp | 0x7;

        return &directory->tables[index]->pages[address % 1024];

    }

    return 0;

}

void tlb_handler(registers_t *r)
{

    uint32_t address;

    __asm__ __volatile__ ("mov %%cr2, %0" : "=r" (address));

    screen_puts("PAGE FAULT (");

    if (!(r->err_code & TLB_ERROR_PRESENT))
        screen_puts("present");

    if (r->err_code & TLB_ERROR_RW)
        screen_puts("read-only");

    if (r->err_code & TLB_ERROR_USER)
        screen_puts("user-mode");

    if (r->err_code & TLB_ERROR_RESERVED)
        screen_puts("reserved");

    if (r->err_code & TLB_ERROR_FETCH)
        screen_puts("fetch");

    screen_puts(") at 0x");
    screen_puts_hex(address);
    screen_puts("\n");

    PANIC("PAGE FAULT");

}

static void tlb_init_frames(uint32_t size)
{

    framesNum = size / TLB_FRAME_SIZE;
    frames = (uint32_t *)kmalloc(framesNum / 32);

    memory_set(frames, 0, framesNum / 32);

}

static void tlb_init_kernel()
{

    kernel_directory = (tlb_directory_t *)kmalloc_aligned(sizeof (tlb_directory_t));
    memory_set(kernel_directory, 0, sizeof (tlb_directory_t));

    uint32_t i;

    for (i = 0; i < heap_address; i += TLB_FRAME_SIZE)
        tlb_alloc_frame(tlb_get_page(i, 1, kernel_directory), 0, 0);

}

void tlb_init(uint32_t size)
{

    tlb_init_frames(size);
    tlb_init_kernel();
    tlb_set_directory(kernel_directory);

}

