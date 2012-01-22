#include <lib/memory.h>
#include <kernel/mmu.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>

static struct mmu_unit unit;
static struct mmu_directory directories[MMU_HEADER_SLOTS];
static struct mmu_directory *kernelDirectory;
static struct mmu_table tables[MMU_HEADER_SLOTS];

static void mmu_handle_pagefault(struct isr_registers *registers)
{

    unsigned int address = cpu_get_cr2();

    mmu_pagefault(address, registers->error);

}

static void mmu_directory_clear(struct mmu_directory *directory)
{

    memory_clear(directory, sizeof (struct mmu_directory));

}

static struct mmu_table *mmu_directory_get_table_by_frame(struct mmu_directory *directory, unsigned int frame)
{

    return (struct mmu_table *)((unsigned int)directory->tables[frame / MMU_DIRECTORY_SLOTS] & 0xFFFFF000);

}

static void mmu_directory_set_table(struct mmu_directory *directory, unsigned int frame, struct mmu_table *table, unsigned int tflags)
{

    directory->tables[frame / MMU_DIRECTORY_SLOTS] = (struct mmu_table *)((unsigned int)table | tflags);

}

static void mmu_table_clear(struct mmu_table *table)
{

    memory_clear(table, sizeof (struct mmu_table));

}

static void *mmu_table_get_page_by_frame(struct mmu_table *table, unsigned int frame)
{

    return (void *)((unsigned int)table->pages[frame % MMU_TABLE_SLOTS] & 0xFFFFF000);

}

static void mmu_table_set_page(struct mmu_table *table, unsigned int frame, void *page, unsigned int pflags)
{

    table->pages[frame % MMU_TABLE_SLOTS] = (void *)((unsigned int)page | pflags);

}

static unsigned int mmu_get_frame(struct mmu_memory *memory)
{

    return (unsigned int)memory->vaddress / MMU_PAGE_SIZE;

}

static void mmu_unit_load_memory(unsigned int index)
{

    struct mmu_directory *directory = &directories[index];

    cpu_set_cr3((unsigned int)directory);

}

static void mmu_unit_map_memory(struct mmu_directory *directory, struct mmu_table *table, struct mmu_memory *memory, unsigned int tflags, unsigned int pflags)
{

    mmu_table_clear(table);

    unsigned int frame = mmu_get_frame(memory);
    unsigned int i;

    for (i = 0; i < memory->size / MMU_PAGE_SIZE; i++)
        mmu_table_set_page(table, frame + i, memory->paddress + i * MMU_PAGE_SIZE, pflags); 

    mmu_directory_set_table(directory, frame, table, tflags);

}

static void mmu_unit_map_kernel_memory(struct mmu_memory *memory)
{

    struct mmu_directory *directory = &directories[0];
    struct mmu_table *table = &tables[0];

    mmu_unit_map_memory(directory, table, memory, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

    kernelDirectory = directory;

}

static void mmu_unit_map_user_memory(unsigned int index, struct mmu_memory *memory)
{

    struct mmu_directory *directory = &directories[index];
    struct mmu_table *table = &tables[index];

    mmu_unit_map_memory(directory, table, memory, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

    memory_copy(directory, kernelDirectory, sizeof (unsigned int));

}

static void mmu_unit_unmap_memory(unsigned int index)
{

    struct mmu_directory *directory = &directories[index];

    mmu_directory_clear(directory);

}

void mmu_unit_enable()
{

    isr_register_routine(ISR_ROUTINE_PF, mmu_handle_pagefault);
    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

void mmu_setup()
{

    unit.enable = mmu_unit_enable;
    unit.load_memory = mmu_unit_load_memory;
    unit.map_kernel_memory = mmu_unit_map_kernel_memory;
    unit.map_user_memory = mmu_unit_map_user_memory;
    unit.unmap_memory = mmu_unit_unmap_memory;

    mmu_register_unit(&unit);
    mmu_init();

}

