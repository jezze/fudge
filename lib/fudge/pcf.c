#include "pcf.h"

unsigned short pcf_convert16(unsigned short value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP16(value) : value;

}

unsigned int pcf_convert32(unsigned int value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP32(value) : value;

}

struct pcf_entry *pcf_findentry(struct pcf_header *header, struct pcf_entry *entries, unsigned int type)
{

    unsigned int i;

    for (i = 0; i < header->entries; i++)
    {

        struct pcf_entry *entry = &entries[i];

        if (entry->type == type)
            return entry;

    }

    return 0;

}

unsigned int pcf_getbdfoffset(struct pcf_entry *entry, struct pcf_bdfencoding *bdfencoding, unsigned int format, unsigned short encoding)
{

    unsigned short mincharorbyte2 = pcf_convert16(bdfencoding->mincharorbyte2, format);
    unsigned short maxcharorbyte2 = pcf_convert16(bdfencoding->maxcharorbyte2, format);
    unsigned int index;

    if (encoding & 0xFF00)
    {

        char enc1 = encoding >> 8;
        char enc2 = encoding & 0xFF;

        index = (enc1 - mincharorbyte2) * (maxcharorbyte2 - mincharorbyte2 + 1) + (enc2 - mincharorbyte2);

    }

    else
    {

        index = encoding - mincharorbyte2;

    }

    return index;

}

