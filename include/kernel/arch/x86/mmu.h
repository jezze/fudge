#ifndef ARCH_MMU_H
#define ARCH_MMU_H

#define MMU_DIRECTORY_SLOTS 1024

#define MMU_TABLE_SLOTS 1024
#define MMU_TABLE_FLAG_PRESENT      1 << 0
#define MMU_TABLE_FLAG_WRITEABLE    1 << 1
#define MMU_TABLE_FLAG_USERMODE     1 << 2
#define MMU_TABLE_FLAG_CACHEWRITE   1 << 3
#define MMU_TABLE_FLAG_CACHEDISABLE 1 << 4
#define MMU_TABLE_FLAG_ACCESSED     1 << 5
#define MMU_TABLE_FLAG_LARGE        1 << 6
#define MMU_TABLE_FLAG_IGNORED      1 << 7

#define MMU_PAGE_SIZE 4096
#define MMU_PAGE_FLAG_PRESENT      1 << 0
#define MMU_PAGE_FLAG_WRITEABLE    1 << 1
#define MMU_PAGE_FLAG_USERMODE     1 << 2
#define MMU_PAGE_FLAG_CACHEWRITE   1 << 3
#define MMU_PAGE_FLAG_CACHEDISABLE 1 << 4
#define MMU_PAGE_FLAG_ACCESSED     1 << 5
#define MMU_PAGE_FLAG_DIRTY        1 << 6
#define MMU_PAGE_FLAG_GLOBAL       1 << 7

struct mmu_memory;

struct mmu_table
{

    unsigned int entries[MMU_TABLE_SLOTS];

} __attribute__((aligned(MMU_PAGE_SIZE)));

struct mmu_directory
{

    struct mmu_table *tables[MMU_DIRECTORY_SLOTS];

} __attribute__((aligned(MMU_PAGE_SIZE)));

struct mmu_header
{

    struct mmu_memory memory;
    struct mmu_directory directory;
    struct mmu_table table;

};

extern void mmu_init();

#endif

