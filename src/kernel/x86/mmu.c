#include <fudge.h>
#include "cpu.h"
#include "mmu.h"

struct mmu_directory *mmu_getdirectory(void)
{

    return (struct mmu_directory *)cpu_getcr3();

}

struct mmu_table *mmu_getdirectorytable(struct mmu_directory *directory, unsigned int vaddress)
{

    unsigned int index = vaddress >> 22;

    return (struct mmu_table *)(directory->tables[index] & 0xFFFFF000);

}

void mmu_setdirectorytable(struct mmu_directory *directory, unsigned int paddress, unsigned int vaddress, unsigned int tflags)
{

    unsigned int index = vaddress >> 22;

    directory->tables[index] = paddress | tflags;

}

void mmu_settablepage(struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int pflags)
{

    unsigned int index = (vaddress << 10) >> 22;

    table->pages[index] = paddress | pflags;

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

