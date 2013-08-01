#include <fudge/kernel.h>
#include "cpu.h"
#include "mmu.h"

static void set_directory_table(struct mmu_directory *directory, unsigned int frame, struct mmu_table *table, unsigned int tflags)
{

    directory->tables[frame / MMU_TABLES] = (struct mmu_table *)((unsigned int)table | tflags);

}

static void set_table_page(struct mmu_table *table, unsigned int frame, unsigned int page, unsigned int pflags)
{

    table->pages[frame % MMU_PAGES] = (void *)(page | pflags);

}

void mmu_map(struct mmu_directory *directory, struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int frame = vaddress / MMU_PAGESIZE;
    unsigned int i;

    for (i = 0; i < size / MMU_PAGESIZE; i++)
        set_table_page(table, frame + i, paddress + i * MMU_PAGESIZE, pflags);

    set_directory_table(directory, frame, table, tflags);

}

void mmu_enable()
{

    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

void mmu_reload()
{

    cpu_set_cr3(cpu_get_cr3());

}

struct mmu_directory *mmu_get_directory()
{

    return (struct mmu_directory *)cpu_get_cr3();

}

void mmu_set_directory(struct mmu_directory *directory)
{

    cpu_set_cr3((unsigned int)directory);

}

