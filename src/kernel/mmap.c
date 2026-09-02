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

void mmap_setbinary(struct mmap_entry *entry, unsigned long fbase, unsigned int fsize, unsigned int msize)
{

    entry->fbase = fbase;
    entry->fsize = fsize;
    entry->msize = msize;

}

void mmap_setmailbox(struct mmap_entry *entry, unsigned int itask, unsigned int ichannel)
{

    entry->itask = itask;
    entry->ichannel = ichannel;

}

void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags)
{

    entry->type = type;
    entry->paddress = paddress;
    entry->vaddress = vaddress;
    entry->size = size;
    entry->flags = flags;

}

struct mmap_entry *mmap_allocate(struct mmap_header *header, unsigned int type, unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags)
{

    if (header->nentries < MMAP_SLOTS)
    {

        struct mmap_entry *entry = &header->entries[header->nentries];

        mmap_initentry(entry, type, paddress, vaddress, size, flags);
        mmap_setbinary(entry, 0, 0, 0);
        mmap_setmailbox(entry, 0, 0);

        header->nentries++;

        return entry;

    }

    return 0;

}

void mmap_initheader(struct mmap_header *header)
{

    header->nentries = 0;
    header->ntables = 0;

    buffer_clear(header->entries, sizeof (struct mmap_entry) * 64);

}

