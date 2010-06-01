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

static void frame_set(uint32_t frame_address)
{

    uint32_t frame = frame_address / 0x1000;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    frames[idx] |= (0x1 << off);

}

static void frame_unset(uint32_t frame_address)
{

    uint32_t frame = frame_address / 0x1000;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    frames[idx] &= ~(0x1 << off);

}

static uint32_t frame_test(uint32_t frame_address)
{

    uint32_t frame = frame_address / 0x1000;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;

    return (frames[idx] & (0x1 << off));

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

void frame_alloc(page_t *page, int is_kernel, int is_writeable)
{

    if (page->frame != 0)
        return;

    uint32_t idx = frame_find();

    if (idx == (uint32_t)-1)
        PANIC("No frames free");

    frame_set(idx * 0x1000);
    page->present = 1;
    page->rw = (is_writeable) ? 1 : 0;
    page->user = (is_kernel) ? 0 : 1;
    page->frame = idx;

}

void frame_free(page_t *page)
{

    uint32_t frame;

    if (!(frame = page->frame))
        return;

    frame_unset(frame);
    page->frame = 0x0;

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

    uint32_t table_idx = address / 1024;

    if (dir->tables[table_idx])
    {

        return &dir->tables[table_idx]->pages[address % 1024];

    }

    else if (make)
    {

        uint32_t tmp;

        dir->tables[table_idx] = (page_table_t *)kmalloc_physical_aligned(sizeof (page_table_t), &tmp);
        memset(dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = tmp | 0x7;

        return &dir->tables[table_idx]->pages[address % 1024];

    }

    else
    {

        return 0;

    }

}

void paging_handler(registers_t *r)
{

    uint32_t address;

    __asm__ __volatile__ ("mov %%cr2, %0" : "=r" (address));

    int present = !(r->err_code & 0x1);
    int rw = r->err_code & 0x2;
    int us = r->err_code & 0x4;
    int reserved = r->err_code & 0x8;
    int fetch = r->err_code & 0x10;

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

    uint32_t i = 0;

    while (i < heap_address)
    {

        frame_alloc(page_get(i, 1, kernel_directory), 0, 0);
        i += 0x1000;

    }

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

