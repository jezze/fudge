#include <lib/types.h>
#include <lib/mem.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/kernel.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/heap.h>
#include <kernel/paging.h>

page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;

uint32_t *frames;
uint32_t framesNum;

extern uint32_t heap_address;

static void frame_set(uint32_t address)
{

    uint32_t frame = address / 0x1000;
    uint32_t index = frame / 32;
    uint32_t off = frame % 32;
    frames[index] |= (0x1 << off);

}

static void frame_unset(uint32_t address)
{

    uint32_t frame = address / 0x1000;
    uint32_t index = frame / 32;
    uint32_t off = frame % 32;
    frames[index] &= ~(0x1 << off);

}

static uint32_t frame_test(uint32_t address)
{

    uint32_t frame = address / 0x1000;
    uint32_t index = frame / 32;
    uint32_t off = frame % 32;

    return (frames[index] & (0x1 << off));

}

static uint32_t frame_find()
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

void frame_alloc(page_t *page, int kernel, int writeable)
{

    if (page->frame != 0)
        return;

    uint32_t index = frame_find();

    if (index == (uint32_t)-1)
        PANIC("No frames free");

    frame_set(index * 0x1000);
    page->present = 1;
    page->rw = (writeable) ? 1 : 0;
    page->user = (kernel) ? 0 : 1;
    page->frame = index;

}

void frame_free(page_t *page)
{

    uint32_t frame;

    if (!(frame = page->frame))
        return;

    frame_unset(frame);
    page->frame = 0;

}

void page_directory_switch(page_directory_t *dir)
{

    current_directory = dir;

    __asm__ __volatile__ ("mov %0, %%cr3" : : "r" (&dir->tablesPhysical));

    uint32_t cr0;

    __asm__ __volatile__ ("mov %%cr0, %0" : "=r" (cr0));

    cr0 |= 0x80000000;

    __asm__ __volatile__ ("mov %0, %%cr0" : : "r" (cr0));

}

page_t *page_get(uint32_t address, int make, page_directory_t *dir)
{

    address /= 0x1000;

    uint32_t index = address / 1024;

    if (dir->tables[index])
        return &dir->tables[index]->pages[address % 1024];

    if (make)
    {

        uint32_t tmp;

        dir->tables[index] = (page_table_t *)kmalloc_physical_aligned(sizeof (page_table_t), &tmp);
        memset(dir->tables[index], 0, 0x1000);
        dir->tablesPhysical[index] = tmp | 0x7;

        return &dir->tables[index]->pages[address % 1024];

    }

    return 0;

}

void paging_handler(registers_t *r)
{

    uint32_t address;

    __asm__ __volatile__ ("mov %%cr2, %0" : "=r" (address));

    int present = !(r->err_code & PAGING_ERROR_PRESENT);
    int rw = r->err_code & PAGING_ERROR_RW;
    int us = r->err_code & PAGING_ERROR_USER;
    int reserved = r->err_code & PAGING_ERROR_RESERVED;
    int fetch = r->err_code & PAGING_ERROR_FETCH;

    screen_puts(&screen, "PAGE FAULT (");

    if (present)
        screen_puts(&screen, "present");

    if (rw)
        screen_puts(&screen, "read-only");

    if (us)
        screen_puts(&screen, "user-mode");

    if (reserved)
        screen_puts(&screen, "reserved");

    if (fetch)
        screen_puts(&screen, "fetch");

    screen_puts(&screen, ") at ");
    screen_puts_hex(&screen, address);
    screen_puts(&screen, "\n");

    PANIC("PAGE FAULT");

}

void frame_init()
{

    uint32_t i;

    for (i = 0; i < heap_address; i += 0x1000)
        frame_alloc(page_get(i, 1, kernel_directory), 0, 0);

}

void paging_init()
{

    uint32_t mem_end_page = 0x1000000;

    framesNum = mem_end_page / 0x1000;
    frames = (uint32_t *)kmalloc(framesNum / 32);
    memset(frames, 0, framesNum / 32);
    kernel_directory = (page_directory_t *)kmalloc_aligned(sizeof (page_directory_t));
    memset(kernel_directory, 0, sizeof (page_directory_t));
    frame_init();
    isr_register_handler(14, paging_handler);
    page_directory_switch(kernel_directory);

}

