#include <lib/types.h>
#include <lib/memory.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/heap.h>
#include <kernel/paging.h>

page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;

uint32_t *frames;
uint32_t framesNum;

static void paging_set_frame(uint32_t address)
{

    uint32_t frame = address / 0x1000;
    uint32_t index = frame / 32;
    uint32_t off = frame % 32;
    frames[index] |= (0x1 << off);

}

static void paging_unset_frame(uint32_t address)
{

    uint32_t frame = address / 0x1000;
    uint32_t index = frame / 32;
    uint32_t off = frame % 32;
    frames[index] &= ~(0x1 << off);

}

static uint32_t paging_test_frame(uint32_t address)
{

    uint32_t frame = address / 0x1000;
    uint32_t index = frame / 32;
    uint32_t off = frame % 32;

    return (frames[index] & (0x1 << off));

}

static uint32_t paging_find_frame()
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

static void paging_alloc_frame(page_t *page, uint8_t kernel, uint8_t writeable)
{

    if (page->frame != 0)
        return;

    uint32_t index = paging_find_frame();

    if (index == (uint32_t)-1)
        PANIC("No frames free");

    paging_set_frame(index * 0x1000);
    page->present = 1;
    page->rw = (writeable) ? 1 : 0;
    page->user = (kernel) ? 0 : 1;
    page->frame = index;

}

static void paging_free_frame(page_t *page)
{

    uint32_t frame;

    if (!(frame = page->frame))
        return;

    paging_unset_frame(frame);
    page->frame = 0;

}

void paging_set_directory(page_directory_t *directory)
{

    current_directory = directory;

    __asm__ __volatile__ ("mov %0, %%cr3" : : "r" (&directory->tablesPhysical));

    uint32_t cr0;

    __asm__ __volatile__ ("mov %%cr0, %0" : "=r" (cr0));

    cr0 |= 0x80000000;

    __asm__ __volatile__ ("mov %0, %%cr0" : : "r" (cr0));

}

static page_t *paging_get_page(uint32_t address, uint8_t make, page_directory_t *directory)
{

    address /= 0x1000;

    uint32_t index = address / 1024;

    if (directory->tables[index])
        return &directory->tables[index]->pages[address % 1024];

    if (make)
    {

        uint32_t tmp;

        directory->tables[index] = (page_table_t *)kmalloc_physical_aligned(sizeof (page_table_t), &tmp);
        memory_set(directory->tables[index], 0, 0x1000);
        directory->tablesPhysical[index] = tmp | 0x7;

        return &directory->tables[index]->pages[address % 1024];

    }

    return 0;

}

void paging_handler(registers_t *r)
{

    uint32_t address;

    __asm__ __volatile__ ("mov %%cr2, %0" : "=r" (address));

    screen_puts(&screen, "PAGE FAULT (");

    if (!(r->err_code & PAGING_ERROR_PRESENT))
        screen_puts(&screen, "present");

    if (r->err_code & PAGING_ERROR_RW)
        screen_puts(&screen, "read-only");

    if (r->err_code & PAGING_ERROR_USER)
        screen_puts(&screen, "user-mode");

    if (r->err_code & PAGING_ERROR_RESERVED)
        screen_puts(&screen, "reserved");

    if (r->err_code & PAGING_ERROR_FETCH)
        screen_puts(&screen, "fetch");

    screen_puts(&screen, ") at 0x");
    screen_puts_hex(&screen, address);
    screen_puts(&screen, "\n");

    PANIC("PAGE FAULT");

}

static void paging_init_frames(uint32_t size)
{

    framesNum = size / 0x1000;
    frames = (uint32_t *)kmalloc(framesNum / 32);

    memory_set(frames, 0, framesNum / 32);

}

static void paging_init_kernel()
{

    kernel_directory = (page_directory_t *)kmalloc_aligned(sizeof (page_directory_t));
    memory_set(kernel_directory, 0, sizeof (page_directory_t));

    uint32_t i;

    for (i = 0; i < heap_address; i += 0x1000)
        paging_alloc_frame(paging_get_page(i, 1, kernel_directory), 0, 0);

}

void paging_init(uint32_t size)
{

    paging_init_frames(size);
    paging_init_kernel();
    isr_register_handler(14, paging_handler);
    paging_set_directory(kernel_directory);

}

