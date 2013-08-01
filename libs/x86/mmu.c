#include <fudge/kernel.h>
#include "cpu.h"
#include "mmu.h"

static void set_directory_table(struct mmu_directory *directory, unsigned int vaddress, struct mmu_table *table, unsigned int tflags)
{

    unsigned int index = (vaddress / MMU_PAGESIZE) / MMU_TABLES;

    directory->tables[index] = (struct mmu_table *)((unsigned int)table | tflags);

}

static void set_table_page(struct mmu_table *table, unsigned int vaddress, unsigned int page, unsigned int pflags)
{

    unsigned int index = (vaddress / MMU_PAGESIZE) % MMU_PAGES;

    table->pages[index] = (void *)(page | pflags);

}

void mmu_map(struct mmu_directory *directory, struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
        set_table_page(table, vaddress + i, paddress + i, pflags);

    set_directory_table(directory, vaddress, table, tflags);

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

