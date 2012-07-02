#include <memory.h>
#include <error.h>
#include <mmu.h>
#include <runtime.h>

static struct mmu_unit *mmuUnit;

void mmu_pagefault(unsigned int index, unsigned int address, unsigned int flags)
{

    struct runtime_task *task = runtime_get_task(index);

    if (address >= task->memory.vaddress && address < task->memory.vaddress + task->memory.size)
    {

        mmu_map_user_memory(task->id, task->memory.paddress, task->memory.vaddress, task->memory.size);

        return;

    }

    error_register(0, address);
    error_register(1, flags);
    error_panic("PAGE FAULT", __FILE__, __LINE__);

}

void mmu_load_memory(unsigned int index)
{

    mmuUnit->load_memory(index);

}

void mmu_reload_memory()
{

    mmuUnit->reload_memory();

}

void mmu_map_kernel_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    mmuUnit->map_kernel_memory(index, paddress, vaddress, size);

}

void mmu_map_user_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    mmuUnit->map_user_memory(index, paddress, vaddress, size);

}

void mmu_unmap_memory(unsigned int index)
{

    mmuUnit->unmap_memory(index);

}

void mmu_unit_init(struct mmu_unit *unit, void (*enable)(), void (*load_memory)(unsigned int index), void (*reload_memory)(), void (*map_kernel_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size), void (*map_user_memory)(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size), void (*unmap_memory)(unsigned int index))
{

    unit->enable = enable;
    unit->load_memory = load_memory;
    unit->reload_memory = reload_memory;
    unit->map_kernel_memory = map_kernel_memory;
    unit->map_user_memory = map_user_memory;
    unit->unmap_memory = unmap_memory;

}

void mmu_init(struct mmu_unit *unit)
{

    error_assert(unit != 0, "MMU not found", __FILE__, __LINE__);

    mmuUnit = unit;
    mmuUnit->map_kernel_memory(0, 0x00000000, 0x00000000, 0x00400000);
    mmuUnit->load_memory(0);
    mmuUnit->enable();

}

