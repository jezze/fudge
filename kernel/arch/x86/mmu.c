#include <lib/memory.h>
#include <kernel/mmu.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>

static struct mmu_unit unit;
static struct mmu_header headers[MMU_HEADER_SLOTS];
static struct mmu_header *kernelHeader;

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

static unsigned int mmu_get_header_slot()
{

    unsigned int i;

    for (i = 0; i < MMU_HEADER_SLOTS; i++)
    {

        if (!headers[i].memory)
            return i;

    }

    return 0;

}

static struct mmu_header *mmu_get_header(struct mmu_memory *memory)
{

    unsigned int i;

    for (i = 0; i < MMU_HEADER_SLOTS; i++)
    {

        if (headers[i].memory == memory)
            return &headers[i];

    }

    return 0;

}

static void mmu_unit_load_memory(struct mmu_memory *memory)
{

    struct mmu_header *header = mmu_get_header(memory);

    cpu_set_cr3((unsigned int)&header->directory);

}

static void mmu_unit_map_memory(struct mmu_memory *memory, unsigned int tflags, unsigned int pflags)
{

    struct mmu_header *header = &headers[mmu_get_header_slot()];
    header->memory = memory;

    mmu_table_clear(&header->table);

    unsigned int frame = (unsigned int)header->memory->vaddress / MMU_PAGE_SIZE;
    unsigned int i;

    for (i = 0; i < header->memory->size / MMU_PAGE_SIZE; i++)
        mmu_table_set_page(&header->table, frame + i, header->memory->paddress + i * MMU_PAGE_SIZE, pflags); 

    mmu_directory_set_table(&header->directory, frame, &header->table, tflags);

}

static void mmu_unit_map_kernel_memory(struct mmu_memory *memory)
{

    mmu_unit_map_memory(memory, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

    kernelHeader = mmu_get_header(memory);

}

static void mmu_unit_map_user_memory(struct mmu_memory *memory)
{

    mmu_unit_map_memory(memory, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

    struct mmu_header *header = mmu_get_header(memory);

    memory_copy(&header->directory, &kernelHeader->directory, sizeof (unsigned int));

}

static void mmu_unit_unmap_memory(struct mmu_memory *memory)
{

    struct mmu_header *header = mmu_get_header(memory);

    header->memory = 0;

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

