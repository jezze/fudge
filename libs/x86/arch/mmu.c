#include <fudge/kernel.h>
#include "cpu.h"
#include "mmu.h"

static void set_directory_table(struct mmu_directory *directory, unsigned long paddress, unsigned long vaddress, unsigned int tflags)
{

    unsigned int index = (vaddress / MMU_PAGESIZE) / MMU_TABLES;

    directory->tables[index] = (struct mmu_table *)(paddress | tflags);

}

static void set_table_page(struct mmu_table *table, unsigned long paddress, unsigned long vaddress, unsigned int pflags)
{

    unsigned int index = (vaddress / MMU_PAGESIZE) % MMU_PAGES;

    table->pages[index] = (void *)(paddress | pflags);

}

void mmu_map(struct mmu_directory *directory, struct mmu_table *table, unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
        set_table_page(table, paddress + i, vaddress + i, pflags);

    set_directory_table(directory, (unsigned long)table, vaddress, tflags);

}

void mmu_enable()
{

    cpu_set_cr0(cpu_get_cr0() | 0x80000000);

}

void mmu_reload()
{

    cpu_set_cr3(cpu_get_cr3());

}

void mmu_load(struct mmu_directory *directory)
{

    cpu_set_cr3((unsigned long)directory);

}

