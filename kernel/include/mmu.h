#ifndef KERNEL_MMU_H
#define KERNEL_MMU_H

#define MMU_ERROR_PRESENT  1 << 0
#define MMU_ERROR_RW       1 << 1
#define MMU_ERROR_USER     1 << 2
#define MMU_ERROR_RESERVED 1 << 3
#define MMU_ERROR_FETCH    1 << 4

struct mmu_unit
{

    void (*enable)();
    void (*load_memory)(unsigned int index);
    void (*reload_memory)();
    void (*map_kernel_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);
    void (*map_user_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);
    void (*unmap_memory)(unsigned int index);

};

void mmu_pagefault(unsigned int index, unsigned int address, unsigned int flags);
struct mmu_memory *mmu_get_task_memory();
void mmu_load_memory(unsigned int index);
void mmu_reload_memory();
void mmu_map_kernel_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);
void mmu_map_user_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);
void mmu_unmap_memory(unsigned int index);
void mmu_unit_init(struct mmu_unit *unit, void (*enable)(), void (*load_memory)(unsigned int index), void (*reload_memory)(), void (*map_kernel_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size), void (*map_user_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size), void (*unmap_memory)(unsigned int index));
void mmu_init(struct mmu_unit *unit);

#endif

