#include <abi.h>
#include <fudge.h>
#include "lib.h"

static unsigned short convert16(unsigned short value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP16(value) : value;

}

static unsigned int convert32(unsigned int value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP32(value) : value;

}

unsigned int pcf_loadheader(unsigned int descriptor, struct pcf_header *header)
{

    return call_read(descriptor, 0, sizeof (struct pcf_header), 1, header);

}

unsigned int pcf_loadentry(unsigned int descriptor, unsigned int type, struct pcf_entry *entry)
{

    struct pcf_header header;
    struct pcf_entry entries[16];
    unsigned int i;

    pcf_loadheader(descriptor, &header);

    if (header.entries > 16)
        return 0;

    call_read(descriptor, sizeof (struct pcf_header), sizeof (struct pcf_entry), header.entries, entries);

    for (i = 0; i < header.entries; i++)
    {

        if (entries[i].type == type)
        {

            memory_copy(entry, &entries[i], sizeof (struct pcf_entry));

            return sizeof (struct pcf_entry);

        }

    }

    return 0;

}

void pcf_loadbitmap(unsigned int descriptor, struct pcf_entry *entry, struct pcf_bitmap *bitmap)
{

    call_read(descriptor, entry->offset, sizeof (struct pcf_bitmap), 1, bitmap);

    bitmap->count = convert32(bitmap->count, bitmap->format);

}

void pcf_loadbitmapdata(unsigned int descriptor, struct pcf_entry *entry, struct pcf_bitmap *bitmap, unsigned int total, unsigned char *bitmapdata)
{

    unsigned int bitmapsizes[4];
    unsigned int i;

    call_read(descriptor, entry->offset + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * bitmap->count, sizeof (unsigned int), 4, bitmapsizes);

    for (i = 0; i < 4; i++)
        bitmapsizes[i] = convert32(bitmapsizes[i], bitmap->format);

    if (bitmapsizes[bitmap->format & 3] > total)
        return;

    call_read(descriptor, entry->offset + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * bitmap->count + sizeof (unsigned int) * 4, sizeof (unsigned char), bitmapsizes[bitmap->format & 3], bitmapdata);

}

void pcf_loadmetrics(unsigned int descriptor, struct pcf_entry *entry, struct pcf_metrics *metrics)
{

    call_read(descriptor, entry->offset, sizeof (struct pcf_metrics), 1, metrics);

}

static void pcf_loadmetricsdatacompressed(unsigned int descriptor, struct pcf_entry *entry, struct pcf_metrics *metrics, unsigned int index, struct pcf_metricsdata_compressed *data)
{

    struct pcf_metrics_compressed metricscompressed;

    call_read(descriptor, entry->offset, sizeof (struct pcf_metrics_compressed), 1, &metricscompressed);

    metricscompressed.count = convert16(metricscompressed.count, metrics->format);

    call_read(descriptor, entry->offset + sizeof (struct pcf_metrics_compressed) + sizeof (struct pcf_metricsdata_compressed) * index, sizeof (struct pcf_metricsdata_compressed), 1, data);

    data->lbearing = data->lbearing - 0x80;
    data->rbearing = data->rbearing - 0x80;
    data->width = data->width - 0x80;
    data->ascent = data->ascent - 0x80;
    data->descent = data->descent - 0x80;

}

static void pcf_loadmetricsdatanormal(unsigned int descriptor, struct pcf_entry *entry, struct pcf_metrics *metrics, unsigned int index, struct pcf_metricsdata_normal *data)
{

    struct pcf_metrics_normal metricsnormal;

    call_read(descriptor, entry->offset, sizeof (struct pcf_metrics_normal), 1, &metricsnormal);

    metricsnormal.count = convert32(metricsnormal.count, metrics->format);

    call_read(descriptor, entry->offset + sizeof (struct pcf_metrics_normal) + sizeof (struct pcf_metricsdata_normal) * index, sizeof (struct pcf_metricsdata_normal), 1, data);

    data->lbearing = convert16(data->lbearing, metrics->format);
    data->rbearing = convert16(data->rbearing, metrics->format);
    data->width = convert16(data->width, metrics->format);
    data->ascent = convert16(data->ascent, metrics->format);
    data->descent = convert16(data->descent, metrics->format);
    data->attributes = convert16(data->attributes, metrics->format);

}

void pcf_loadmetricsdata(unsigned int descriptor, struct pcf_entry *entry, struct pcf_metrics *metrics, unsigned int index, struct pcf_metricsdata_normal *data)
{

    if (metrics->format & PCF_FORMAT_COMPRESSED)
    {

        struct pcf_metricsdata_compressed compressed;

        pcf_loadmetricsdatacompressed(descriptor, entry, metrics, index, &compressed);

        data->lbearing = compressed.lbearing;
        data->rbearing = compressed.rbearing;
        data->width = compressed.width;
        data->ascent = compressed.ascent;
        data->descent = compressed.descent;
        data->attributes = 0;

    }

    else
    {

        pcf_loadmetricsdatanormal(descriptor, entry, metrics, index, data);

    }

}

void pcf_loadbdfencoding(unsigned int descriptor, struct pcf_entry *entry, struct pcf_bdfencoding *bdfencoding)
{

    call_read(descriptor, entry->offset, sizeof (struct pcf_bdfencoding), 1, bdfencoding);

    bdfencoding->mincharorbyte2 = convert16(bdfencoding->mincharorbyte2, bdfencoding->format);
    bdfencoding->maxcharorbyte2 = convert16(bdfencoding->maxcharorbyte2, bdfencoding->format);
    bdfencoding->minbyte1 = convert16(bdfencoding->minbyte1, bdfencoding->format);
    bdfencoding->maxbyte1 = convert16(bdfencoding->maxbyte1, bdfencoding->format);
    bdfencoding->defaultchar = convert16(bdfencoding->defaultchar, bdfencoding->format);

}

unsigned int pcf_getbitmapoffset(unsigned int descriptor, unsigned short index)
{

    struct pcf_entry entry;
    struct pcf_bitmap bitmap;
    unsigned int bitmapoffset;

    pcf_loadentry(descriptor, PCF_TYPE_BITMAPS, &entry);
    pcf_loadbitmap(descriptor, &entry, &bitmap);
    call_read(descriptor, entry.offset + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * index, sizeof (unsigned int), 1, &bitmapoffset);

    return convert32(bitmapoffset, bitmap.format);

}

unsigned int pcf_getascent(unsigned int descriptor, unsigned short index)
{

    struct pcf_entry entry;
    struct pcf_metrics metrics;
    struct pcf_metricsdata_normal data;

    pcf_loadentry(descriptor, PCF_TYPE_METRICS, &entry);
    pcf_loadmetrics(descriptor, &entry, &metrics);
    pcf_loadmetricsdata(descriptor, &entry, &metrics, index, &data);

    return data.ascent;

}

unsigned int pcf_getdescent(unsigned int descriptor, unsigned short index)
{

    struct pcf_entry entry;
    struct pcf_metrics metrics;
    struct pcf_metricsdata_normal data;

    pcf_loadentry(descriptor, PCF_TYPE_METRICS, &entry);
    pcf_loadmetrics(descriptor, &entry, &metrics);
    pcf_loadmetricsdata(descriptor, &entry, &metrics, index, &data);

    return data.descent;

}

unsigned int pcf_getpadding(unsigned int descriptor)
{

    struct pcf_entry entry;
    struct pcf_bitmap bitmap;

    pcf_loadentry(descriptor, PCF_TYPE_BITMAPS, &entry);
    pcf_loadbitmap(descriptor, &entry, &bitmap);

    switch (bitmap.format & 3)
    {

        case 0:
            return 1;

        case 1:
            return 2;

        case 2:
            return 4;

    }

    return 0;

}

unsigned short pcf_getindex(unsigned int descriptor, unsigned short encoding)
{

    struct pcf_entry entry;
    struct pcf_bdfencoding bdfencoding;
    unsigned int index;
    unsigned short glyphindex;

    pcf_loadentry(descriptor, PCF_TYPE_BDFENCODINGS, &entry);
    pcf_loadbdfencoding(descriptor, &entry, &bdfencoding);

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

    call_read(descriptor, entry.offset + sizeof (struct pcf_bdfencoding) + sizeof (unsigned short) * index, sizeof (unsigned short), 1, &glyphindex);

    return convert16(glyphindex, bdfencoding.format);

}

void pcf_readdata(unsigned int descriptor, unsigned int count, void *buffer)
{

    struct pcf_entry entry;
    struct pcf_bitmap bitmap;

    pcf_loadentry(descriptor, PCF_TYPE_BITMAPS, &entry);
    pcf_loadbitmap(descriptor, &entry, &bitmap);
    pcf_loadbitmapdata(CALL_P0, &entry, &bitmap, count, buffer);

}

