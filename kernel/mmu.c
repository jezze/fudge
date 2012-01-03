#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/log.h>
#include <kernel/mmu.h>

static struct mmu_unit *primary;
static struct mmu_memory kernel;

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

void mmu_load_memory(struct mmu_memory *memory)
{

    primary->load_memory(memory);

}

void mmu_map_kernel_memory(struct mmu_memory *memory)
{

    primary->map_kernel_memory(memory);

}

void mmu_map_user_memory(struct mmu_memory *memory)
{

    primary->map_user_memory(memory);

}

void mmu_unmap_memory(struct mmu_memory *memory)
{

    primary->unmap_memory(memory);

}

void mmu_register_unit(struct mmu_unit *unit)
{

    primary = unit;

    mmu_memory_init(&kernel, (void *)0x00000000, (void *)0x00000000, 0x00400000);
    mmu_map_kernel_memory(&kernel);
    mmu_load_memory(&kernel);

}

void mmu_memory_init(struct mmu_memory *memory, void *paddress, void *vaddress, unsigned int size)
{

    memory_clear(memory, sizeof (struct mmu_memory));

    memory->paddress = paddress;
    memory->vaddress = vaddress;
    memory->size = size;

}

void mmu_unit_init(struct mmu_unit *unit, void (*load_memory)(struct mmu_memory *memory), void (*map_kernel_memory)(struct mmu_memory *memory), void (*map_user_memory)(struct mmu_memory *memory), void (*unmap_memory)(struct mmu_memory *memory))
{

    memory_clear(unit, sizeof (struct mmu_unit));

    unit->load_memory = load_memory;
    unit->map_kernel_memory = map_kernel_memory;
    unit->map_user_memory = map_user_memory;
    unit->unmap_memory = unmap_memory;

}

