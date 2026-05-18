#include <fudge.h>
#include "mmap.h"

struct mmap_entry *mmap_find(struct mmap_header *header, unsigned long vaddress)
{

    unsigned int i;

    for (i = 0; i < header->nentries; i++)
    {

        struct mmap_entry *entry = &header->entries[i];

        if (vaddress >= entry->vaddress && vaddress < entry->vaddress + entry->size)
            return entry;

    }

    return 0;

}

void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags, unsigned long ioaddress, unsigned int iofsize, unsigned int iomsize, unsigned int ioflags)
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

    header->nentries = 0;
    header->ntables = 0;

    buffer_clear(header->entries, sizeof (struct mmap_entry) * 64);

}

