#ifndef MMU_H
#define MMU_H

#define MMU_ERROR_PRESENT          1
#define MMU_ERROR_RW               2
#define MMU_ERROR_USER             4
#define MMU_ERROR_RESERVED         8
#define MMU_ERROR_FETCH            16
#define MMU_PAGE_SIZE              0x1000
#define MMU_PAGE_DIRECTORY_ADDRESS 0x9C000;
#define MMU_PAGE_DIRECTORY_SIZE    1024
#define MMU_PAGE_TABLE_ADDRESS     0x9D000;
#define MMU_PAGE_TABLE_SIZE        1024

#define MMU_PAGE_DIRECTORY_FLAG_PRESENT      1
#define MMU_PAGE_DIRECTORY_FLAG_WRITEABLE    2
#define MMU_PAGE_DIRECTORY_FLAG_USERMODE     4
#define MMU_PAGE_DIRECTORY_FLAG_CACHEWRITE   8
#define MMU_PAGE_DIRECTORY_FLAG_CACHEDISABLE 16
#define MMU_PAGE_DIRECTORY_FLAG_ACCESSED     32
#define MMU_PAGE_DIRECTORY_FLAG_LARGE        128
#define MMU_PAGE_DIRECTORY_FLAG_IGNORED      256

#define MMU_PAGE_TABLE_FLAG_PRESENT      1
#define MMU_PAGE_TABLE_FLAG_WRITEABLE    2
#define MMU_PAGE_TABLE_FLAG_USERMODE     4
#define MMU_PAGE_TABLE_FLAG_CACHEWRITE   8
#define MMU_PAGE_TABLE_FLAG_CACHEDISABLE 16
#define MMU_PAGE_TABLE_FLAG_ACCESSED     32
#define MMU_PAGE_TABLE_FLAG_DIRTY        64
#define MMU_PAGE_TABLE_FLAG_GLOBAL       256

struct isr_registers;

struct mmu_table
{

    unsigned int entries[1024];

};

struct mmu_directory
{

    struct mmu_table *tables[1024];

};

extern void mmu_flush(struct mmu_directory *directory);
extern unsigned int mmu_read_cr0();
extern void mmu_write_cr0(unsigned int value);
extern unsigned int mmu_read_cr2();
extern unsigned int mmu_read_cr3();
extern void mmu_write_cr3(unsigned int value);

extern void mmu_handler(struct isr_registers *registers);
extern void mmu_init();

#endif

