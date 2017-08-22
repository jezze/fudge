#include "pcf.h"

static unsigned short convert16(unsigned short value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP16(value) : value;

}

static unsigned int convert32(unsigned int value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP32(value) : value;

}

static struct pcf_entry *getentry(struct pcf_header *header, unsigned int type)
{

    struct pcf_entry *entries = (struct pcf_entry *)(header + 1);
    unsigned int i;

    for (i = 0; i < header->entries; i++)
    {

        if (entries[i].type == type)
            return &entries[i];

    }

    return 0;

}

static unsigned int getformat(void *base, struct pcf_entry *entry)
{

    return *((unsigned int *)((unsigned char *)base + entry->offset));

}

void pcf_readbitmap(void *base, struct pcf_bitmap *data)
{

    struct pcf_entry *entry = getentry(base, PCF_TYPE_BITMAPS);
    unsigned int format = getformat(base, entry);
    struct pcf_bitmap *bitmap = (struct pcf_bitmap *)((unsigned char *)base + entry->offset + sizeof (unsigned int));

    data->count = convert32(bitmap->count, format);

}

unsigned int pcf_getbitmapoffset(void *base, unsigned short index)
{

    struct pcf_entry *entry = getentry(base, PCF_TYPE_BITMAPS);
    unsigned int format = getformat(base, entry);
    struct pcf_bitmap bitmap;

    pcf_readbitmap(base, &bitmap);

    return convert32(((unsigned int *)((unsigned char *)base + entry->offset + sizeof (unsigned int) + sizeof (struct pcf_bitmap)))[index], format);

}

unsigned int *pcf_getbitmapsizes(void *base)
{

    struct pcf_entry *entry = getentry(base, PCF_TYPE_BITMAPS);
    struct pcf_bitmap bitmap;

    pcf_readbitmap(base, &bitmap);

    return (unsigned int *)((unsigned char *)base + entry->offset + sizeof (unsigned int) + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * bitmap.count);

}

unsigned char *pcf_getbitmapdata(void *base)
{

    struct pcf_entry *entry = getentry(base, PCF_TYPE_BITMAPS);
    struct pcf_bitmap bitmap;

    pcf_readbitmap(base, &bitmap);

    return (unsigned char *)((unsigned char *)base + entry->offset + sizeof (unsigned int) + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * bitmap.count + sizeof (unsigned int) * 4);

}

unsigned int pcf_getbitmapalign(void *base)
{

    struct pcf_entry *entry = getentry(base, PCF_TYPE_BITMAPS);
    unsigned int format = getformat(base, entry);

    return 1 << (format & 3);

}

void pcf_readmetricsdata(void *base, unsigned int index, struct pcf_metricsdata *data)
{

    struct pcf_entry *entry = getentry(base, PCF_TYPE_METRICS);
    unsigned int format = getformat(base, entry);

    if (format & PCF_FORMAT_COMPRESSED)
    {

        struct pcf_metricsdata_compressed *metrics = (struct pcf_metricsdata_compressed *)((unsigned char *)base + entry->offset + sizeof (unsigned int) + sizeof (struct pcf_metrics_compressed)) + index;

        data->lbearing = metrics->lbearing - 0x80;
        data->rbearing = metrics->rbearing - 0x80;
        data->width = metrics->width - 0x80;
        data->ascent = metrics->ascent - 0x80;
        data->descent = metrics->descent - 0x80;
        data->attributes = 0;

    }

    else
    {

        struct pcf_metricsdata *metrics = (struct pcf_metricsdata *)((unsigned char *)base + entry->offset + sizeof (unsigned int) + sizeof (struct pcf_metrics)) + index;

        data->lbearing = convert16(metrics->lbearing, format);
        data->rbearing = convert16(metrics->rbearing, format);
        data->width = convert16(metrics->width, format);
        data->ascent = convert16(metrics->ascent, format);
        data->descent = convert16(metrics->descent, format);
        data->attributes = convert16(metrics->attributes, format);

    }

}

void pcf_readbdfencoding(void *base, struct pcf_bdfencoding *data)
{

    struct pcf_entry *entry = getentry(base, PCF_TYPE_BDFENCODINGS);
    unsigned int format = getformat(base, entry);
    struct pcf_bdfencoding *bdfencoding = (struct pcf_bdfencoding *)((unsigned char *)base + entry->offset + sizeof (unsigned int));

    data->mincharorbyte2 = convert16(bdfencoding->mincharorbyte2, format);
    data->maxcharorbyte2 = convert16(bdfencoding->maxcharorbyte2, format);
    data->minbyte1 = convert16(bdfencoding->minbyte1, format);
    data->maxbyte1 = convert16(bdfencoding->maxbyte1, format);
    data->defaultchar = convert16(bdfencoding->defaultchar, format);

}

unsigned short pcf_getindex(void *base, unsigned short encoding)
{

    struct pcf_entry *entry = getentry(base, PCF_TYPE_BDFENCODINGS);
    unsigned int format = getformat(base, entry);
    struct pcf_bdfencoding bdfencoding;
    unsigned int index;

    pcf_readbdfencoding(base, &bdfencoding);

    if (encoding & 0xFF00)
    {

        char enc1 = encoding >> 8;
        char enc2 = encoding & 0xFF;

        index = (enc1 - bdfencoding.mincharorbyte2) * (bdfencoding.maxcharorbyte2 - bdfencoding.mincharorbyte2 + 1) + (enc2 - bdfencoding.mincharorbyte2);

    }

    else
    {

        index = encoding - bdfencoding.mincharorbyte2;

    }

    return convert16(((unsigned short *)((unsigned char *)base + entry->offset + sizeof (unsigned int) + sizeof (struct pcf_bdfencoding)))[index], format);

}

