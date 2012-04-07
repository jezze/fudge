#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/log.h>
#include <kernel/mmu.h>

static struct mmu_unit *mmuUnit;
static struct mmu_memory mmuKernelMemory[2];

void mmu_pagefault(unsigned int address, unsigned int flags)
{

    log_write("ERROR!\n");

    if (!(flags & MMU_ERROR_PRESENT))
        log_write("Page: present\n");

    if (flags & MMU_ERROR_RW)
        log_write("Page: read-only\n");

    if (flags & MMU_ERROR_USER)
        log_write("Page: user-mode\n");

    if (flags & MMU_ERROR_RESERVED)
        log_write("Page: reserved\n");

    if (flags & MMU_ERROR_FETCH)
        log_write("Page: fetch\n");

    log_write("Address: 0x%x\n", address);

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

void mmu_map_kernel_memory(struct mmu_memory *memory)
{

    mmuUnit->map_kernel_memory(memory);

}

void mmu_map_user_memory(unsigned int index, struct mmu_memory *memory)
{

    mmuUnit->map_user_memory(index, memory);

}

void mmu_unmap_memory(unsigned int index)
{

    mmuUnit->unmap_memory(index);

}

void mmu_memory_init(struct mmu_memory *memory, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    memory_clear(memory, sizeof (struct mmu_memory));

    memory->paddress = paddress;
    memory->vaddress = vaddress;
    memory->size = size;

}

void mmu_unit_init(struct mmu_unit *unit, void (*enable)(), void (*load_memory)(unsigned int index), void (*reload_memory)(), void (*map_kernel_memory)(struct mmu_memory *memory), void (*map_user_memory)(unsigned int index, struct mmu_memory *memory), void (*unmap_memory)(unsigned int index))
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

    mmu_memory_init(&mmuKernelMemory[0], 0x00000000, 0x00000000, 0x00400000);
    mmu_memory_init(&mmuKernelMemory[1], 0x00400000, 0x00400000, 0x00400000);

    mmuUnit = unit;
    mmuUnit->map_kernel_memory(&mmuKernelMemory[0]);
    mmuUnit->map_kernel_memory(&mmuKernelMemory[1]);
    mmuUnit->load_memory(0);
    mmuUnit->enable();

}

