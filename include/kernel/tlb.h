#ifndef TLB_H
#define TLB_H

#define TLB_FRAME_SIZE     0x1000
#define TLB_ERROR_PRESENT  0x01
#define TLB_ERROR_RW       0x02
#define TLB_ERROR_USER     0x04
#define TLB_ERROR_RESERVED 0x08
#define TLB_ERROR_FETCH    0x16

typedef struct tlb_page
{

    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t unused : 7;
    uint32_t frame : 20;

} tlb_page_t;

typedef struct tlb_table
{

    tlb_page_t pages[1024];

} tlb_table_t;

typedef struct tlb_directory
{

    tlb_table_t *tables[1024];
    uint32_t tablesPhysical[1024];
    uint32_t physicalAddress;

} tlb_directory_t;

extern void tlb_flush(uint32_t *tablesPhysical);

extern void tlb_set_directory(tlb_directory_t *directory);
extern void tlb_handler(registers_t *r);
extern void tlb_init();

#endif

