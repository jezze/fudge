#ifndef MMU_H
#define MMU_H

#define MMU_ERROR_PRESENT       0x01
#define MMU_ERROR_RW            0x02
#define MMU_ERROR_USER          0x04
#define MMU_ERROR_RESERVED      0x08
#define MMU_ERROR_FETCH         0x16
#define MMU_PAGE_SIZE           0x1000
#define MMU_PAGE_DIRECTORY_SIZE 1024
#define MMU_PAGE_TABLE_SIZE     1024

typedef struct mmu_table_entry
{

    uint32_t present           : 1;
    uint32_t writeable         : 1;
    uint32_t usermode          : 1;
    uint32_t cacheWritethrough : 1;
    uint32_t cacheDisabled     : 1;
    uint32_t accessed          : 1;
    uint32_t dirty             : 1;
    uint32_t zero              : 1;
    uint32_t global            : 1;
    uint32_t unused            : 3;
    uint32_t frame             : 20;

} mmu_table_entry_t;

typedef struct mmu_table
{

    mmu_table_entry_t *entries[MMU_PAGE_TABLE_SIZE];

} mmu_table_t;

typedef struct mmu_directory_entry
{

    uint32_t present           : 1;
    uint32_t writeable         : 1;
    uint32_t usermode          : 1;
    uint32_t cacheWritethrough : 1;
    uint32_t cacheDisabled     : 1;
    uint32_t accessed          : 1;
    uint32_t zero              : 1;
    uint32_t large             : 1;
    uint32_t ignored           : 1;
    uint32_t unused            : 3;
    uint32_t table             : 20;

} mmu_directory_entry_t;

typedef struct mmu_directory
{

    mmu_directory_entry_t *entries[MMU_PAGE_DIRECTORY_SIZE];

} mmu_directory_t;

extern void mmu_flush(mmu_directory_entry_t *directory);

extern void mmu_handler(registers_t *r);
extern void mmu_init();

#endif

