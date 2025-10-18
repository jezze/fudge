#include <fudge.h>
#include "cpu.h"
#include "mmu.h"

struct mmu_directory *mmu_getdirectory(void)
{

    return (struct mmu_directory *)cpu_getcr3();

}

struct mmu_table *mmu_gettable(struct mmu_directory *directory, unsigned int vaddress)
{

    unsigned int index = vaddress >> 22;

    return (struct mmu_table *)(directory->tables[index] & 0xFFFFF000);

}

unsigned int mmu_gettableflags(struct mmu_directory *directory, unsigned int vaddress, unsigned int flags)
{

    unsigned int index = vaddress >> 22;

    return directory->tables[index] & 0xFFF;

}

unsigned int mmu_getpageflags(struct mmu_directory *directory, unsigned int vaddress, unsigned int flags)
{

    struct mmu_table *table = mmu_gettable(directory, vaddress);

    if (table)
    {

        unsigned int index = (vaddress << 10) >> 22;

        return table->pages[index] & 0xFFF;

    }

    return 0;

}

void mmu_setdirectory(struct mmu_directory *directory)
{

    cpu_setcr3((unsigned int)directory);

}

void mmu_settable(struct mmu_directory *directory, unsigned int paddress, unsigned int vaddress, unsigned int flags)
{

    unsigned int index = vaddress >> 22;

    directory->tables[index] = paddress | flags;

}

void mmu_settableflags(struct mmu_directory *directory, unsigned int vaddress, unsigned int flags)
{

    unsigned int index = vaddress >> 22;

    directory->tables[index] &= 0xFFFFF000;
    directory->tables[index] |= flags;

}

void mmu_setpage(struct mmu_directory *directory, unsigned int paddress, unsigned int vaddress, unsigned int flags)
{

    struct mmu_table *table = mmu_gettable(directory, vaddress);

    if (table)
    {

        unsigned int index = (vaddress << 10) >> 22;

        table->pages[index] = paddress | flags;

    }

}

void mmu_setpageflags(struct mmu_directory *directory, unsigned int vaddress, unsigned int flags)
{

    struct mmu_table *table = mmu_gettable(directory, vaddress);

    if (table)
    {

        unsigned int index = (vaddress << 10) >> 22;

        table->pages[index] &= 0xFFFFF000;
        table->pages[index] |= flags;

    }

}

void mmu_enable(void)
{

    cpu_setcr0(cpu_getcr0() | 0x80000000);

}

void mmu_disable(void)
{

    cpu_setcr0(cpu_getcr0() & ~0x80000000);

}

