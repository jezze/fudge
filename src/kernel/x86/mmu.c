#include <fudge.h>
#include "cpu.h"
#include "mmu.h"

static void setdirectorytable(struct mmu_directory *directory, unsigned int paddress, unsigned int vaddress, unsigned int tflags)
{

    unsigned int index = vaddress >> 22;

    directory->tables[index] = (struct mmu_table *)(paddress | tflags);

}

static void settablepage(struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int pflags)
{

    unsigned int index = (vaddress << 10) >> 22;

    table->pages[index] = (void *)(paddress | pflags);

}

void mmu_map(struct mmu_directory *directory, struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
        settablepage(table, paddress + i, vaddress + i, pflags);

    setdirectorytable(directory, (unsigned int)table, vaddress, tflags);

}

void mmu_setdirectory(struct mmu_directory *directory)
{

    cpu_setcr3((unsigned int)directory);

}

void mmu_enable(void)
{

    cpu_setcr0(cpu_getcr0() | 0x80000000);

}

void mmu_disable(void)
{

    cpu_setcr0(cpu_getcr0() & ~0x80000000);

}

