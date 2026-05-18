#include <fudge.h>
#include "cpu.h"
#include "mmu.h"

struct mmu_table
{

    unsigned int pages[MMU_PAGES];

};

struct mmu_directory
{

    unsigned int tables[MMU_TABLES];

};

unsigned long mmu_getdirectory(void)
{

    return cpu_getcr3();

}

unsigned long mmu_gettable(unsigned long daddress, unsigned long vaddress)
{

    struct mmu_directory *directory = (struct mmu_directory *)daddress;
    unsigned int index = vaddress >> 22;

    return directory->tables[index];

}

unsigned long mmu_getpage(unsigned long daddress, unsigned long vaddress)
{

    struct mmu_table *table = (struct mmu_table *)(mmu_gettable(daddress, vaddress) & ~MMU_PAGEMASK);

    if (table)
    {

        unsigned int index = (vaddress << 10) >> 22;

        return table->pages[index];

    }

    return 0;

}

void mmu_setdirectory(unsigned long daddress)
{

    cpu_setcr3(daddress);

}

void mmu_settable(unsigned long daddress, unsigned long vaddress, unsigned long taddress, unsigned int flags)
{

    struct mmu_directory *directory = (struct mmu_directory *)daddress;
    unsigned int index = vaddress >> 22;

    directory->tables[index] = (taddress & ~MMU_PAGEMASK) | (flags & MMU_PAGEMASK);

}

void mmu_setpage(unsigned long daddress, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    struct mmu_table *table = (struct mmu_table *)(mmu_gettable(daddress, vaddress) & ~MMU_PAGEMASK);

    if (table)
    {

        unsigned int index = (vaddress << 10) >> 22;

        table->pages[index] = (paddress & ~MMU_PAGEMASK) | (flags & MMU_PAGEMASK);

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

