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

unsigned int mmu_gettableaddress(unsigned int directory, unsigned int vaddress)
{

    return mmu_gettable(directory, vaddress) & ~MMU_PAGEMASK;

}

unsigned int mmu_gettableflags(unsigned int directory, unsigned int vaddress)
{

    return mmu_gettable(directory, vaddress) & MMU_PAGEMASK;

}

unsigned int mmu_getpage(unsigned int directory, unsigned int vaddress)
{

    struct mmu_table *t = (struct mmu_table *)mmu_gettableaddress(directory, vaddress);

    if (t)
    {

        unsigned int index = (vaddress << 10) >> 22;

        return t->pages[index];

    }

    return 0;

}

unsigned int mmu_getpageaddress(unsigned int directory, unsigned int vaddress)
{

    return mmu_getpage(directory, vaddress) & ~MMU_PAGEMASK;

}

unsigned int mmu_getpageflags(unsigned int directory, unsigned int vaddress)
{

    return mmu_getpage(directory, vaddress) & MMU_PAGEMASK;

}

void mmu_setdirectory(unsigned int directory)
{

    cpu_setcr3(directory);

}

void mmu_settable(unsigned int directory, unsigned int vaddress, unsigned int value)
{

    struct mmu_directory *d = (struct mmu_directory *)directory;
    unsigned int index = vaddress >> 22;

    d->tables[index] = value;

}

void mmu_settableaddress(unsigned int directory, unsigned int vaddress, unsigned int address)
{

    mmu_settable(directory, vaddress, address | mmu_gettableflags(directory, vaddress));

}

void mmu_settableflags(unsigned int directory, unsigned int vaddress, unsigned int flags)
{

    mmu_settable(directory, vaddress, mmu_gettableaddress(directory, vaddress) | flags);

}

void mmu_setpage(unsigned int directory, unsigned int vaddress, unsigned int value)
{

    struct mmu_table *t = (struct mmu_table *)mmu_gettableaddress(directory, vaddress);

    if (t)
    {

        unsigned int index = (vaddress << 10) >> 22;

        t->pages[index] = value;

    }

}

void mmu_setpageaddress(unsigned int directory, unsigned int vaddress, unsigned int address)
{

    mmu_setpage(directory, vaddress, address | mmu_getpageflags(directory, vaddress));

}

void mmu_setpageflags(unsigned int directory, unsigned int vaddress, unsigned int flags)
{

    mmu_setpage(directory, vaddress, mmu_getpageaddress(directory, vaddress) | flags);

}

void mmu_setflagrange(unsigned int directory, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
    {

        unsigned int v = vaddress + i;

        mmu_settableflags(directory, v, tflags);
        mmu_setpageflags(directory, v, pflags);

    }

}

void mmu_addflagrange(unsigned int directory, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
    {

        unsigned int v = vaddress + i;

        mmu_settableflags(directory, v, mmu_gettableflags(directory, v) | tflags);
        mmu_setpageflags(directory, v, mmu_getpageflags(directory, v) | pflags);

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

