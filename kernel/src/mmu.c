#include <error.h>
#include <runtime.h>
#include <mmu.h>

static struct mmu_interface *interfaces[MMU_INTERFACE_SLOTS];

void mmu_pagefault(unsigned int address, unsigned int flags)
{

    error_register(0, address);
    error_register(1, flags);
    error_panic("PAGE FAULT", __FILE__, __LINE__);

}

void mmu_load_user_memory(unsigned int index)
{

    interfaces[0]->load_user_memory(index);

}

void mmu_reload_memory()
{

    interfaces[0]->reload_memory();

}

void mmu_map_kernel_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    interfaces[0]->map_kernel_memory(index, paddress, vaddress, size);

}

void mmu_map_user_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    interfaces[0]->map_user_memory(index, paddress, vaddress, size);

}

void mmu_unmap_user_memory(unsigned int index)
{

    interfaces[0]->unmap_user_memory(index);

}

void mmu_register_interface(struct mmu_interface *interface)
{

    error_assert(interface != 0, "MMU not found", __FILE__, __LINE__);

    interfaces[0] = interface;
    interfaces[0]->map_kernel_memory(0, 0x00000000, 0x00000000, 0x00400000);
    interfaces[0]->map_user_memory(1, RUNTIME_TASK_PADDRESS_BASE + 1 * RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE, RUNTIME_TASK_ADDRESS_SIZE);
    interfaces[0]->load_user_memory(1);
    interfaces[0]->enable();

}

void mmu_interface_init(struct mmu_interface *interface, void (*enable)(), void (*load_user_memory)(unsigned int index), void (*reload_memory)(), void (*map_kernel_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size), void (*map_user_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size), void (*unmap_user_memory)(unsigned int index))
{

    interface->enable = enable;
    interface->load_user_memory = load_user_memory;
    interface->reload_memory = reload_memory;
    interface->map_kernel_memory = map_kernel_memory;
    interface->map_user_memory = map_user_memory;
    interface->unmap_user_memory = unmap_user_memory;

}

