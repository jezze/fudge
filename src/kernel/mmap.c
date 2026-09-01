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

void mmap_register(struct mmap_header *header, struct mmap_entry *entry)
{

    struct mmap_entry *target = &header->entries[header->nentries];

    buffer_copy(target, entry, sizeof (struct mmap_entry));

    header->nentries++;

}

void mmap_setbinary(struct mmap_entry *entry, unsigned long ioaddress, unsigned int iofsize, unsigned int iomsize, unsigned int ioflags)
{

    entry->ioaddress = ioaddress;
    entry->iofsize = iofsize;
    entry->iomsize = iomsize;
    entry->ioflags = ioflags;

}

void mmap_setmailbox(struct mmap_entry *entry, unsigned int ichannel)
{

    entry->ichannel = 0;

}

void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags)
{

    entry->type = type;
    entry->paddress = paddress;
    entry->vaddress = vaddress;
    entry->size = size;
    entry->flags = flags;
    entry->ioaddress = 0;
    entry->iofsize = 0;
    entry->iomsize = 0;
    entry->ioflags = 0;
    entry->ichannel = 0;

}

void mmap_initheader(struct mmap_header *header)
{

    header->nentries = 0;
    header->ntables = 0;

    buffer_clear(header->entries, sizeof (struct mmap_entry) * 64);

}

