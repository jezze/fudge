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

static void table_set_page(struct mmu_table *table, unsigned int frame, unsigned int page, unsigned int pflags)
{

    table->pages[frame % MMU_TABLE_SLOTS] = (void *)(page | pflags);

}

static unsigned int get_frame(unsigned int vaddress)
{

    return vaddress / MMU_PAGE_SIZE;

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

static void map_memory(struct mmu_directory *directory, struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int frame = get_frame(vaddress);
    unsigned int i;

    table_clear(table);

    for (i = 0; i < size / MMU_PAGE_SIZE; i++)
        table_set_page(table, frame + i, paddress + i * MMU_PAGE_SIZE, pflags); 

    directory_set_table(directory, frame, table, tflags);

}

static void map_kernel_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = &kernelDirectory;
    struct mmu_table *table = &kernelTables[index];
    unsigned int i;

    map_memory(directory, table, paddress, vaddress, size, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

    for (i = 0; i < MMU_HEADER_SLOTS; i++)
        map_memory(&directories[i], table, paddress, vaddress, size, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

}

static void map_user_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    struct mmu_directory *directory = &directories[index];
    struct mmu_table *table = &tables[index];

    memory_copy(directory, &kernelDirectory, sizeof (struct mmu_directory));
    map_memory(directory, table, paddress, vaddress, size, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

}

static void unmap_memory(unsigned int index)
{

    struct mmu_directory *directory = &directories[index];

    directory_clear(directory);

}

static void enable()
{

    isr_register_routine(ISR_INDEX_PF, handle_pagefault);
    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

void mmu_setup()
{

    mmu_unit_init(&unit, enable, load_memory, reload_memory, map_kernel_memory, map_user_memory, unmap_memory);
    mmu_init(&unit);

}

