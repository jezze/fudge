#include <kernel/error.h>
#include <kernel/log.h>
#include <kernel/mmu.h>

static struct mmu_unit *primary;

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

struct mmu_memory *mmu_get_task_memory(unsigned int id)
{

    return primary->get_task_memory(id);

}

void mmu_unget_task_memory(struct mmu_memory *memory)
{

    return primary->unget_task_memory(memory);

}

void mmu_load_task_memory(struct mmu_memory *memory)
{

    primary->load_task_memory(memory);

}

void mmu_map_task_memory(struct mmu_memory *memory, unsigned int tflags, unsigned int pflags)
{

    primary->map_task_memory(memory, tflags, pflags);

}

void mmu_register_unit(struct mmu_unit *unit)
{

    primary = unit;

}

void mmu_memory_init(struct mmu_memory *memory, unsigned int used, void *paddress, void *vaddress, unsigned int size)
{

    memory->used = used;
    memory->paddress = paddress;
    memory->vaddress = vaddress;
    memory->size = size;

}
