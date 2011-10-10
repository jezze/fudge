#ifndef ARCH_MMU_H
#define ARCH_MMU_H

#define MMU_ERROR_PRESENT  1 << 0
#define MMU_ERROR_RW       1 << 1
#define MMU_ERROR_USER     1 << 2
#define MMU_ERROR_RESERVED 1 << 3
#define MMU_ERROR_FETCH    1 << 4

#define MMU_DIRECTORY_SIZE    1024

#define MMU_TABLE_SIZE              1024
#define MMU_TABLE_FLAG_PRESENT      1 << 0
#define MMU_TABLE_FLAG_WRITEABLE    1 << 1
#define MMU_TABLE_FLAG_USERMODE     1 << 2
#define MMU_TABLE_FLAG_CACHEWRITE   1 << 3
#define MMU_TABLE_FLAG_CACHEDISABLE 1 << 4
#define MMU_TABLE_FLAG_ACCESSED     1 << 5
#define MMU_TABLE_FLAG_LARGE        1 << 6
#define MMU_TABLE_FLAG_IGNORED      1 << 7

#define MMU_PAGE_SIZE              4096
#define MMU_PAGE_FLAG_PRESENT      1 << 0
#define MMU_PAGE_FLAG_WRITEABLE    1 << 1
#define MMU_PAGE_FLAG_USERMODE     1 << 2
#define MMU_PAGE_FLAG_CACHEWRITE   1 << 3
#define MMU_PAGE_FLAG_CACHEDISABLE 1 << 4
#define MMU_PAGE_FLAG_ACCESSED     1 << 5
#define MMU_PAGE_FLAG_DIRTY        1 << 6
#define MMU_PAGE_FLAG_GLOBAL       1 << 7

struct isr_registers;

struct mmu_table
{

    unsigned int entries[MMU_TABLE_SIZE];

} __attribute__((aligned(MMU_PAGE_SIZE)));

struct mmu_directory
{

    struct mmu_table *tables[MMU_DIRECTORY_SIZE];

} __attribute__((aligned(MMU_PAGE_SIZE)));

struct mmu_header
{

    struct mmu_directory directory;
    struct mmu_table table;

};

extern void mmu_init();
extern void mmu_set_directory(struct mmu_directory *directory);
extern void mmu_map(struct mmu_directory *directory, struct mmu_table *table, unsigned int virtualAddress, unsigned int physicalAddress, unsigned int size, unsigned int tableFlags, unsigned int pageFlags);
extern void mmu_enable();
extern struct mmu_directory *mmu_get_kernel_directory();
extern struct mmu_table *mmu_get_kernel_table();
extern struct mmu_table *mmu_get_program_table();
extern void *mmu_get_slot();

#endif

