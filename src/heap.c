#include <system.h>
#include <heap.h>

uint32_t heap_address;

uint32_t kmalloc_page(uint32_t size, int align, uint32_t *physical)
{

	if (align == 1 && (heap_address & 0xFFFFF000))
	{

		heap_address &= 0xFFFFF000;
		heap_address += 0x1000;

	}

	if (physical)
		*physical = heap_address;

	uint32_t tmp = heap_address;
	heap_address += size;

	return tmp;

}

uint32_t kmalloc(uint32_t size)
{

	return kmalloc_page(size, 0, 0);

}

uint32_t kmalloc_aligned(uint32_t size)
{

	return kmalloc_page(size, 1, 0);

}

uint32_t kmalloc_physical(uint32_t size, uint32_t *physical)
{

	return kmalloc_page(size, 0, physical);

}

uint32_t kmalloc_physical_aligned(uint32_t size, uint32_t *physical)
{

	return kmalloc_page(size, 1, physical);

}

void heap_init(uint32_t address)
{

	heap_address = address;

}

