#include <abi.h>
#include <fudge.h>

#define PCF_TYPE_PROPERTIES             (1 << 0)
#define PCF_TYPE_ACCELERATORS           (1 << 1)
#define PCF_TYPE_METRICS                (1 << 2)
#define PCF_TYPE_BITMAPS                (1 << 3)
#define PCF_TYPE_INKMETRICS             (1 << 4)
#define PCF_TYPE_BDFENCODINGS           (1 << 5)
#define PCF_TYPE_SWIDTHS                (1 << 6)
#define PCF_TYPE_GLYPHNAMES             (1 << 7)
#define PCF_TYPE_BDFACCELERATORS        (1 << 8)
#define PCF_FORMAT_DEFAULT              0x00000000
#define PCF_FORMAT_INKBOUNDS            0x00000200
#define PCF_FORMAT_ACCELINKBOUNDS       0x00000100
#define PCF_FORMAT_COMPRESSED           0x00000100
#define PCF_FORMAT_PADMASK              (3 << 0)
#define PCF_FORMAT_BYTEMASK             (1 << 2)
#define PCF_FORMAT_BITMASK              (1 << 3)
#define PCF_FORMAT_UNITMASK             (3 << 4)

#define BSWAP16(x)                      (((x) >> 8) | ((x) << 8))
#define BSWAP32(x)                      (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

static unsigned short convert16(unsigned short value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP16(value) : value;

}

static unsigned int convert32(unsigned int value, unsigned int format)
{

    return (format & PCF_FORMAT_BYTEMASK) ? BSWAP32(value) : value;

}

struct pcf_header
{

    unsigned char magic[4];
    unsigned int entries;

} __attribute__((packed));

struct pcf_entry
{

    unsigned int type;
    unsigned int format;
    unsigned int size;
    unsigned int offset;

} __attribute__((packed));

struct pcf_bitmap
{

    unsigned int format;
    unsigned int count;

};

struct pcf_bdfencoding
{

    unsigned int format;
    unsigned short mincharorbyte2;
    unsigned short maxcharorbyte2;
    unsigned short minbyte1;
    unsigned short maxbyte1;
    unsigned short defaultchar;

} __attribute__((packed));

struct pcf_metrics
{

    unsigned int format;

} __attribute__((packed));

struct pcf_metrics_normal
{

    struct pcf_metrics header;
    unsigned int count;

} __attribute__((packed));

struct pcf_metrics_compressed
{

    struct pcf_metrics header;
    unsigned short count;

} __attribute__((packed));

struct pcf_metricsdata_normal
{

    unsigned short lbearing;
    unsigned short rbearing;
    unsigned short width;
    unsigned short ascent;
    unsigned short descent;
    unsigned short attributes;

} __attribute__((packed));

struct pcf_metricsdata_compressed
{

    unsigned char lbearing;
    unsigned char rbearing;
    unsigned char width;
    unsigned char ascent;
    unsigned char descent;

} __attribute__((packed));

struct pcf_entry *pcf_findentry(struct pcf_header *header, struct pcf_entry *entries, unsigned int type)
{

    unsigned int i;

    for (i = 0; i < header->entries; i++)
    {

        if (entries[i].type == type)
            return &entries[i];

    }

    return 0;

}

static void writenum(unsigned int value, unsigned int base)
{

    char num[32];

    call_write(CALL_PO, 0, ascii_wvalue(num, 32, value, base, 0), 1, num);
    call_write(CALL_PO, 0, 1, 1, "\n");

}

static void writedata(unsigned int size, unsigned int offset, unsigned int count, unsigned char *buffer)
{

    char num[32];
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        call_write(CALL_PO, 0, ascii_wzerovalue(num, 32, buffer[offset + i], 16, 2, 0), 1, num);

        if (!((i + 1) % size))
            call_write(CALL_PO, 0, 2, 1, "  ");

    }

    call_write(CALL_PO, 0, 1, 1, "\n");

}

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

void writestring(unsigned int count, char *text)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int encoding = text[i] - bdfencoding.mincharorbyte2;
        unsigned short glyphindex = glyphindices[encoding];
        unsigned int lbearing = (metrics.format & PCF_FORMAT_COMPRESSED) ? metricsdatacompressed[glyphindex].lbearing : metricsdatanormal[glyphindex].lbearing;
        unsigned int rbearing = (metrics.format & PCF_FORMAT_COMPRESSED) ? metricsdatacompressed[glyphindex].rbearing : metricsdatanormal[glyphindex].rbearing;
        unsigned int width = (metrics.format & PCF_FORMAT_COMPRESSED) ? metricsdatacompressed[glyphindex].width : metricsdatanormal[glyphindex].width;
        unsigned int ascent = (metrics.format & PCF_FORMAT_COMPRESSED) ? metricsdatacompressed[glyphindex].ascent : metricsdatanormal[glyphindex].ascent;
        unsigned int descent = (metrics.format & PCF_FORMAT_COMPRESSED) ? metricsdatacompressed[glyphindex].descent : metricsdatanormal[glyphindex].descent;

        writenum(lbearing, 10);
        writenum(rbearing, 10);
        writenum(width, 10);
        writenum(ascent, 10);
        writenum(descent, 10);
        writedata(4, bitmapoffset[glyphindex], 72, bitmapdata);

    }

}

void loadbitmap(unsigned int descriptor, unsigned int offset)
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

void loadmetrics(unsigned int descriptor, unsigned int offset)
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

void loadbdfencoding(unsigned int descriptor, unsigned int offset)
{

    unsigned int i;

    call_read(descriptor, offset, sizeof (struct pcf_bdfencoding), 1, &bdfencoding);

    bdfencoding.mincharorbyte2 = convert16(bdfencoding.mincharorbyte2, bdfencoding.format);
    bdfencoding.maxcharorbyte2 = convert16(bdfencoding.maxcharorbyte2, bdfencoding.format);
    bdfencoding.minbyte1 = convert16(bdfencoding.minbyte1, bdfencoding.format);
    bdfencoding.maxbyte1 = convert16(bdfencoding.maxbyte1, bdfencoding.format);
    bdfencoding.defaultchar = convert16(bdfencoding.defaultchar, bdfencoding.format);

    if ((bdfencoding.maxcharorbyte2 - bdfencoding.mincharorbyte2 + 1) * (bdfencoding.maxbyte1 - bdfencoding.minbyte1 + 1) > 256)
        return;

    call_read(descriptor, offset + sizeof (struct pcf_bdfencoding), sizeof (unsigned short), (bdfencoding.maxcharorbyte2 - bdfencoding.mincharorbyte2 + 1) * (bdfencoding.maxbyte1 - bdfencoding.minbyte1 + 1), glyphindices);

    for (i = 0; i < (bdfencoding.maxcharorbyte2 - bdfencoding.mincharorbyte2 + 1) * (bdfencoding.maxbyte1 - bdfencoding.minbyte1 + 1); i++)
        glyphindices[i] = convert16(glyphindices[i], bdfencoding.format);

}

void main()
{

    struct pcf_header header;
    struct pcf_entry entries[16];
    struct pcf_entry *bdfencodingentry;
    struct pcf_entry *bitmapentry;
    struct pcf_entry *metricsentry;

    call_open(CALL_P0);
    call_read(CALL_P0, 0, sizeof (struct pcf_header), 1, &header);

    if (header.entries > 16)
        return;

    call_read(CALL_P0, sizeof (struct pcf_header), sizeof (struct pcf_entry), header.entries, entries);

    bdfencodingentry = pcf_findentry(&header, entries, PCF_TYPE_BDFENCODINGS);

    if (!bdfencodingentry)
        return;

    bitmapentry = pcf_findentry(&header, entries, PCF_TYPE_BITMAPS);

    if (!bitmapentry)
        return;

    metricsentry = pcf_findentry(&header, entries, PCF_TYPE_METRICS);

    if (!metricsentry)
        return;

    loadbitmap(CALL_P0, bitmapentry->offset);
    loadmetrics(CALL_P0, metricsentry->offset);
    loadbdfencoding(CALL_P0, bdfencodingentry->offset);
    writestring(3, "A Hello World!");
    call_close(CALL_P0);

}

