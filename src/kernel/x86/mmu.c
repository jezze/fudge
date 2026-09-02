#include <fudge.h>
#include <kernel.h>
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

unsigned int mmu_tflags(unsigned int flags)
{

    unsigned int tflags = MMU_TFLAG_PRESENT;

    if (flags & MMAP_FLAG_GLOBAL)
        tflags |= MMU_TFLAG_GLOBAL;

    if (flags & MMAP_FLAG_USERMODE)
        tflags |= MMU_TFLAG_USERMODE;

    if (flags & MMAP_FLAG_WRITEABLE)
        tflags |= MMU_TFLAG_WRITEABLE;

    if (flags & MMAP_FLAG_WRITETHROUGH)
        tflags |= MMU_TFLAG_WRITETHROUGH;

    return tflags;

}

unsigned int mmu_pflags(unsigned int flags)
{

    unsigned int pflags = MMU_PFLAG_PRESENT;

    if (flags & MMAP_FLAG_GLOBAL)
        pflags |= MMU_PFLAG_GLOBAL;

    if (flags & MMAP_FLAG_USERMODE)
        pflags |= MMU_PFLAG_USERMODE;

    if (flags & MMAP_FLAG_WRITEABLE)
        pflags |= MMU_PFLAG_WRITEABLE;

    if (flags & MMAP_FLAG_WRITETHROUGH)
        pflags |= MMU_PFLAG_WRITETHROUGH;

    return pflags;

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

    cpu_setcr0(cpu_getcr0() | CPU_CR0_PG);

}

void mmu_disable(void)
{

    cpu_setcr0(cpu_getcr0() & ~CPU_CR0_PG);

}

