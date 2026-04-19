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

unsigned int mmu_getdirectory(void)
{

    return cpu_getcr3();

}

unsigned int mmu_gettable(unsigned int directory, unsigned int vaddress)
{

    struct mmu_directory *d = (struct mmu_directory *)directory;
    unsigned int index = vaddress >> 22;

    return d->tables[index];

}

unsigned int mmu_getpage(unsigned int directory, unsigned int vaddress)
{

    struct mmu_table *t = (struct mmu_table *)(mmu_gettable(directory, vaddress) & ~MMU_PAGEMASK);

    if (t)
    {

        unsigned int index = (vaddress << 10) >> 22;

        return t->pages[index];

    }

    return 0;

}

void mmu_setdirectory(unsigned int directory)
{

    cpu_setcr3(directory);

}

void mmu_settable(unsigned int directory, unsigned int vaddress, unsigned int taddress, unsigned int flags)
{

    struct mmu_directory *d = (struct mmu_directory *)directory;
    unsigned int index = vaddress >> 22;

    d->tables[index] = (taddress & ~MMU_PAGEMASK) | (flags & MMU_PAGEMASK);

}

void mmu_setpage(unsigned int directory, unsigned int vaddress, unsigned int paddress, unsigned int flags)
{

    struct mmu_table *t = (struct mmu_table *)(mmu_gettable(directory, vaddress) & ~MMU_PAGEMASK);

    if (t)
    {

        unsigned int index = (vaddress << 10) >> 22;

        t->pages[index] = (paddress & ~MMU_PAGEMASK) | (flags & MMU_PAGEMASK);

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

