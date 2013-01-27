#include <fudge/kernel.h>
#include "cpu.h"
#include "mmu.h"

static void set_directory_table(struct mmu_directory *directory, unsigned int frame, struct mmu_table *table, unsigned int tflags)
{

    directory->tables[frame / MMU_TABLE_SLOTS] = (struct mmu_table *)((unsigned int)table | tflags);

}

static void set_table_page(struct mmu_table *table, unsigned int frame, unsigned int page, unsigned int pflags)
{

    table->pages[frame % MMU_PAGE_SLOTS] = (void *)(page | pflags);

}

void mmu_map_memory(struct mmu_directory *directory, struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int frame = vaddress / MMU_PAGE_SIZE;
    unsigned int i;

    memory_clear(table, sizeof (struct mmu_table));

    for (i = 0; i < size / MMU_PAGE_SIZE; i++)
        set_table_page(table, frame + i, paddress + i * MMU_PAGE_SIZE, pflags);

    set_directory_table(directory, frame, table, tflags);

}

void mmu_enable()
{

    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

void mmu_load_memory(struct mmu_directory *directory)
{

    cpu_set_cr3((unsigned int)directory);

}

void mmu_reload_memory()
{

    cpu_set_cr3(cpu_get_cr3());

}

