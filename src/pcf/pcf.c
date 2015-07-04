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

};

struct pcf_entry
{

    unsigned int type;
    unsigned int format;
    unsigned int size;
    unsigned int offset;

};

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

};

struct pcf_metrics
{

    unsigned int format;

};

struct pcf_metrics_normal
{

    unsigned int count;

};

struct pcf_metrics_compressed
{

    unsigned short count;

};

struct pcf_metricsdata_normal
{

    unsigned short lbearing;
    unsigned short rbearing;
    unsigned short width;
    unsigned short ascent;
    unsigned short descent;
    unsigned short attributes;

};

struct pcf_metricsdata_compressed
{

    unsigned char lbearing;
    unsigned char rbearing;
    unsigned char width;
    unsigned char ascent;
    unsigned char descent;
    unsigned char attributes;

};

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
static unsigned short glyphindices[256];
static unsigned int bitmapoffset[256];
static unsigned char bitmapdata[0x8000];

void writestring(unsigned int count, char *text)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int encoding = text[i] - convert16(bdfencoding.mincharorbyte2, bdfencoding.format);
        unsigned short glyphindex = convert16(glyphindices[encoding], bdfencoding.format);

        writenum(encoding, 10);
        writenum(glyphindex, 10);
        writedata(4, convert32(bitmapoffset[glyphindex], bitmap.format), 72, bitmapdata);

    }

}

void main()
{

    struct pcf_header header;
    struct pcf_entry entries[16];
    struct pcf_entry *bdfencodingentry;
    struct pcf_entry *bitmapentry;
    unsigned int bitmapsizes[4];
    unsigned int bitmapsize;
    unsigned int glyphcount;

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

    call_read(CALL_P0, bitmapentry->offset, sizeof (struct pcf_bitmap), 1, &bitmap);

    glyphcount = convert32(bitmap.count, bitmap.format);

    if (glyphcount > 256)
        return;

    call_read(CALL_P0, bitmapentry->offset + sizeof (struct pcf_bitmap) + sizeof (unsigned int), sizeof (unsigned int), glyphcount, &bitmapoffset);
    call_read(CALL_P0, bitmapentry->offset + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * glyphcount, sizeof (unsigned int), 4, bitmapsizes);

    bitmapsize = convert32(bitmapsizes[bitmap.format & 3], bitmap.format);

    if (bitmapsize > 0x8000)
        return;

    call_read(CALL_P0, bitmapentry->offset + sizeof (struct pcf_bitmap) + sizeof (unsigned int) * glyphcount + sizeof (unsigned int) * 4, sizeof (unsigned char), bitmapsize, bitmapdata);
    call_read(CALL_P0, bdfencodingentry->offset, sizeof (struct pcf_bdfencoding), 1, &bdfencoding);
    call_read(CALL_P0, bdfencodingentry->offset + sizeof (struct pcf_bdfencoding) - 4, sizeof (unsigned short), glyphcount, glyphindices);
    writestring(3, "A Hello World!");
    call_close(CALL_P0);

}

