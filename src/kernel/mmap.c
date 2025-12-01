#include "mmap.h"

void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags, unsigned int ioaddress, unsigned int iofsize, unsigned int iomsize, unsigned int ioflags)
{

    entry->type = type;
    entry->paddress = paddress;
    entry->vaddress = vaddress;
    entry->size = size;
    entry->flags = flags;
    entry->ioaddress = ioaddress;
    entry->iofsize = iofsize;
    entry->iomsize = iomsize;
    entry->ioflags = ioflags;

}

void mmap_initheader(struct mmap_header *header)
{

    header->entries = 0;
    header->offset = 0;
    header->ntables = 0;

}
 
