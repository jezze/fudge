#ifndef KERNEL_MMU_H
#define KERNEL_MMU_H

#define MMU_ERROR_PRESENT  1 << 0
#define MMU_ERROR_RW       1 << 1
#define MMU_ERROR_USER     1 << 2
#define MMU_ERROR_RESERVED 1 << 3
#define MMU_ERROR_FETCH    1 << 4

struct mmu_memory
{

    void *paddress;
    void *vaddress;
    unsigned int size;

};

struct mmu_unit
{

    void (*load_memory)(struct mmu_memory *memory);
    void (*map_kernel_memory)(struct mmu_memory *memory);
    void (*map_user_memory)(struct mmu_memory *memory);
    void (*unmap_memory)(struct mmu_memory *memory);

};

extern void mmu_pagefault(unsigned int address, unsigned int flags);
extern struct mmu_memory *mmu_get_task_memory();
extern void mmu_load_memory(struct mmu_memory *memory);
extern void mmu_map_kernel_memory(struct mmu_memory *memory);
extern void mmu_map_user_memory(struct mmu_memory *memory);
extern void mmu_unmap_memory(struct mmu_memory *memory);
extern void mmu_register_unit(struct mmu_unit *unit);
extern void mmu_memory_init(struct mmu_memory *memory, void *paddress, void *vaddress, unsigned int size);

#endif

