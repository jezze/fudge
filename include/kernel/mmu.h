#ifndef MMU_H
#define MMU_H

#define MMU_ERROR_PRESENT          0x01
#define MMU_ERROR_RW               0x02
#define MMU_ERROR_USER             0x04
#define MMU_ERROR_RESERVED         0x08
#define MMU_ERROR_FETCH            0x16
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

extern void mmu_flush(uint32_t *directory);
extern uint32_t mmu_read_cr0();
extern void mmu_write_cr0(uint32_t value);
extern uint32_t mmu_read_cr2();
extern uint32_t mmu_read_cr3();
extern void mmu_write_cr3(uint32_t value);

extern void mmu_handler(registers_t *r);
extern void mmu_init();

#endif

