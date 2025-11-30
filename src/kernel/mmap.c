#include "mmap.h"

void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned int address, unsigned int size, unsigned int fsize, unsigned int msize, unsigned int flags, unsigned int paddress, unsigned int vaddress, unsigned int pagesize, unsigned int pagemask)
{

    entry->type = type;
    entry->address = address;
    entry->size = size;
    entry->fsize = fsize;
    entry->msize = msize;
    entry->flags = flags;
    entry->paddress = paddress;
    entry->vaddress = vaddress;
    entry->vpaddress = entry->vaddress & ~pagemask;
    entry->vpsize = (entry->size + pagesize) & ~pagemask;

}

void mmap_initheader(struct mmap_header *header)
{

    header->entries = 0;
    header->offset = 0;

}
 
