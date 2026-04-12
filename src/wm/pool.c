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

static struct list widgetlist;
static struct list bumplist;
static struct list freelist;
static struct widget entries[MAX_WIDGETS];
static struct list_item items[MAX_WIDGETS];
static struct text_font fonts[MAX_FONTS];
static struct pool_pcxresource pcxresources[64];
static unsigned int npcxresources;

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

    if (!strpool_getcstringlength(parent->attributes.id))
        return 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (!parent->source || widget->source == parent->source)
        {

            if (cstring_match(strpool_getstring(widget->attributes.in), strpool_getstring(parent->attributes.id)))
                return current;

        }

    }

    return 0;

}

struct list_item *pool_nextsource(struct list_item *current, unsigned int source)
{

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (widget->source == source)
            return current;

    }

    return 0;

}

struct widget *pool_getwidgetbyid(unsigned int source, char *id)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (widget->source != source)
            continue;

        if (cstring_match(strpool_getstring(widget->attributes.id), id))
            return widget;

    }

    return 0;

}

static struct list_item *finditem(struct widget *widget)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct widget *w = current->data;

        if (w == widget)
            return current;

    }

    return 0;

}

static void bump(struct list_item *item)
{

    struct widget *widget = item->data;
    struct list_item *current = item->prev;

    list_move(&bumplist, &widgetlist, item);

    while ((current = pool_nextin(current, widget)))
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

        struct widget *widget = item->data;

        widget_init(widget, source, type);
        widget_setattribute(widget, ATTR_ID, id);
        widget_setattribute(widget, ATTR_IN, in);
        list_move(&widgetlist, &freelist, item);

        return widget;

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

struct pool_pcxresource *pool_createpcx(struct widget *widget, char *source)
{

    struct pool_pcxresource *resource = &pcxresources[npcxresources++];

    resource->target = fs_auth(source);

    if (resource->target)
    {

        resource->id = fs_walk(1, resource->target, 0, source);

        if (resource->id)
        {

            struct pcx_header header;
            struct record record;

            fs_stat(1, resource->target, resource->id, &record);
            fs_read_all(1, resource->target, resource->id, &header, sizeof (struct pcx_header), 0);

            if (header.version == 5)
            {

                buffer_copy(resource->colormap, header.palette, 48);

                if (record.size > sizeof (struct pcx_header) + 768 + 1)
                {

                    unsigned char magic;

                    fs_read_all(1, resource->target, resource->id, &magic, 1, record.size - 768 - 1);

                    if (magic == PCX_COLORMAP_MAGIC)
                        fs_read_all(1, resource->target, resource->id, resource->colormap, 768, record.size - 768);

                }

            }

            resource->width = header.xend - header.xstart + 1;
            resource->height = header.yend - header.ystart + 1;
            widget->size = util_size(resource->width, resource->height);

        }

    }

    return resource;

}

void pool_pcxreadline(struct pool_pcxresource *resource, int line, int y, unsigned char *buffer)
{

    unsigned char data[4096];

    if (resource->lastline == line - 1)
    {

        fs_read_full(1, resource->target, resource->id, data, 4096, 128 + resource->lastoffset);

        resource->lastoffset += pcx_readline(data, resource->width, buffer);

    }

    else
    {

        int h;

        resource->lastoffset = 0;

        for (h = 0; h < line - y + 1; h++)
        {

            fs_read_full(1, resource->target, resource->id, data, 4096, 128 + resource->lastoffset);

            resource->lastoffset += pcx_readline(data, resource->width, buffer);

        }


    }

    resource->lastline = line;

}

struct text_font *pool_getfont(unsigned int index)
{

    return &fonts[index];

}

void pool_setfont(unsigned int index, unsigned int lineheight, unsigned int padding)
{

    struct text_font *font = &fonts[index];

    font->lineheight = lineheight;
    font->padding = padding;

}

static void loadatlasdata(struct text_atlas *atlas, unsigned int target, unsigned int id, unsigned int offset, unsigned int align)
{

    unsigned int i;

    for (i = 0; i < atlas->height; i++)
        fs_read_full(1, target, id, atlas->bdata + i * atlas->width, atlas->width, offset + i * align);

}

static void loadatlas(struct text_font *font, unsigned int target, unsigned int id, struct pcf_entry *entries, unsigned int count)
{

    struct pcf_entry *bdfentry = pcf_findentry(entries, count, PCF_TYPE_BDFENCODINGS);
    struct pcf_entry *metricsentry = pcf_findentry(entries, count, PCF_TYPE_METRICS);
    struct pcf_entry *bitmapentry = pcf_findentry(entries, count, PCF_TYPE_BITMAPS);

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

            bdfoffset = pcf_getbdfoffset(bdfentry, &bdfencoding, i);

            fs_read_full(1, target, id, &index, sizeof (unsigned short), bdfentry->offset + 4 + sizeof (struct pcf_bdfencoding) + 2 * bdfoffset);

            index = pcf_convert16(index, bdfentry->format);

            if (metricsentry->format & PCF_FORMAT_COMPRESSED)
            {

                struct pcf_metricsdata_compressed metrics;

                fs_read_full(1, target, id, &metrics, sizeof (struct pcf_metricsdata_compressed), metricsentry->offset + 6 + sizeof (struct pcf_metricsdata_compressed) * index);

                atlas->width = metrics.width - 0x80;
                atlas->height = (metrics.ascent - 0x80) + (metrics.descent - 0x80);

            }

            else
            {

                struct pcf_metricsdata metrics;

                fs_read_full(1, target, id, &metrics, sizeof (struct pcf_metricsdata), metricsentry->offset + 8 + sizeof (struct pcf_metricsdata) * index);

                atlas->width = pcf_convert16(metrics.width, metricsentry->format);
                atlas->height = pcf_convert16(metrics.ascent, metricsentry->format) + pcf_convert16(metrics.descent, metricsentry->format);

            }

            fs_read_full(1, target, id, &bitmapoffset, sizeof (unsigned int), bitmapentry->offset + 8 + bitmapalign * index);

            bitmapoffset = pcf_convert32(bitmapoffset, bitmapentry->format);

            loadatlasdata(atlas, target, id, bitmapentry->offset + 8 + bitmapalign * bitmapcount + 4 * 4 + bitmapoffset, bitmapalign);

        }

    }

}

void pool_loadfont(unsigned int index, char *path)
{

    unsigned int target = fs_auth(path);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, path);

        if (id)
        {

            struct pcf_header header;

            fs_read_full(1, target, id, &header, sizeof (struct pcf_header), 0);

            if (buffer_match(header.magic, PCF_MAGIC, 4) && header.entries < 24)
            {

                struct text_font *font = &fonts[index];
                struct pcf_entry entries[24];

                fs_read_full(1, target, id, entries, sizeof (struct pcf_entry) * header.entries, sizeof (struct pcf_header));
                loadatlas(font, target, id, entries, header.entries);

            }

        }

    }

}

void pool_loadresources(void)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (widget->type == WIDGET_TYPE_IMAGE)
        {

            if (!widget->resource && widget->attributes.source && widget->attributes.mimetype)
            {

                switch (widget->attributes.mimetype)
                {

                case ATTR_MIMETYPE_PCX:
                    widget->resource = pool_createpcx(widget, strpool_getstring(widget->attributes.source));

                    break;

                }

            }

        }

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

