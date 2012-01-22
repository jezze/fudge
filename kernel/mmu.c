#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/log.h>
#include <kernel/mmu.h>

static struct mmu_unit *primary;
static struct mmu_memory kernel[2];

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

    primary->load_memory(index);

}

void mmu_reload_memory()
{

    primary->reload_memory();

}

void mmu_map_kernel_memory(struct mmu_memory *memory)
{

    primary->map_kernel_memory(memory);

}

void mmu_map_user_memory(unsigned int index, struct mmu_memory *memory)
{

    primary->map_user_memory(index, memory);

}

void mmu_unmap_memory(unsigned int index)
{

    primary->unmap_memory(index);

}

void mmu_register_unit(struct mmu_unit *unit)
{

    primary = unit;

}

void mmu_memory_init(struct mmu_memory *memory, void *paddress, void *vaddress, unsigned int size)
{

    memory_clear(memory, sizeof (struct mmu_memory));

    memory->paddress = paddress;
    memory->vaddress = vaddress;
    memory->size = size;

}

void mmu_init()
{

    mmu_memory_init(&kernel[0], (void *)0x00000000, (void *)0x00000000, 0x00400000);
    mmu_memory_init(&kernel[1], (void *)0x00400000, (void *)0x00400000, 0x00400000);

    if (!primary)
        error_panic("No MMU registered", __FILE__, __LINE__);

    primary->map_kernel_memory(&kernel[0]);
    primary->map_kernel_memory(&kernel[1]);
    primary->load_memory(0);
    primary->enable();

}

