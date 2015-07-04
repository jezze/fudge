#include <abi.h>
#include <fudge.h>
#include "lib.h"

static struct pcf_bdfencoding bdfencoding;
static struct pcf_bitmap bitmap;
static struct pcf_metrics metrics;
static unsigned short glyphindices[256];
static unsigned int bitmapoffset[256];
static unsigned char bitmapdata[0x8000];
static struct pcf_metrics_normal metricsnormal;
static struct pcf_metricsdata_normal metricsdatanormal[256];
static struct pcf_metrics_compressed metricscompressed;
static struct pcf_metricsdata_compressed metricsdatacompressed[256];

struct pcf_entry *findentry(struct pcf_header *header, struct pcf_entry *entries, unsigned int type)
{

    unsigned int i;

    for (i = 0; i < header->entries; i++)
    {

        if (entries[i].type == type)
            return &entries[i];

    }

    return 0;

}

static unsigned short convert16(unsigned short value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP16(value) : value;

}

static unsigned int convert32(unsigned int value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP32(value) : value;

}

static void loadbitmap(unsigned int descriptor, unsigned int offset)
{

    unsigned int bitmapsizes[4];
    unsigned int i;

    call_read(descriptor, offset, sizeof (struct pcf_bitmap), 1, &bitmap);

    bitmap.count = convert32(bitmap.count, bitmap.format);

    if (bitmap.count > 256)
        return;

    call_read(descriptor, offset + sizeof (struct pcf_bitmap), sizeof (unsigned int), bitmap.count, &bitmapoffset);

    for (i = 0; i < bitmap.count; i++)
        bitmapoffset[i] = convert32(bitmapoffset[i], bitmap.format);

    call_read(descriptor, offset + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * bitmap.count, sizeof (unsigned int), 4, bitmapsizes);

    for (i = 0; i < 4; i++)
        bitmapsizes[i] = convert32(bitmapsizes[i], bitmap.format);

    if (bitmapsizes[bitmap.format & 3] > 0x8000)
        return;

    call_read(descriptor, offset + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * bitmap.count + sizeof (unsigned int) * 4, sizeof (unsigned char), bitmapsizes[bitmap.format & 3], bitmapdata);

}

static void loadmetrics(unsigned int descriptor, unsigned int offset)
{

    unsigned int i;

    call_read(descriptor, offset, sizeof (struct pcf_metrics), 1, &metrics);

    if (metrics.format & PCF_FORMAT_COMPRESSED)
    {

        call_read(descriptor, offset, sizeof (struct pcf_metrics_compressed), 1, &metricscompressed);

        metricscompressed.count = convert16(metricscompressed.count, metrics.format);

        if (metricscompressed.count > 256)
            return;

        call_read(descriptor, offset + sizeof (struct pcf_metrics_compressed), sizeof (struct pcf_metricsdata_compressed), metricscompressed.count, &metricsdatacompressed);

        for (i = 0; i < metricscompressed.count; i++)
        {

            metricsdatacompressed[i].lbearing = metricsdatacompressed[i].lbearing - 0x80;
            metricsdatacompressed[i].rbearing = metricsdatacompressed[i].rbearing - 0x80;
            metricsdatacompressed[i].width = metricsdatacompressed[i].width - 0x80;
            metricsdatacompressed[i].ascent = metricsdatacompressed[i].ascent - 0x80;
            metricsdatacompressed[i].descent = metricsdatacompressed[i].descent - 0x80;

        }

    }

    else
    {

        call_read(descriptor, offset, sizeof (struct pcf_metrics_normal), 1, &metricsnormal);

        metricsnormal.count = convert32(metricsnormal.count, metrics.format);

        if (metricsnormal.count > 256)
            return;

        call_read(descriptor, offset + sizeof (struct pcf_metrics_normal), sizeof (struct pcf_metricsdata_normal), metricsnormal.count, &metricsdatanormal);

        for (i = 0; i < metricsnormal.count; i++)
        {

            metricsdatanormal[i].lbearing = convert16(metricsdatanormal[i].lbearing, metrics.format);
            metricsdatanormal[i].rbearing = convert16(metricsdatanormal[i].rbearing, metrics.format);
            metricsdatanormal[i].width = convert16(metricsdatanormal[i].width, metrics.format);
            metricsdatanormal[i].ascent = convert16(metricsdatanormal[i].ascent, metrics.format);
            metricsdatanormal[i].descent = convert16(metricsdatanormal[i].descent, metrics.format);
            metricsdatanormal[i].attributes = convert16(metricsdatanormal[i].attributes, metrics.format);

        }

    }

}

static void loadbdfencoding(unsigned int descriptor, unsigned int offset)
{

    unsigned int count;
    unsigned int i;

    call_read(descriptor, offset, sizeof (struct pcf_bdfencoding), 1, &bdfencoding);

    bdfencoding.mincharorbyte2 = convert16(bdfencoding.mincharorbyte2, bdfencoding.format);
    bdfencoding.maxcharorbyte2 = convert16(bdfencoding.maxcharorbyte2, bdfencoding.format);
    bdfencoding.minbyte1 = convert16(bdfencoding.minbyte1, bdfencoding.format);
    bdfencoding.maxbyte1 = convert16(bdfencoding.maxbyte1, bdfencoding.format);
    bdfencoding.defaultchar = convert16(bdfencoding.defaultchar, bdfencoding.format);
    count = (bdfencoding.maxcharorbyte2 - bdfencoding.mincharorbyte2 + 1) * (bdfencoding.maxbyte1 - bdfencoding.minbyte1 + 1);

    if (count > 256)
        return;

    call_read(descriptor, offset + sizeof (struct pcf_bdfencoding), sizeof (unsigned short), count, glyphindices);

    for (i = 0; i < count; i++)
        glyphindices[i] = convert16(glyphindices[i], bdfencoding.format);

}

unsigned short pcf_getindex(unsigned short encoding)
{

    if (encoding & 0xFF00)
    {

        char enc1 = encoding >> 8;
        char enc2 = encoding & 0xFF;

        return glyphindices[(enc1 - bdfencoding.mincharorbyte2) * (bdfencoding.maxcharorbyte2 - bdfencoding.mincharorbyte2 + 1) + (enc2 - bdfencoding.mincharorbyte2)];

    }

    else
    {

        return glyphindices[encoding - bdfencoding.mincharorbyte2];

    }

}

unsigned int pcf_getascent(unsigned short index)
{

    return (metrics.format & PCF_FORMAT_COMPRESSED) ? metricsdatacompressed[index].ascent : metricsdatanormal[index].ascent;

}

unsigned int pcf_getdescent(unsigned short index)
{

    return (metrics.format & PCF_FORMAT_COMPRESSED) ? metricsdatacompressed[index].descent : metricsdatanormal[index].descent;

}

unsigned int pcf_getpadding()
{

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

void *pcf_getdata(unsigned short index)
{

    return bitmapdata + bitmapoffset[index];

}

void pcf_load(unsigned int descriptor)
{

    struct pcf_header header;
    struct pcf_entry entries[16];
    struct pcf_entry *bdfencodingentry;
    struct pcf_entry *bitmapentry;
    struct pcf_entry *metricsentry;

    call_open(descriptor);
    call_read(descriptor, 0, sizeof (struct pcf_header), 1, &header);

    if (header.entries > 16)
        return;

    call_read(descriptor, sizeof (struct pcf_header), sizeof (struct pcf_entry), header.entries, entries);

    bdfencodingentry = findentry(&header, entries, PCF_TYPE_BDFENCODINGS);

    if (!bdfencodingentry)
        return;

    bitmapentry = findentry(&header, entries, PCF_TYPE_BITMAPS);

    if (!bitmapentry)
        return;

    metricsentry = findentry(&header, entries, PCF_TYPE_METRICS);

    if (!metricsentry)
        return;

    loadbitmap(descriptor, bitmapentry->offset);
    loadmetrics(descriptor, metricsentry->offset);
    loadbdfencoding(descriptor, bdfencodingentry->offset);
    call_close(descriptor);

}

