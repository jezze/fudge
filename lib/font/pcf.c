#include "pcf.h"

unsigned short pcf_convert16(unsigned short value, unsigned int format)
{

    unsigned char *bytes = (unsigned char *)&value;

    return (format & PCF_FORMAT_BYTEMASK) ? (bytes[0] << 8) | bytes[1] : (bytes[1] << 8) | bytes[0];

}

unsigned int pcf_convert32(unsigned int value, unsigned int format)
{

    unsigned char *bytes = (unsigned char *)&value;

    if (format & PCF_FORMAT_BYTEMASK)
        return ((unsigned int)bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    else
        return ((unsigned int)bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];

}

struct pcf_entry *pcf_findentry(struct pcf_entry *entries, unsigned int count, unsigned int type)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct pcf_entry *entry = &entries[i];

        if (entry->type == type)
            return entry;

    }

    return 0;

}

unsigned int pcf_getbdfoffset(struct pcf_entry *entry, struct pcf_bdfencoding *bdfencoding, unsigned short encoding)
{

    unsigned short mincharorbyte2 = pcf_convert16(bdfencoding->mincharorbyte2, entry->format);
    unsigned short maxcharorbyte2 = pcf_convert16(bdfencoding->maxcharorbyte2, entry->format);
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

