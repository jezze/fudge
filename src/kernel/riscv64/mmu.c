#include <fudge.h>
#include <kernel.h>
#include "mmu.h"

/* Sv39: entries hold the physical page number from bit 10 and the flags below it */
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

    return ((address >> 12) << 10) | (flags & MMU_FLAGMASK);

}

static unsigned long toaddress(unsigned long entry)
{

    return (entry >> 10) << 12;

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

/* a table entry without read, write or execute points at the next level */
unsigned int mmu_tflags(unsigned int flags)
{

    return MMU_TFLAG_PRESENT;

}

unsigned int mmu_pflags(unsigned int flags)
{

    unsigned int pflags = MMU_PFLAG_PRESENT | MMU_PFLAG_READ | MMU_PFLAG_EXECUTE | MMU_PFLAG_ACCESSED | MMU_PFLAG_DIRTY;

    if (flags & MMAP_FLAG_GLOBAL)
        pflags |= MMU_PFLAG_GLOBAL;

    if (flags & MMAP_FLAG_USERMODE)
        pflags |= MMU_PFLAG_USERMODE;

    if (flags & MMAP_FLAG_WRITEABLE)
        pflags |= MMU_PFLAG_WRITE;

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

/* taddress is either a table's address or, from copytable, another directory's entry */
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
        table->entries[getindex(vaddress, 0)] = (flags) ? toentry(paddress, flags) : 0;

}
