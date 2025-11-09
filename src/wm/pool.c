#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"
#include "strpool.h"
#include "pool.h"

#define MAX_WIDGETS                     1024
#define MAX_FONTS                       16

struct entry
{

    struct widget widget;
    union
    {

        struct widget_button button;
        struct widget_choice choice;
        struct widget_layout layout;
        struct widget_fill fill;
        struct widget_image image;
        struct widget_listbox listbox;
        struct widget_select select;
        struct widget_text text;
        struct widget_textbox textbox;
        struct widget_textbutton textbutton;
        struct widget_window window;

    } payload;

};

static struct list widgetlist;
static struct list bumplist;
static struct list freelist;
static struct entry entries[MAX_WIDGETS];
static struct list_item items[MAX_WIDGETS];
static struct text_font fonts[MAX_FONTS];
static char *fontn[] = {
    "initrd:data/font/ter-112n.pcf",
    "initrd:data/font/ter-114n.pcf",
    "initrd:data/font/ter-116n.pcf",
    "initrd:data/font/ter-118n.pcf",
};
static char *fontb[] = {
    "initrd:data/font/ter-112b.pcf",
    "initrd:data/font/ter-114b.pcf",
    "initrd:data/font/ter-116b.pcf",
    "initrd:data/font/ter-118b.pcf",
};

struct list_item *pool_prev(struct list_item *current)
{

    return (current) ? current->prev : widgetlist.tail;

}

struct list_item *pool_next(struct list_item *current)
{

    return (current) ? current->next : widgetlist.head;

}

struct list_item *pool_nextin(struct list_item *current, struct widget *parent)
{

    if (!strpool_getcstringlength(parent->id))
        return 0;

    while ((current = pool_next(current)))
    {

        struct entry *entry = current->data;
        struct widget *widget = &entry->widget;

        if (!parent->source || widget->source == parent->source)
        {

            if (cstring_match(strpool_getstring(widget->in), strpool_getstring(parent->id)))
                return current;

        }

    }

    return 0;

}

struct list_item *pool_nextsource(struct list_item *current, unsigned int source)
{

    while ((current = pool_next(current)))
    {

        struct entry *entry = current->data;

        if (entry->widget.source == source)
            return current;

    }

    return 0;

}

struct widget *pool_getwidgetbyid(unsigned int source, char *id)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct entry *entry = current->data;

        if (entry->widget.source != source)
            continue;

        if (cstring_match(strpool_getstring(entry->widget.id), id))
            return &entry->widget;

    }

    return 0;

}

static struct list_item *finditem(struct widget *widget)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct entry *entry = current->data;

        if (&entry->widget == widget)
            return current;

    }

    return 0;

}

static void bump(struct list_item *item)
{

    struct entry *entry = item->data;
    struct list_item *current = item->prev;

    list_move(&bumplist, &widgetlist, item);

    while ((current = pool_nextin(current, &entry->widget)))
    {

        struct list_item *prev = current->prev;

        bump(current);

        current = prev;

    }

}

void pool_bump(struct widget *widget)
{

    struct list_item *item = finditem(widget);

    if (item)
        bump(item);

    while (bumplist.head)
        list_move(&widgetlist, &bumplist, bumplist.head);

}

struct widget *pool_create(unsigned int source, unsigned int type, char *id, char *in)
{

    struct list_item *item = list_pickhead(&freelist);

    if (item)
    {

        struct entry *entry = item->data;

        widget_init(&entry->widget, source, type, id, in, &entry->payload);
        list_move(&widgetlist, &freelist, item);

        return &entry->widget;

    }

    return 0;

}

void pool_destroy(struct widget *widget)
{

    struct list_item *item = finditem(widget);

    if (item)
    {

        widget_unsetattributes(widget);
        list_move(&freelist, &widgetlist, item);

    }

}

void pool_pcxload(struct pool_pcxresource *pcxresource, char *source)
{

    if (!pcxresource->cached)
    {

        unsigned int target = fs_auth(source);

        if (target)
        {

            unsigned int id = fs_walk(1, target, 0, source);

            if (id)
            {

                struct pcx_header header;
                struct record record;
                unsigned char magic;

                fs_stat(1, target, id, &record);
                fs_read_all(1, target, id, &header, sizeof (struct pcx_header), 0);
                fs_read_full(1, target, id, pcxresource->data, 0x10000, 128);

                pcxresource->width = header.xend - header.xstart + 1;
                pcxresource->height = header.yend - header.ystart + 1;

                fs_read_all(1, target, id, &magic, 1, record.size - 768 - 1);

                if (magic == PCX_COLORMAP_MAGIC)
                    fs_read_all(1, target, id, pcxresource->colormap, 768, record.size - 768);

                pcxresource->cached = 1;

            }

        }

    }

}

void pool_pcxreadline(struct pool_pcxresource *pcxresource, int line, int y, unsigned char *buffer)
{

    if (pcxresource->lastline == line - 1)
    {

        pcxresource->lastoffset += pcx_readline(pcxresource->data + pcxresource->lastoffset, pcxresource->width, buffer);
        pcxresource->lastline = line;

    }

    else
    {

        int h;

        pcxresource->lastoffset = 0;

        for (h = 0; h < line - y + 1; h++)
            pcxresource->lastoffset += pcx_readline(pcxresource->data + pcxresource->lastoffset, pcxresource->width, buffer);

        pcxresource->lastline = line;

    }

}

struct text_font *pool_getfont(unsigned int index)
{

    return &fonts[index];

}

static void setfont(struct text_font *font, unsigned int lineheight, unsigned int padding)
{

    font->lineheight = lineheight;
    font->padding = padding;

}

static void loadatlasdata(struct text_atlas *atlas, unsigned int target, unsigned int id, unsigned int offset, unsigned int align)
{

    unsigned int i;

    for (i = 0; i < atlas->height; i++)
        fs_read_full(1, target, id, atlas->bdata + i * atlas->width, atlas->width, offset + i * align);

}

static void loadatlas(struct text_font *font, unsigned int target, unsigned int id, struct pcf_header *header, struct pcf_entry *entries)
{

    struct pcf_entry *bdfentry = pcf_findentry(header, entries, PCF_TYPE_BDFENCODINGS);
    struct pcf_entry *metricsentry = pcf_findentry(header, entries, PCF_TYPE_METRICS);
    struct pcf_entry *bitmapentry = pcf_findentry(header, entries, PCF_TYPE_BITMAPS);

    if (bdfentry && metricsentry && bitmapentry)
    {

        struct pcf_bdfencoding bdfencoding;
        unsigned int bitmapalign;
        unsigned int bitmapcount;
        unsigned int i;

        fs_read_full(1, target, id, &bdfencoding, sizeof (struct pcf_bdfencoding), bdfentry->offset + 4);
        fs_read_full(1, target, id, &bitmapcount, sizeof (unsigned int), bitmapentry->offset + 4);

        bitmapalign = 1 << (bitmapentry->format & PCF_FORMAT_PADMASK);
        bitmapcount = pcf_convert32(bitmapcount, bitmapentry->format);

        for (i = 0; i < 128; i++)
        {

            struct text_atlas *atlas = &font->atlas[i];
            unsigned short index;
            unsigned int bdfoffset;
            unsigned int bitmapoffset;

            bdfoffset = pcf_getbdfoffset(bdfentry, &bdfencoding, bdfentry->format, i);

            fs_read_full(1, target, id, &index, sizeof (unsigned short), bdfentry->offset + 4 + sizeof (struct pcf_bdfencoding) + 2 * bdfoffset);

            index = pcf_convert16(index, bdfentry->format);

            if (metricsentry->format & PCF_FORMAT_COMPRESSED)
            {

                struct pcf_metricsdata_compressed metrics;

                fs_read_full(1, target, id, &metrics, sizeof (struct pcf_metricsdata_compressed), metricsentry->offset + 6 + sizeof (struct pcf_metricsdata_compressed) * index);

                atlas->height = (metrics.ascent - 0x80) + (metrics.descent - 0x80);
                atlas->width = metrics.width - 0x80;

            }

            else
            {

                struct pcf_metricsdata metrics;

                fs_read_full(1, target, id, &metrics, sizeof (struct pcf_metricsdata), metricsentry->offset + 8 + sizeof (struct pcf_metricsdata) * index);

                atlas->height = pcf_convert16(metrics.ascent, metricsentry->format) + pcf_convert16(metrics.descent, metricsentry->format);
                atlas->width = pcf_convert16(metrics.width, metricsentry->format);

            }

            fs_read_full(1, target, id, &bitmapoffset, sizeof (unsigned int), bitmapentry->offset + 8 + bitmapalign * index);

            bitmapoffset = pcf_convert32(bitmapoffset, bitmapentry->format);

            loadatlasdata(atlas, target, id, bitmapentry->offset + 8 + bitmapalign * bitmapcount + 4 * 4 + bitmapoffset, bitmapalign);

        }

    }

}

static void loadfont(struct text_font *font, unsigned int target, unsigned int id, unsigned int lineheight, unsigned int padding)
{

    struct pcf_header header;

    fs_read_full(1, target, id, &header, sizeof (struct pcf_header), 0);

    if (buffer_match(header.magic, PCF_MAGIC, 4) && header.entries < 24)
    {

        struct pcf_entry entries[24];

        setfont(font, lineheight, padding);
        fs_read_full(1, target, id, entries, sizeof (struct pcf_entry) * header.entries, sizeof (struct pcf_header));
        loadatlas(font, target, id, &header, entries);

    }

}

void pool_loadfont(unsigned int factor)
{

    unsigned int target = fs_auth("initrd:");

    if (target)
    {

        unsigned int idn = fs_walk(1, target, 0, fontn[factor]);
        unsigned int idb = fs_walk(1, target, 0, fontb[factor]);
        unsigned int lineheight = 12 + factor * 4;
        unsigned int padding = 4 + factor * 2;

        if (idn)
            loadfont(&fonts[0], target, idn, lineheight, padding);

        if (idb)
            loadfont(&fonts[1], target, idb, lineheight, padding);

    }

}

void pool_setup(void)
{

    unsigned int i;

    list_init(&widgetlist);
    list_init(&bumplist);
    list_init(&freelist);

    for (i = 0; i < MAX_WIDGETS; i++)
    {

        struct list_item *item = &items[i];

        list_inititem(item, &entries[i]);
        list_add(&freelist, item);

    }

}

