#ifndef PAGING_H
#define PAGING_H

typedef struct page
{

	uint32_t present : 1;
	uint32_t rw : 1;
	uint32_t user : 1;
	uint32_t accessed : 1;
	uint32_t dirty : 1;
	uint32_t unused : 7;
	uint32_t frame : 20;

} page_t;

typedef struct page_table
{

	page_t pages[1024];

} page_table_t;

typedef struct page_directory
{

	page_table_t *tables[1024];
	uint32_t tablesPhysical[1024];
	uint32_t physicalAddress;

} page_directory_t;

extern void paging_init();
extern void page_directory_switch(page_directory_t *dir);

#endif

