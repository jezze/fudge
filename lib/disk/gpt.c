#include <fudge.h>
#include "gpt.h"

unsigned int gpt_read32(unsigned char *value)
{

    return (value[3] << 24) | (value[2] << 16) | (value[1] << 8) | value[0];

}

unsigned int gpt_isused(struct gpt_entry *entry)
{

    unsigned int i;

    for (i = 0; i < 16; i++)
    {

        if (entry->type[i])
            return 1;

    }

    return 0;

}

unsigned int gpt_validate(struct gpt_header *header)
{

    return buffer_match(header->signature, "EFI PART", 8) && gpt_read32(header->entrysize) >= sizeof (struct gpt_entry);

}
