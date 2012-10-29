#ifndef ARCH_MMU_H
#define ARCH_MMU_H

#define MMU_TABLE_SLOTS                 1024
#define MMU_TABLE_FLAG_PRESENT          (1 << 0)
#define MMU_TABLE_FLAG_WRITEABLE        (1 << 1)
#define MMU_TABLE_FLAG_USERMODE         (1 << 2)
#define MMU_TABLE_FLAG_CACHEWRITE       (1 << 3)
#define MMU_TABLE_FLAG_CACHEDISABLE     (1 << 4)
#define MMU_TABLE_FLAG_ACCESSED         (1 << 5)
#define MMU_TABLE_FLAG_LARGE            (1 << 6)
#define MMU_TABLE_FLAG_IGNORED          (1 << 7)

#define MMU_PAGE_SIZE                   4096
#define MMU_PAGE_SLOTS                  1024
#define MMU_PAGE_FLAG_PRESENT           (1 << 0)
#define MMU_PAGE_FLAG_WRITEABLE         (1 << 1)
#define MMU_PAGE_FLAG_USERMODE          (1 << 2)
#define MMU_PAGE_FLAG_CACHEWRITE        (1 << 3)
#define MMU_PAGE_FLAG_CACHEDISABLE      (1 << 4)
#define MMU_PAGE_FLAG_ACCESSED          (1 << 5)
#define MMU_PAGE_FLAG_DIRTY             (1 << 6)
#define MMU_PAGE_FLAG_GLOBAL            (1 << 7)

#define MMU_ERROR_PRESENT               (1 << 0)
#define MMU_ERROR_RW                    (1 << 1)
#define MMU_ERROR_USER                  (1 << 2)
#define MMU_ERROR_RESERVED              (1 << 3)
#define MMU_ERROR_FETCH                 (1 << 4)

struct mmu_table
{

    void *pages[MMU_PAGE_SLOTS];

} __attribute__((aligned(MMU_PAGE_SIZE)));

struct mmu_directory
{

    struct mmu_table *tables[MMU_TABLE_SLOTS];

} __attribute__((aligned(MMU_PAGE_SIZE)));

void mmu_load_memory(unsigned int index);
void mmu_map_kernel_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);
void mmu_map_user_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);
void mmu_reload_memory();
void mmu_routine();
void mmu_setup_arch(unsigned short selector);

#endif

