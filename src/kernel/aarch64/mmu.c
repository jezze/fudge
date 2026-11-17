#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "mmu.h"

/* 4 KB granule, 39-bit addresses: entries hold the physical address in bits 12-47 and the flags below it; memory below ARCH_RAMBASE is mapped as device memory */
struct mmu_table
{

    unsigned long entries[MMU_ENTRIES];

};

static unsigned int getindex(unsigned long vaddress, unsigned int level)
{

    return (vaddress >> (12 + 9 * level)) & (MMU_ENTRIES - 1);

}

static unsigned long toentry(unsigned long address, unsigned int flags)
{

    return (address & MMU_ADDRESSMASK) | (flags & MMU_FLAGMASK);

}

static unsigned long toaddress(unsigned long entry)
{

    return entry & MMU_ADDRESSMASK;

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

        table = (struct mmu_table *)toaddress(entry);

    }

    return table;

}

unsigned int mmu_tflags(unsigned int flags)
{

    return MMU_TFLAG_PRESENT | MMU_TFLAG_TABLE;

}

unsigned int mmu_pflags(unsigned int flags)
{

    unsigned int pflags = MMU_PFLAG_PRESENT | MMU_PFLAG_PAGE | MMU_PFLAG_SHARED | MMU_PFLAG_ACCESSED;

    if (!(flags & MMAP_FLAG_GLOBAL))
        pflags |= MMU_PFLAG_NOTGLOBAL;

    if (flags & MMAP_FLAG_USERMODE)
        pflags |= MMU_PFLAG_USERMODE;

    if (!(flags & MMAP_FLAG_WRITEABLE))
        pflags |= MMU_PFLAG_READONLY;

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
        table->entries[getindex(vaddress, level)] = (taddress & MMU_TFLAG_PRESENT) ? taddress : toentry(taddress, flags);

}

void mmu_setpage(unsigned long daddress, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    struct mmu_table *table = gettable(daddress, vaddress, 0);

    if (table)
        table->entries[getindex(vaddress, 0)] = (flags) ? toentry(paddress, flags | ((paddress < ARCH_RAMBASE) ? MMU_PFLAG_DEVICE : MMU_PFLAG_NORMAL)) : 0;

}

