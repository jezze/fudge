#include <lib/memory.h>
#include <kernel/mmu.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>

static struct mmu_unit unit;
static struct mmu_header headers[8];
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

static unsigned int mmu_get_frame(struct mmu_memory *memory)
{

    return (unsigned int)memory->vaddress / MMU_PAGE_SIZE;

}

static struct mmu_header *mmu_get_header(struct mmu_memory *memory)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (headers[i].memory.paddress == memory->paddress)
            return &headers[i];

    }

    return 0;

}

static void mmu_map_memory(struct mmu_header *header, unsigned int tflags, unsigned int pflags)
{

    mmu_table_clear(&header->table);

    unsigned int frame = mmu_get_frame(&header->memory);
    unsigned int i;

    for (i = 0; i < header->memory.size / MMU_PAGE_SIZE; i++)
        mmu_table_set_page(&header->table, frame + i, header->memory.paddress + i * MMU_PAGE_SIZE, pflags); 

    mmu_directory_set_table(&header->directory, frame, &header->table, tflags);
}

static void mmu_map_memory_kernel(struct mmu_memory *memory)
{

    struct mmu_header *header = mmu_get_header(memory);

    mmu_map_memory(header, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

}

static void mmu_map_memory_user(struct mmu_memory *memory)
{

    struct mmu_header *header = mmu_get_header(memory);

    memory_copy(&header->directory, &kernelHeader->directory, sizeof (struct mmu_directory));
    mmu_map_memory(header, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

}

static unsigned int mmu_get_unused_header_slot()
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        struct mmu_memory *memory = &headers[i].memory;

        if (!memory->used)
            return i;

    }

    return 0;

}

static struct mmu_memory *mmu_get_memory()
{

    unsigned int index = mmu_get_unused_header_slot();
    struct mmu_header *header = &headers[index];

    mmu_memory_init(&header->memory, 1, (void *)(0x00300000 + index * 0x10000), (void *)0x00000000, 0x10000);

    return &header->memory;

}

static void mmu_unget_memory(struct mmu_memory *memory)
{

    memory->used = 0;

}

static void mmu_load_memory(struct mmu_memory *memory)
{

    struct mmu_header *header = mmu_get_header(memory);

    cpu_set_cr3((unsigned int)&header->directory);

}

void mmu_enable()
{

    kernelHeader = &headers[mmu_get_unused_header_slot()];

    mmu_memory_init(&kernelHeader->memory, 1, (void *)0x00000000, (void *)0x00000000, 0x00400000);
    mmu_map_memory_kernel(&kernelHeader->memory);
    mmu_load_memory(&kernelHeader->memory);

    isr_register_routine(ISR_ROUTINE_PF, mmu_handle_pagefault);
    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

    unit.get_task_memory = mmu_get_memory;
    unit.unget_task_memory = mmu_unget_memory;
    unit.load_task_memory = mmu_load_memory;
    unit.map_task_memory = mmu_map_memory_user;

    mmu_register_unit(&unit);

}

