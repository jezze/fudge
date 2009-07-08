#include <system.h>
#include <mem.h>
#include <isr.h>
#include <heap.h>
#include <screen.h>
#include <paging.h>

page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;

uint32_t *frames;
uint32_t framesNum;

extern uint32_t placement_address;

static void set_frame(uint32_t frame_address)
{

	uint32_t frame = frame_address / 0x1000;
	uint32_t idx = (frame / (8 * 4));
	uint32_t off = (frame % (8 * 4));
	frames[idx] |= (0x1 << off);

}

static void clear_frame(uint32_t frame_address)
{

	uint32_t frame = frame_address / 0x1000;
	uint32_t idx = (frame / (8 * 4));
	uint32_t off = (frame % (8 * 4));
	frames[idx] &= ~(0x1 << off);

}

static uint32_t test_frame(uint32_t frame_address)
{

	uint32_t frame = frame_address / 0x1000;
	uint32_t idx = (frame / (8 * 4));
	uint32_t off = (frame % (8 * 4));

	return (frames[idx] & (0x1 << off));

}

static uint32_t first_frame()
{

	uint32_t i, j;

	for (i = 0; i < (framesNum / (8 * 4)); i++)
	{

		if (frames[i] != 0xFFFFFFFF)
		{

			for (j = 0; j < 32; j++)
			{

				uint32_t toTest = 0x1 << j;

				if (!(frames[i] & toTest))
					return (i * 4 * 8 + j);

			}

		}

	}

	return -1;

}

void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{

	if (page->frame != 0)
	{

		return;

	}

	else
	{

		uint32_t idx = first_frame();

		if (idx == (uint32_t)-1)
			PANIC("No frames free");

		set_frame(idx * 0x1000);
		page->present = 1;
		page->rw = (is_writeable) ? 1 : 0;
		page->user = (is_kernel) ? 0 : 1;
		page->frame = idx;

	}

}

void free_frame(page_t *page)
{

	uint32_t frame;

	if (!(frame = page->frame))
	{

		return;

	}

	else
	{

		clear_frame(frame);
		page->frame = 0x0;

	}

}

void switch_page_directory(page_directory_t *dir)
{

	current_directory = dir;
	__asm__ __volatile__ ("mov %0, %%cr3" : : "r" (&dir->tablesPhysical));
	uint32_t cr0;
	__asm__ __volatile__ ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	__asm__ __volatile__ ("mov %0, %%cr0" : : "r" (cr0));

}

page_t *get_page(uint32_t address, int make, page_directory_t *dir)
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

void page_fault(registers_t *r)
{

	uint32_t faulting_address;

	__asm__ __volatile__ ("mov %%cr2, %0" : "=r" (faulting_address));

	int present = !(r->err_code & 0x1);
	int rw = r->err_code & 0x2;
	int us = r->err_code & 0x4;
	int reserved = r->err_code & 0x8;
	int id = r->err_code & 0x10;

	puts("PAGE FAULT ( ");

	if (present)
		puts("present ");

	if (rw)
		puts("read-only ");

	if (us)
		puts("user-mode ");

	if (reserved)
		puts("reserved ");

	if (id)
		puts("fetch");

	puts(") at 0x");
	puts_hex(faulting_address);
	puts("\n");

	PANIC("PAGE FAULT");

}

void paging_init()
{

	uint32_t mem_end_page = 0x1000000;

	framesNum = mem_end_page / 0x1000;
	frames = (uint32_t *)kmalloc((framesNum / (8 * 4)));
	memset(frames, 0, (framesNum / (8 * 4)));

	kernel_directory = (page_directory_t *)kmalloc_aligned(sizeof (page_directory_t));
	memset(kernel_directory, 0, sizeof (page_directory_t));
	current_directory = kernel_directory;

	uint32_t i = 0;

	while (i < placement_address + 0x1000)
	{

		alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
		i += 0x1000;

	}

	isr_register_handler(14, page_fault);

	switch_page_directory(kernel_directory);

}

