#ifndef PAGING_H
#define PAGING_H

#define PAGING_FRAME_SIZE     0x1000
#define PAGING_ERROR_PRESENT  0x01
#define PAGING_ERROR_RW       0x02
#define PAGING_ERROR_USER     0x04
#define PAGING_ERROR_RESERVED 0x08
#define PAGING_ERROR_FETCH    0x16

typedef struct paging_page
{

    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t unused : 7;
    uint32_t frame : 20;

} paging_page_t;

typedef struct paging_table
{

    paging_page_t pages[1024];

} paging_table_t;

typedef struct paging_directory
{

    paging_table_t *tables[1024];
    uint32_t tablesPhysical[1024];
    uint32_t physicalAddress;

} paging_directory_t;

extern void paging_init();
extern void paging_set_directory(paging_directory_t *directory);
extern void paging_handler(registers_t *registers);

#endif

