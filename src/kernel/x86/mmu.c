#include <fudge.h>
#include "cpu.h"
#include "mmu.h"

struct mmu_directory *mmu_getdirectory(void)
{

    return (struct mmu_directory *)cpu_getcr3();

}

unsigned int mmu_gettablevalue(struct mmu_directory *directory, unsigned int vaddress)
{

    unsigned int index = vaddress >> 22;

    return directory->tables[index];

}

unsigned int mmu_gettableaddress(struct mmu_directory *directory, unsigned int vaddress)
{

    return mmu_gettablevalue(directory, vaddress) & 0xFFFFF000;

}

unsigned int mmu_gettableflags(struct mmu_directory *directory, unsigned int vaddress)
{

    return mmu_gettablevalue(directory, vaddress) & 0xFFF;

}

unsigned int mmu_getpagevalue(struct mmu_directory *directory, unsigned int vaddress)
{

    struct mmu_table *table = (struct mmu_table *)mmu_gettableaddress(directory, vaddress);

    if (table)
    {

        unsigned int index = (vaddress << 10) >> 22;

        return table->pages[index];

    }

    return 0;

}

unsigned int mmu_getpageaddress(struct mmu_directory *directory, unsigned int vaddress)
{

    return mmu_getpagevalue(directory, vaddress) & 0xFFFFF000;

}

unsigned int mmu_getpageflags(struct mmu_directory *directory, unsigned int vaddress)
{

    return mmu_getpagevalue(directory, vaddress) & 0xFFF;

}

void mmu_setdirectory(struct mmu_directory *directory)
{

    cpu_setcr3((unsigned int)directory);

}

void mmu_settablevalue(struct mmu_directory *directory, unsigned int vaddress, unsigned int value)
{

    unsigned int index = vaddress >> 22;

    directory->tables[index] = value;

}

void mmu_settableaddress(struct mmu_directory *directory, unsigned int vaddress, unsigned int address)
{

    unsigned int index = vaddress >> 22;

    directory->tables[index] = address & 0xFFFFF000;

}

void mmu_settableflags(struct mmu_directory *directory, unsigned int vaddress, unsigned int flags)
{

    unsigned int index = vaddress >> 22;

    directory->tables[index] &= 0xFFFFF000;
    directory->tables[index] |= flags;

}

void mmu_setpagevalue(struct mmu_directory *directory, unsigned int vaddress, unsigned int value)
{

    struct mmu_table *table = (struct mmu_table *)mmu_gettableaddress(directory, vaddress);

    if (table)
    {

        unsigned int index = (vaddress << 10) >> 22;

        table->pages[index] = value;

    }

}

void mmu_setpageaddress(struct mmu_directory *directory, unsigned int vaddress, unsigned int address)
{

    struct mmu_table *table = (struct mmu_table *)mmu_gettableaddress(directory, vaddress);

    if (table)
    {

        unsigned int index = (vaddress << 10) >> 22;

        table->pages[index] = address & 0xFFFFF000;

    }

}

void mmu_setpageflags(struct mmu_directory *directory, unsigned int vaddress, unsigned int flags)
{

    struct mmu_table *table = (struct mmu_table *)mmu_gettableaddress(directory, vaddress);

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

