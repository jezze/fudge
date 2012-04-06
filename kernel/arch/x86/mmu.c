#include <lib/memory.h>
#include <kernel/mmu.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>

static struct mmu_unit unit;
static struct mmu_directory directories[MMU_HEADER_SLOTS];
static struct mmu_table tables[MMU_HEADER_SLOTS];
static struct mmu_directory kernelDirectory;
static struct mmu_table kernelTables[4];

static void handle_pagefault(struct isr_cpu_registers *registers)
{

    mmu_pagefault(cpu_get_cr2(), registers->error);

}

static void directory_clear(struct mmu_directory *directory)
{

    memory_clear(directory, sizeof (struct mmu_directory));

}

static void directory_set_table(struct mmu_directory *directory, unsigned int frame, struct mmu_table *table, unsigned int tflags)
{

    directory->tables[frame / MMU_DIRECTORY_SLOTS] = (struct mmu_table *)((unsigned int)table | tflags);

}

static void table_clear(struct mmu_table *table)
{

    memory_clear(table, sizeof (struct mmu_table));

}

static void table_set_page(struct mmu_table *table, unsigned int frame, void *page, unsigned int pflags)
{

    table->pages[frame % MMU_TABLE_SLOTS] = (void *)((unsigned int)page | pflags);

}

static unsigned int get_frame(struct mmu_memory *memory)
{

    return (unsigned int)memory->vaddress / MMU_PAGE_SIZE;

}

static void load_memory(unsigned int index)
{

    struct mmu_directory *directory = (index) ? &directories[index] : &kernelDirectory;

    cpu_set_cr3((unsigned int)directory);

}

static void reload_memory()
{

    cpu_set_cr3(cpu_get_cr3());

}

static void map_memory(struct mmu_directory *directory, struct mmu_table *table, struct mmu_memory *memory, unsigned int tflags, unsigned int pflags)
{

    table_clear(table);

    unsigned int frame = get_frame(memory);
    unsigned int i;

    for (i = 0; i < memory->size / MMU_PAGE_SIZE; i++)
        table_set_page(table, frame + i, memory->paddress + i * MMU_PAGE_SIZE, pflags); 

    directory_set_table(directory, frame, table, tflags);

}

static void map_kernel_memory(struct mmu_memory *memory)
{

    struct mmu_directory *directory = &kernelDirectory;

    // FIX THIS
    struct mmu_table *table;

    switch ((unsigned int)memory->paddress)
    {

        case 0x00000000:

            table = &kernelTables[0];

            break;

        case 0x00400000:

            table = &kernelTables[1];

            break;

        default:

            table = &kernelTables[2];

            break;

    }

    map_memory(directory, table, memory, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

    unsigned int i;

    for (i = 0; i < MMU_HEADER_SLOTS; i++)
        map_memory(&directories[i], table, memory, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

}

static void map_user_memory(unsigned int index, struct mmu_memory *memory)
{

    struct mmu_directory *directory = &directories[index];
    struct mmu_table *table = &tables[index];

    memory_copy(directory, &kernelDirectory, sizeof (struct mmu_directory));

    map_memory(directory, table, memory, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

}

static void unmap_memory(unsigned int index)
{

    struct mmu_directory *directory = &directories[index];

    directory_clear(directory);

}

static void enable()
{

    isr_register_routine(ISR_ROUTINE_PF, handle_pagefault);
    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

void mmu_setup()
{

    mmu_unit_init(&unit, enable, load_memory, reload_memory, map_kernel_memory, map_user_memory, unmap_memory);
    mmu_init(&unit);

}

