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

static unsigned int pcf_loadheader(unsigned int descriptor, struct pcf_header *header)
{

    return call_read(descriptor, 0, sizeof (struct pcf_header), 1, header);

}

static unsigned int pcf_loadentry(unsigned int descriptor, unsigned int type, struct pcf_entry *entry)
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

static unsigned int pcf_loadformat(unsigned int descriptor, unsigned int offset)
{

    unsigned int format;

    call_read(descriptor, offset, sizeof (unsigned int), 1, &format);

    return format;

}

static void pcf_loadbitmap(unsigned int descriptor, unsigned int offset, struct pcf_bitmap *bitmap)
{

    unsigned int format = pcf_loadformat(descriptor, offset);

    call_read(descriptor, offset + sizeof (unsigned int), sizeof (struct pcf_bitmap), 1, bitmap);

    bitmap->count = convert32(bitmap->count, format);

}

static void pcf_loadbitmapdata(unsigned int descriptor, unsigned int offset, unsigned int total, unsigned char *bitmapdata)
{

    unsigned int format = pcf_loadformat(descriptor, offset);
    struct pcf_bitmap bitmap;
    unsigned int bitmapsizes[4];
    unsigned int i;

    pcf_loadbitmap(descriptor, offset, &bitmap);
    call_read(descriptor, offset + sizeof (unsigned int) + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * bitmap.count, sizeof (unsigned int), 4, bitmapsizes);

    for (i = 0; i < 4; i++)
        bitmapsizes[i] = convert32(bitmapsizes[i], format);

    if (bitmapsizes[format & 3] > total)
        return;

    call_read(descriptor, offset + sizeof (unsigned int) + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * bitmap.count + sizeof (unsigned int) * 4, sizeof (unsigned char), bitmapsizes[format & 3], bitmapdata);

}

static void pcf_loadmetricsdatacompressed(unsigned int descriptor, unsigned int offset, unsigned int index, struct pcf_metricsdata_compressed *data)
{

    call_read(descriptor, offset + sizeof (unsigned int) + sizeof (struct pcf_metrics_compressed) + sizeof (struct pcf_metricsdata_compressed) * index, sizeof (struct pcf_metricsdata_compressed), 1, data);

    data->lbearing = data->lbearing - 0x80;
    data->rbearing = data->rbearing - 0x80;
    data->width = data->width - 0x80;
    data->ascent = data->ascent - 0x80;
    data->descent = data->descent - 0x80;

}

static void pcf_loadmetricsdatanormal(unsigned int descriptor, unsigned int offset, unsigned int index, struct pcf_metricsdata *data)
{

    unsigned int format = pcf_loadformat(descriptor, offset);
    struct pcf_metrics metricsnormal;

    call_read(descriptor, offset + sizeof (unsigned int), sizeof (struct pcf_metrics), 1, &metricsnormal);
    call_read(descriptor, offset + sizeof (unsigned int) + sizeof (struct pcf_metrics) + sizeof (struct pcf_metricsdata) * index, sizeof (struct pcf_metricsdata), 1, data);

    data->lbearing = convert16(data->lbearing, format);
    data->rbearing = convert16(data->rbearing, format);
    data->width = convert16(data->width, format);
    data->ascent = convert16(data->ascent, format);
    data->descent = convert16(data->descent, format);
    data->attributes = convert16(data->attributes, format);

}

static void pcf_loadmetricsdata(unsigned int descriptor, unsigned int offset, unsigned int index, struct pcf_metricsdata *data)
{

    unsigned int format = pcf_loadformat(descriptor, offset);

    if (format & PCF_FORMAT_COMPRESSED)
    {

        struct pcf_metricsdata_compressed compressed;

        pcf_loadmetricsdatacompressed(descriptor, offset, index, &compressed);

        data->lbearing = compressed.lbearing;
        data->rbearing = compressed.rbearing;
        data->width = compressed.width;
        data->ascent = compressed.ascent;
        data->descent = compressed.descent;
        data->attributes = 0;

    }

    else
    {

        pcf_loadmetricsdatanormal(descriptor, offset, index, data);

    }

}

static void pcf_loadbdfencoding(unsigned int descriptor, unsigned int offset, struct pcf_bdfencoding *bdfencoding)
{

    unsigned int format = pcf_loadformat(descriptor, offset);

    call_read(descriptor, offset + sizeof (unsigned int), sizeof (struct pcf_bdfencoding), 1, bdfencoding);

    bdfencoding->mincharorbyte2 = convert16(bdfencoding->mincharorbyte2, format);
    bdfencoding->maxcharorbyte2 = convert16(bdfencoding->maxcharorbyte2, format);
    bdfencoding->minbyte1 = convert16(bdfencoding->minbyte1, format);
    bdfencoding->maxbyte1 = convert16(bdfencoding->maxbyte1, format);
    bdfencoding->defaultchar = convert16(bdfencoding->defaultchar, format);

}

unsigned int pcf_getbitmapoffset(unsigned int descriptor, unsigned short index)
{

    struct pcf_entry entry;
    struct pcf_bitmap bitmap;
    unsigned int bitmapoffset;
    unsigned int format;

    pcf_loadentry(descriptor, PCF_TYPE_BITMAPS, &entry);

    format = pcf_loadformat(descriptor, entry.offset);

    pcf_loadbitmap(descriptor, entry.offset, &bitmap);
    call_read(descriptor, entry.offset + sizeof (unsigned int) + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * index, sizeof (unsigned int), 1, &bitmapoffset);

    return convert32(bitmapoffset, format);

}

void pcf_readmetrics(unsigned int descriptor, unsigned short index, struct pcf_metricsdata *data)
{

    struct pcf_entry entry;

    pcf_loadentry(descriptor, PCF_TYPE_METRICS, &entry);
    pcf_loadmetricsdata(descriptor, entry.offset, index, data);

}

unsigned int pcf_getpadding(unsigned int descriptor)
{

    struct pcf_entry entry;
    unsigned int format;

    pcf_loadentry(descriptor, PCF_TYPE_BITMAPS, &entry);

    format = pcf_loadformat(descriptor, entry.offset);

    return 1 << (format & 3);

}

unsigned short pcf_getindex(unsigned int descriptor, unsigned short encoding)
{

    struct pcf_entry entry;
    struct pcf_bdfencoding bdfencoding;
    unsigned int index;
    unsigned short glyphindex;
    unsigned int format;

    pcf_loadentry(descriptor, PCF_TYPE_BDFENCODINGS, &entry);

    format = pcf_loadformat(descriptor, entry.offset);

    pcf_loadbdfencoding(descriptor, entry.offset, &bdfencoding);

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

    call_read(descriptor, entry.offset + sizeof (unsigned int) + sizeof (struct pcf_bdfencoding) + sizeof (unsigned short) * index, sizeof (unsigned short), 1, &glyphindex);

    return convert16(glyphindex, format);

}

void pcf_readdata(unsigned int descriptor, unsigned int count, void *buffer)
{

    struct pcf_entry entry;

    pcf_loadentry(descriptor, PCF_TYPE_BITMAPS, &entry);
    pcf_loadbitmapdata(CALL_P0, entry.offset, count, buffer);

}

