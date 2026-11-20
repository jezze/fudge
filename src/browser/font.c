#include <fudge.h>
#include <font.h>
#include "font.h"

#define PCF_ENTRIES                     24

static unsigned int sizes[FONT_SIZES] = {12, 14, 16, 18};

static unsigned int mapcodepoint(unsigned int codepoint)
{

    if (codepoint < 0x80 || (codepoint >= 0xA0 && codepoint < FONT_GLYPHS))
        return codepoint;

    switch (codepoint)
    {

    case 0x20AC:
        return 0x80;

    case 0x2018:
    case 0x2019:
    case 0x201A:
    case 0x2032:
        return '\'';

    case 0x201C:
    case 0x201D:
    case 0x201E:
    case 0x2033:
        return '"';

    case 0x2010:
    case 0x2011:
    case 0x2012:
    case 0x2013:
    case 0x2014:
    case 0x2015:
    case 0x2212:
        return '-';

    case 0x2022:
    case 0x2026:
    case 0x22C5:
        return 0xB7;

    case 0x2039:
    case 0x2190:
        return '<';

    case 0x203A:
    case 0x2192:
        return '>';

    case 0x2002:
    case 0x2003:
    case 0x2009:
    case 0x200A:
    case 0x202F:
        return ' ';

    }

    return '?';

}

struct font_glyph *font_getglyph(struct font_face *face, unsigned int codepoint)
{

    return &face->glyphs[mapcodepoint(codepoint)];

}

struct font_choice font_choose(unsigned int size, unsigned int bold)
{

    struct font_choice choice;
    unsigned int best = 0xFFFF;
    unsigned int scale;
    unsigned int i;

    choice.face = 0;
    choice.scale = 1;

    for (scale = 1; scale <= FONT_SCALES; scale++)
    {

        for (i = 0; i < FONT_SIZES; i++)
        {

            unsigned int height = sizes[i] * scale;
            unsigned int distance = (height > size) ? height - size : size - height;

            if (distance < best)
            {

                best = distance;
                choice.face = i;
                choice.scale = scale;

            }

        }

    }

    if (bold)
        choice.face += FONT_SIZES;

    return choice;

}

static unsigned int readat(void *out, unsigned int count, char *data, unsigned int size, unsigned int offset)
{

    if (offset > size || count > size - offset)
    {

        buffer_clear(out, count);

        return 0;

    }

    buffer_copy(out, data + offset, count);

    return 1;

}

static void loadglyph(struct font_glyph *glyph, char *data, unsigned int size, unsigned int offset, unsigned int align)
{

    unsigned int rowbytes = (glyph->width + 7) / 8;
    unsigned int i;

    if (rowbytes > FONT_ROWBYTES)
        rowbytes = FONT_ROWBYTES;

    if (glyph->height > FONT_ROWS)
        glyph->height = FONT_ROWS;

    for (i = 0; i < glyph->height; i++)
        readat(glyph->rows + i * FONT_ROWBYTES, rowbytes, data, size, offset + i * align);

}

static void loadglyphs(struct font_face *face, char *data, unsigned int size, struct pcf_entry *entries, unsigned int count)
{

    struct pcf_entry *bdfentry = pcf_findentry(entries, count, PCF_TYPE_BDFENCODINGS);
    struct pcf_entry *metricsentry = pcf_findentry(entries, count, PCF_TYPE_METRICS);
    struct pcf_entry *bitmapentry = pcf_findentry(entries, count, PCF_TYPE_BITMAPS);
    struct pcf_bdfencoding bdfencoding;
    unsigned int bitmapalign;
    unsigned int bitmapcount;
    unsigned int i;

    if (!bdfentry || !metricsentry || !bitmapentry)
        return;

    readat(&bdfencoding, sizeof (struct pcf_bdfencoding), data, size, bdfentry->offset + 4);
    readat(&bitmapcount, sizeof (unsigned int), data, size, bitmapentry->offset + 4);

    bitmapalign = 1 << (bitmapentry->format & PCF_FORMAT_PADMASK);
    bitmapcount = pcf_convert32(bitmapcount, bitmapentry->format);

    for (i = 0; i < FONT_GLYPHS; i++)
    {

        struct font_glyph *glyph = &face->glyphs[i];
        unsigned int ascent;
        unsigned short index;
        unsigned int bitmapoffset;

        if (!readat(&index, sizeof (unsigned short), data, size, bdfentry->offset + 4 + sizeof (struct pcf_bdfencoding) + 2 * pcf_getbdfoffset(bdfentry, &bdfencoding, i)))
            continue;

        index = pcf_convert16(index, bdfentry->format);

        if (index == 0xFFFF)
            continue;

        if (metricsentry->format & PCF_FORMAT_COMPRESSED)
        {

            struct pcf_metricsdata_compressed metrics;

            readat(&metrics, sizeof (struct pcf_metricsdata_compressed), data, size, metricsentry->offset + 6 + sizeof (struct pcf_metricsdata_compressed) * index);

            glyph->width = metrics.width - 0x80;
            ascent = metrics.ascent - 0x80;
            glyph->height = ascent + (metrics.descent - 0x80);

        }

        else
        {

            struct pcf_metricsdata metrics;

            readat(&metrics, sizeof (struct pcf_metricsdata), data, size, metricsentry->offset + 8 + sizeof (struct pcf_metricsdata) * index);

            glyph->width = pcf_convert16(metrics.width, metricsentry->format);
            ascent = pcf_convert16(metrics.ascent, metricsentry->format);
            glyph->height = ascent + pcf_convert16(metrics.descent, metricsentry->format);

        }

        if (glyph->height > face->height)
        {

            face->height = glyph->height;
            face->ascent = ascent;

        }

        readat(&bitmapoffset, sizeof (unsigned int), data, size, bitmapentry->offset + 8 + 4 * index);

        bitmapoffset = pcf_convert32(bitmapoffset, bitmapentry->format);

        loadglyph(glyph, data, size, bitmapentry->offset + 8 + 4 * bitmapcount + 4 * 4 + bitmapoffset, bitmapalign);

    }

}

unsigned int font_load(struct font_face *face, char *data, unsigned int size)
{

    struct pcf_header header;
    struct pcf_entry entries[PCF_ENTRIES];
    unsigned int i;

    buffer_clear(face, sizeof (struct font_face));
    readat(&header, sizeof (struct pcf_header), data, size, 0);

    header.entries = pcf_convert32(header.entries, PCF_FORMAT_DEFAULT);

    if (!buffer_match(header.magic, PCF_MAGIC, 4) || header.entries >= PCF_ENTRIES)
        return 0;

    readat(entries, sizeof (struct pcf_entry) * header.entries, data, size, sizeof (struct pcf_header));

    for (i = 0; i < header.entries; i++)
    {

        entries[i].type = pcf_convert32(entries[i].type, PCF_FORMAT_DEFAULT);
        entries[i].format = pcf_convert32(entries[i].format, PCF_FORMAT_DEFAULT);
        entries[i].size = pcf_convert32(entries[i].size, PCF_FORMAT_DEFAULT);
        entries[i].offset = pcf_convert32(entries[i].offset, PCF_FORMAT_DEFAULT);

    }

    loadglyphs(face, data, size, entries, header.entries);

    return face->height != 0;

}
