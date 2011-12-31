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

static void mmu_clear_directory(struct mmu_directory *directory)
{

    memory_clear(directory, sizeof (struct mmu_directory));

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

static void mmu_map(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    struct mmu_header *header = mmu_get_header(paddress);

    unsigned int frame = (unsigned int)vaddress / MMU_PAGE_SLOTS;
    unsigned int index = frame / MMU_DIRECTORY_SLOTS;
    unsigned int count = size / MMU_PAGE_SIZE + ((size & 0xFFF) > 0);

    mmu_clear_table(&header->table);
    header->directory.tables[index] = (struct mmu_table *)((unsigned int)&header->table | tflags);

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        *mmu_get_entry(&header->directory, frame + i) = (unsigned int)paddress | pflags;

        paddress += MMU_PAGE_SIZE;

    }

}

static void mmu_set_directory(void *paddress)
{

    struct mmu_header *header = mmu_get_header(paddress);

    cpu_set_cr3((unsigned int)&header->directory);

}

static void mmu_enable()
{

    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

static void *mmu_get_paddress(unsigned int id)
{

    return programHeaders[id].memory.paddress;

}

static void mmu_setup()
{

    kernelHeader.memory.paddress = 0x00000000;
    kernelHeader.memory.vaddress = 0x00000000;
    kernelHeader.memory.size = 0x00400000;

    mmu_clear_directory(&kernelHeader.directory);
    mmu_map(kernelHeader.memory.paddress, kernelHeader.memory.vaddress, kernelHeader.memory.size, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);
    mmu_set_directory(kernelHeader.memory.paddress);

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        programHeaders[i].memory.paddress = (void *)(0x00300000 + i * 0x10000);
        memory_copy(&programHeaders[i].directory, &kernelHeader.directory, sizeof (struct mmu_directory));

    }

    isr_register_routine(ISR_ROUTINE_PF, mmu_handle_pagefault);

}

void mmu_init()
{

    mmu_unit_init(&unit, mmu_setup, mmu_enable, mmu_get_paddress, mmu_set_directory, mmu_map);
    mmu_register_unit(&unit);

}

