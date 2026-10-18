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

    return MMU_TFLAG_PRESENT | MMU_TFLAG_ALWAYS1;

}

unsigned int mmu_pflags(unsigned int flags)
{

    if (!(flags & MMAP_FLAG_USERMODE))
        return MMU_PFLAG_PRESENT | MMU_PFLAG_KERNEL;

    if (!(flags & MMAP_FLAG_WRITEABLE))
        return MMU_PFLAG_PRESENT | MMU_PFLAG_USERREAD;

    return MMU_PFLAG_PRESENT | MMU_PFLAG_USERWRITE;

}

unsigned long mmu_gettable(unsigned long daddress, unsigned long vaddress)
{

    struct mmu_directory *directory = (struct mmu_directory *)daddress;

    return directory->tables[vaddress >> 20];

}

unsigned long mmu_getpage(unsigned long daddress, unsigned long vaddress)
{

    struct mmu_table *table = (struct mmu_table *)(mmu_gettable(daddress, vaddress) & ~MMU_TABLEMASK);

    if (table)
        return table->pages[(vaddress >> 12) & (MMU_PAGES - 1)];

    return 0;

}

void mmu_settable(unsigned long daddress, unsigned long vaddress, unsigned long taddress, unsigned int flags)
{

    struct mmu_directory *directory = (struct mmu_directory *)daddress;

    directory->tables[vaddress >> 20] = (taddress & ~MMU_TABLEMASK) | (flags & MMU_TABLEMASK);

}

void mmu_setpage(unsigned long daddress, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    struct mmu_table *table = (struct mmu_table *)(mmu_gettable(daddress, vaddress) & ~MMU_TABLEMASK);

    if (table)
        table->pages[(vaddress >> 12) & (MMU_PAGES - 1)] = (paddress & ~MMU_PAGEMASK) | (flags & MMU_PAGEMASK);

}

void mmu_enable(void)
{

    cpu_setdomains(CPU_DOMAIN0_CLIENT);
    cpu_setcontrol(cpu_getcontrol() | CPU_CONTROL_MMU);

}
