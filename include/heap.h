#ifndef HEAP_H
#define HEAP_H

typedef struct
{

	uint32_t magic;
	uint8_t is_hole;
	uint32_t size;

} header_t;

typedef struct
{

	uint32_t magic;
	header_t *header;

} footer_t;

extern uint32_t kmalloc_page(uint32_t, int, uint32_t *);
extern uint32_t kmalloc(uint32_t);
extern uint32_t kmalloc_aligned(uint32_t);
extern uint32_t kmalloc_physical(uint32_t, uint32_t *);
extern uint32_t kmalloc_physical_aligned(uint32_t, uint32_t *);

#endif

