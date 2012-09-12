#include <memory.h>
#include <runtime.h>
#include <mmu.h>
#include <arch/x86/cpu.h>
#include <arch/x86/isr.h>
#include <arch/x86/mmu.h>

static struct mmu_table kernelTables[8];
static struct mmu_directory runtimeDirectories[RUNTIME_TASK_SLOTS];
static struct mmu_table runtimeTables[RUNTIME_TASK_SLOTS];

static void directory_set_table(struct mmu_directory *directory, unsigned int frame, struct mmu_table *table, unsigned int tflags)
{

    directory->tables[frame / MMU_TABLE_SLOTS] = (struct mmu_table *)((unsigned int)table | tflags);

}

static void table_set_page(struct mmu_table *table, unsigned int frame, unsigned int page, unsigned int pflags)
{

    table->pages[frame % MMU_PAGE_SLOTS] = (void *)(page | pflags);

}

static void load_memory(struct mmu_directory *directory)
{

    cpu_set_cr3((unsigned int)directory);

}

static void load_user_memory(unsigned int index)
{

    load_memory(&runtimeDirectories[index]);

}

static void reload_memory()
{

    cpu_set_cr3(cpu_get_cr3());

}

static void map_memory(struct mmu_directory *directory, struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int frame = vaddress / MMU_PAGE_SIZE;
    unsigned int i;

    memory_clear(table, sizeof (struct mmu_table));

    for (i = 0; i < size / MMU_PAGE_SIZE; i++)
        table_set_page(table, frame + i, paddress + i * MMU_PAGE_SIZE, pflags);

    directory_set_table(directory, frame, table, tflags);

}

static void map_kernel_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
        map_memory(&runtimeDirectories[i], &kernelTables[index], paddress, vaddress, size, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);

}

static void map_user_memory(unsigned int index, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    map_memory(&runtimeDirectories[index], &runtimeTables[index], paddress, vaddress, size, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

}

static void unmap_memory(struct mmu_directory *directory)
{

    memory_clear(directory, sizeof (struct mmu_directory));

}

static void unmap_user_memory(unsigned int index)
{

    unmap_memory(&runtimeDirectories[index]);

}

static void enable()
{

    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

static void handle_interrupt(struct isr_registers *registers)
{

    mmu_pagefault(cpu_get_cr2(), registers->extra);

}

void mmu_setup_arch()
{

    mmu_set_interface(0, enable, load_user_memory, reload_memory, map_kernel_memory, map_user_memory, unmap_user_memory);

    isr_register_routine(ISR_INDEX_PF, handle_interrupt);

}

