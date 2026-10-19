#include <fudge.h>
#include <kernel.h>
#include "mmu.h"

/* the 68040's three levels with 4 KB pages: 128 root entries of 32 MB, 128 pointer entries of 256 KB, 64 pages */
static unsigned int *getroot(unsigned long root, unsigned long vaddress)
{

    return (unsigned int *)root + (vaddress >> 25);

}

static unsigned int *getpointer(unsigned long root, unsigned long vaddress)
{

    unsigned long pointers = mmu_getpointers(root, vaddress);

    return (pointers) ? (unsigned int *)pointers + ((vaddress >> 18) & 0x7F) : 0;

}

static unsigned int *getpage(unsigned long root, unsigned long vaddress)
{

    unsigned long pages = mmu_getpages(root, vaddress);

    return (pages) ? (unsigned int *)pages + ((vaddress >> 12) & 0x3F) : 0;

}

unsigned int mmu_pflags(unsigned int flags)
{

    if (!(flags & MMAP_FLAG_USERMODE))
        return MMU_PFLAG_PRESENT | MMU_PFLAG_SUPERVISOR;

    if (!(flags & MMAP_FLAG_WRITEABLE))
        return MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEPROTECT;

    return MMU_PFLAG_PRESENT;

}

unsigned long mmu_getpointers(unsigned long root, unsigned long vaddress)
{

    unsigned int entry = *getroot(root, vaddress);

    return (entry & MMU_TFLAG_PRESENT) ? entry & ~0x1FF : 0;

}

unsigned long mmu_getpages(unsigned long root, unsigned long vaddress)
{

    unsigned int *entry = getpointer(root, vaddress);

    return (entry && (*entry & MMU_TFLAG_PRESENT)) ? *entry & ~0xFF : 0;

}

unsigned long mmu_getpage(unsigned long root, unsigned long vaddress)
{

    unsigned int *entry = getpage(root, vaddress);

    return (entry) ? *entry : 0;

}

void mmu_setpointers(unsigned long root, unsigned long vaddress, unsigned long taddress)
{

    *getroot(root, vaddress) = taddress | MMU_TFLAG_PRESENT;

}

void mmu_setpages(unsigned long root, unsigned long vaddress, unsigned long taddress)
{

    *getpointer(root, vaddress) = taddress | MMU_TFLAG_PRESENT;

}

void mmu_setpage(unsigned long root, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    *getpage(root, vaddress) = (paddress & ~MMU_PAGEMASK) | flags;

}

