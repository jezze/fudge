#include "mmap.h"

void mmap_setmapping(struct mmap_entry *entry, unsigned int paddress, unsigned int pagesize, unsigned int pagemask)
{

    entry->paddress = paddress;
    entry->vpaddress = entry->vaddress & ~pagemask;
    entry->vpsize = (entry->msize + pagesize) & ~pagemask;

}

void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned int address, unsigned int size, unsigned int fsize, unsigned int msize, unsigned int flags, unsigned int vaddress)
{

    entry->type = type;
    entry->address = address;
    entry->size = size;
    entry->fsize = fsize;
    entry->msize = msize;
    entry->flags = flags;
    entry->vaddress = vaddress;
    entry->paddress = 0;
    entry->vpaddress = 0;
    entry->vpsize = 0;

}

void mmap_initheader(struct mmap_header *header)
{

    header->entries = 0;
    header->offset = 0;

}
 
