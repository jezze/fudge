#include <fudge.h>
#include "cpu.h"
#include "mmu.h"

static void setdirectorytable(struct mmu_directory *directory, unsigned long paddress, unsigned long vaddress, unsigned int tflags)
{

    unsigned int index = (vaddress / MMU_PAGESIZE) / MMU_TABLES;

    directory->tables[index] = (struct mmu_table *)(paddress | tflags);

}

static void settablepage(struct mmu_table *table, unsigned long paddress, unsigned long vaddress, unsigned int pflags)
{

    unsigned int index = (vaddress / MMU_PAGESIZE) % MMU_PAGES;

    table->pages[index] = (void *)(paddress | pflags);

}

void mmu_map(struct mmu_directory *directory, struct mmu_table *table, unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
        settablepage(table, paddress + i, vaddress + i, pflags);

    setdirectorytable(directory, (unsigned long)table, vaddress, tflags);

}

void mmu_setdirectory(struct mmu_directory *directory)
{

    cpu_setcr3((unsigned long)directory);

}

void mmu_setup()
{

    cpu_setcr0(cpu_getcr0() | 0x80000000);

}

