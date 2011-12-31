#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/mmu.h>
#include <kernel/log.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>

static struct mmu_unit unit;
static struct mmu_header kernelHeader;
static struct mmu_header programHeaders[8];

static void mmu_handle_pagefault(struct isr_registers *registers)
{

    unsigned int address = cpu_get_cr2();

    mmu_pagefault(address, registers->error);

}

static void mmu_clear_table(struct mmu_table *table)
{

    memory_clear(table, sizeof (struct mmu_table));

}

static struct mmu_table *mmu_get_table(struct mmu_directory *directory, unsigned int frame)
{

    return (struct mmu_table *)((unsigned int)directory->tables[frame / MMU_DIRECTORY_SLOTS] & 0xFFFFF000);

}

static unsigned int *mmu_get_entry(struct mmu_directory *directory, unsigned int frame)
{

    return &mmu_get_table(directory, frame)->entries[frame % MMU_TABLE_SLOTS];

}

static struct mmu_header *mmu_get_header(void *paddress)
{

    if (!paddress)
        return &kernelHeader;

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (programHeaders[i].memory.paddress == paddress)
            return &programHeaders[i];

    }

    return 0;

}

static void mmu_enable()
{

    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

static void mmu_map_memory(struct mmu_memory *memory, unsigned int tflags, unsigned int pflags)
{

    struct mmu_header *header = mmu_get_header(memory->paddress);

    unsigned int frame = (unsigned int)memory->vaddress / MMU_PAGE_SIZE;
    unsigned int index = frame / MMU_DIRECTORY_SLOTS;
    unsigned int count = memory->size / MMU_PAGE_SIZE + ((memory->size & 0xFFF) > 0);

    mmu_clear_table(&header->table);
    header->directory.tables[index] = (struct mmu_table *)((unsigned int)&header->table | tflags);

    unsigned int i;
    void *paddress = memory->paddress;

    for (i = 0; i < count; i++)
    {

        *mmu_get_entry(&header->directory, frame + i) = (unsigned int)paddress | pflags;

        paddress += MMU_PAGE_SIZE;

    }

}

static struct mmu_memory *mmu_get_memory()
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        struct mmu_memory *memory = &programHeaders[i].memory;

        if (memory->used)
            continue;

        mmu_memory_init(memory, 1, (void *)(0x00300000 + i * 0x10000), (void *)0x00000000, 0x10000);
        memory_copy(&programHeaders[i].directory, &kernelHeader.directory, sizeof (struct mmu_directory));

        return memory;

    }

    return 0;

}

static void mmu_unget_memory(struct mmu_memory *memory)
{

    memory->used = 0;

}

static void mmu_load_memory(struct mmu_memory *memory)
{

    struct mmu_header *header = mmu_get_header(memory->paddress);

    cpu_set_cr3((unsigned int)&header->directory);

}

void mmu_init()
{

    struct mmu_memory *memory = &kernelHeader.memory;

    mmu_memory_init(memory, 1, (void *)0x00000000, (void *)0x00000000, 0x00400000);
    mmu_map_memory(memory, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);
    mmu_load_memory(memory);

    isr_register_routine(ISR_ROUTINE_PF, mmu_handle_pagefault);

    mmu_enable();

    unit.get_task_memory = mmu_get_memory;
    unit.unget_task_memory = mmu_unget_memory;
    unit.load_task_memory = mmu_load_memory;
    unit.map_task_memory = mmu_map_memory;

    mmu_register_unit(&unit);

}

