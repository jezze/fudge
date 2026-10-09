#include <fudge.h>
#include <kernel.h>
#include "mmu.h"

struct mmu_table
{

    unsigned long entries[MMU_ENTRIES];

};

static unsigned int getindex(unsigned long vaddress, unsigned int level)
{

    return (vaddress >> (12 + 9 * level)) & (MMU_ENTRIES - 1);

}

static struct mmu_table *gettable(unsigned long daddress, unsigned long vaddress, unsigned int level)
{

    struct mmu_table *table = (struct mmu_table *)daddress;
    unsigned int i;

    for (i = MMU_LEVELS - 1; i > level; i--)
    {

        unsigned long entry = table->entries[getindex(vaddress, i)];

        if (!(entry & MMU_TFLAG_PRESENT))
            return 0;

        table = (struct mmu_table *)(entry & MMU_ADDRESSMASK);

    }

    return table;

}

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

unsigned long mmu_gettable(unsigned long daddress, unsigned long vaddress, unsigned int level)
{

    struct mmu_table *table = gettable(daddress, vaddress, level);

    return (table) ? table->entries[getindex(vaddress, level)] : 0;

}

unsigned long mmu_getpage(unsigned long daddress, unsigned long vaddress)
{

    struct mmu_table *table = gettable(daddress, vaddress, 0);

    return (table) ? table->entries[getindex(vaddress, 0)] : 0;

}

void mmu_settable(unsigned long daddress, unsigned long vaddress, unsigned int level, unsigned long taddress, unsigned int flags)
{

    struct mmu_table *table = gettable(daddress, vaddress, level);

    if (table)
        table->entries[getindex(vaddress, level)] = (taddress & MMU_ADDRESSMASK) | (flags & MMU_PAGEMASK);

}

void mmu_setpage(unsigned long daddress, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    struct mmu_table *table = gettable(daddress, vaddress, 0);

    if (table)
        table->entries[getindex(vaddress, 0)] = (paddress & MMU_ADDRESSMASK) | (flags & MMU_PAGEMASK);

}

