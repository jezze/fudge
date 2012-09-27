#ifndef KERNEL_MMU_H
#define KERNEL_MMU_H

#define MMU_INTERFACE_SLOTS     2
#define MMU_ERROR_PRESENT  1 << 0
#define MMU_ERROR_RW       1 << 1
#define MMU_ERROR_USER     1 << 2
#define MMU_ERROR_RESERVED 1 << 3
#define MMU_ERROR_FETCH    1 << 4

struct mmu_interface
{

    void (*enable)();
    void (*load_memory)(unsigned int index);
    void (*reload_memory)();
    void (*map_kernel_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);
    void (*map_user_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);

};

void mmu_pagefault(unsigned int address, unsigned int flags);
void mmu_load_memory(unsigned int index);
void mmu_reload_memory();
void mmu_map_kernel_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);
void mmu_map_user_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size);
void mmu_set_interface(unsigned int index, void (*enable)(), void (*load_memory)(unsigned int index), void (*reload_memory)(), void (*map_kernel_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size), void (*map_user_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size));

#endif

