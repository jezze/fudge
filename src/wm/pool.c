#include <fudge.h>
#include <abi.h>
#include <font.h>
#include <hash.h>
#include <compression.h>
#include <image.h>
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"
#include "strpool.h"
#include "pool.h"

#define MAX_FONTS                       2

static struct list widgetlist;
static struct list bumplist;
static struct list freelist;
static struct widget entries[POOL_WIDGETS];
static struct list_item items[POOL_WIDGETS];
static struct list_item siblings[POOL_WIDGETS];
static struct list children[POOL_WIDGETS];
static struct text_font fonts[MAX_FONTS];
static struct pool_pcxresource pcxresources[POOL_PCXRESOURCES];
static struct pool_pngresource pngresources[POOL_PNGRESOURCES];
static struct pool_rawresource rawresources[POOL_RAWRESOURCES];
static unsigned int pngrow[POOL_PNGROWSIZE];
static struct png_checkpoint pngcheckpoints[POOL_PNGCHECKPOINTS];
static struct pool_pngresource *pngcheckpointowners[POOL_PNGCHECKPOINTS];

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

    return (current) ? current->next : children[parent - entries].head;

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
    unsigned int hash = djb_hash(cstring_length(id), id);

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (widget->idhash != hash || widget->source != source)
            continue;

        if (cstring_match(strpool_getstring(widget->attributes.id), id))
            return widget;

    }

    return 0;

}

static struct widget *findparent(struct widget *widget, unsigned int source)
{

    struct list_item *current = 0;
    unsigned int length = strpool_getcstringlength(widget->attributes.in);
    char *in = strpool_getstring(widget->attributes.in);
    unsigned int hash = djb_hash(length, in);

    while ((current = pool_next(current)))
    {

        struct widget *parent = current->data;

        if (parent == widget || parent->destroyed || parent->idhash != hash || parent->source != source)
            continue;

        if (strpool_getcstringlength(parent->attributes.id) == length && buffer_match(strpool_getstring(parent->attributes.id), in, length))
            return parent;

    }

    return 0;

}

/* a widget is placed in a widget of its own source first, and otherwise in one of the wm's */
void pool_setparent(struct widget *widget)
{

    if (widget->parent)
        list_remove(&children[widget->parent - entries], &siblings[widget - entries]);

    widget->parent = 0;

    if (!strpool_getcstringlength(widget->attributes.in))
        return;

    widget->parent = findparent(widget, widget->source);

    if (!widget->parent && widget->source)
        widget->parent = findparent(widget, 0);

    if (widget->parent)
        list_add(&children[widget->parent - entries], &siblings[widget - entries]);

}

/* a widget can name a parent that comes later in the same message */
void pool_setorphans(void)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (!widget->parent && !widget->destroyed)
            pool_setparent(widget);

    }

}

static struct list_item *finditem(struct widget *widget)
{

    return &items[widget - entries];

}

static void bump(struct widget *widget)
{

    struct list_item *current = 0;

    list_move(&bumplist, &widgetlist, finditem(widget));

    while ((current = pool_nextin(current, widget)))
        bump(current->data);

}

/* the texts in a widget count as one run of characters */
unsigned int pool_gettextlength(struct widget *widget)
{

    struct list_item *current = 0;
    unsigned int total = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
            total += strpool_getcstringlength(child->attributes.label);

    }

    return total;

}

/* finds the text an index into that run falls in, and the index inside it; past the end it is the end of the last text */
struct widget *pool_gettextat(struct widget *widget, unsigned int index, unsigned int *offset)
{

    struct list_item *current = 0;
    struct widget *text = 0;
    unsigned int base = 0;
    unsigned int start = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            unsigned int length = strpool_getcstringlength(child->attributes.label);

            text = child;
            start = base;

            if (index < base + length)
                break;

            base += length;

        }

    }

    if (text)
        *offset = util_min(index - util_min(index, start), strpool_getcstringlength(text->attributes.label));

    return text;

}

void pool_bump(struct widget *widget)
{

    if (widget->parent)
        list_move(&children[widget->parent - entries], &children[widget->parent - entries], &siblings[widget - entries]);

    bump(widget);

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
        pool_setparent(widget);

        return widget;

    }

    return 0;

}

static void destroypng(struct pool_pngresource *resource)
{

    unsigned int i;

    for (i = 0; i < POOL_PNGCHECKPOINTS; i++)
    {

        if (pngcheckpointowners[i] == resource)
            pngcheckpointowners[i] = 0;

    }

    resource->used = 0;

}

static void destroyraw(struct pool_rawresource *resource)
{

    if (resource->data)
        fs_munmap(1, resource->target, resource->id);

    resource->used = 0;

}

void pool_destroy(struct widget *widget)
{

    struct list *list = &children[widget - entries];

    if (widget->type == WIDGET_TYPE_IMAGE && widget->resource)
    {

        switch (widget->attributes.mimetype)
        {

        case ATTR_MIMETYPE_PCX:
            ((struct pool_pcxresource *)widget->resource)->used = 0;

            break;

        case ATTR_MIMETYPE_PNG:
            destroypng(widget->resource);

            break;

        case ATTR_MIMETYPE_RAW:
            destroyraw(widget->resource);

            break;

        }

    }

    if (widget->parent)
        list_remove(&children[widget->parent - entries], &siblings[widget - entries]);

    while (list->head)
    {

        struct widget *child = list->head->data;

        list_remove(list, list->head);

        child->parent = 0;

    }

    widget_unsetattributes(widget);
    list_move(&freelist, &widgetlist, finditem(widget));

}

struct pool_pcxresource *pool_createpcx(struct widget *widget, char *source)
{

    struct pool_pcxresource *resource = 0;
    unsigned int i;

    for (i = 0; i < POOL_PCXRESOURCES && !resource; i++)
    {

        if (!pcxresources[i].used)
            resource = &pcxresources[i];

    }

    if (!resource)
        return 0;

    buffer_clear(resource, sizeof (struct pool_pcxresource));

    resource->used = 1;
    resource->target = fs_auth(source);

    if (resource->target)
    {

        resource->id = fs_walk(1, resource->target, 0, source);

        if (resource->id)
        {

            struct pcx_header header;
            struct record record;

            fs_stat(1, resource->target, resource->id, &record);

            if (fs_read_all(1, resource->target, resource->id, &header, sizeof (struct pcx_header), 0) == sizeof (struct pcx_header) && header.identifier == PCX_IDENTIFIER && header.version == PCX_VERSION && header.encoding == PCX_ENCODING && header.bpp == 8 && header.nplanes == 1 && (unsigned short)header.bpl <= POOL_PCXROWSIZE && (unsigned short)header.xend - (unsigned short)header.xstart < (unsigned short)header.bpl)
            {

                buffer_copy(resource->colormap, header.palette, 48);

                if (record.size > sizeof (struct pcx_header) + 768 + 1)
                {

                    unsigned char magic;

                    fs_read_all(1, resource->target, resource->id, &magic, 1, record.size - 768 - 1);

                    if (magic == PCX_COLORMAP_MAGIC)
                        fs_read_all(1, resource->target, resource->id, resource->colormap, 768, record.size - 768);

                }

                resource->bpl = (unsigned short)header.bpl;
                resource->width = (unsigned short)header.xend - (unsigned short)header.xstart + 1;
                resource->height = (unsigned short)header.yend - (unsigned short)header.ystart + 1;
                resource->rowstep = resource->height / POOL_PCXOFFSETS + 1;
                widget->size = util_size(resource->width, resource->height);

            }

        }

    }

    return resource;

}

/* rows are decoded in order and the last one is kept, so a stretched image can draw it again; where every rowstep-th row starts is remembered, so a redraw higher up starts from the nearest one above instead of the top */
unsigned char *pool_pcxreadline(struct pool_pcxresource *resource, unsigned int row)
{

    unsigned char data[POOL_PCXROWSIZE * 2];

    if (row + 1 < resource->row)
    {

        unsigned int index = row / resource->rowstep;

        resource->row = index * resource->rowstep;
        resource->offset = resource->rowoffsets[index];

    }

    while (resource->row <= row)
    {

        if (resource->row % resource->rowstep == 0 && resource->row / resource->rowstep == resource->noffsets)
            resource->rowoffsets[resource->noffsets++] = resource->offset;

        fs_read_full(1, resource->target, resource->id, data, resource->bpl * 2, 128 + resource->offset);

        resource->offset += pcx_readline(data, resource->bpl, resource->rowdata);
        resource->row++;

    }

    return resource->rowdata;

}

/* the png comes first in the resource, so the reads find the file */
static unsigned int readpng(struct png *png, void *buffer, unsigned int count, unsigned int offset)
{

    struct pool_pngresource *resource = (struct pool_pngresource *)png;

    return fs_read_full(1, resource->target, resource->id, buffer, count, offset);

}

/* an image gets a checkpoint for every POOL_PNGSPACING bytes it inflates to, as far as there are free ones */
static void givecheckpoints(struct pool_pngresource *resource)
{

    struct png_checkpoint *checkpoints[PNG_CHECKPOINTS];
    unsigned int wanted = resource->png.height / (POOL_PNGSPACING / (resource->png.stride + 1));
    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < POOL_PNGCHECKPOINTS && count < wanted && count < PNG_CHECKPOINTS; i++)
    {

        if (!pngcheckpointowners[i])
        {

            pngcheckpointowners[i] = resource;
            checkpoints[count++] = &pngcheckpoints[i];

        }

    }

    png_setcheckpoints(&resource->png, checkpoints, count);

}

struct pool_pngresource *pool_createpng(struct widget *widget, char *source)
{

    struct pool_pngresource *resource = 0;
    unsigned int i;

    for (i = 0; i < POOL_PNGRESOURCES && !resource; i++)
    {

        if (!pngresources[i].used)
            resource = &pngresources[i];

    }

    if (!resource)
        return 0;

    buffer_clear(resource, sizeof (struct pool_pngresource));

    resource->used = 1;
    resource->target = fs_auth(source);
    resource->id = (resource->target) ? fs_walk(1, resource->target, 0, source) : 0;

    if (resource->id && png_open(&resource->png, readpng) && resource->png.width <= POOL_PNGROWSIZE)
    {

        resource->loaded = 1;
        widget->size = util_size(resource->png.width, resource->png.height);

        givecheckpoints(resource);

    }

    return resource;

}

unsigned int *pool_pngreadrow(struct pool_pngresource *resource, unsigned int row)
{

    return (png_readrow(&resource->png, row, pngrow)) ? pngrow : 0;

}

/* the pixels are another task's file, mapped as it is: width times height 32-bit pixels, which the file must hold */
struct pool_rawresource *pool_createraw(struct widget *widget, char *source)
{

    struct pool_rawresource *resource = 0;
    unsigned int width = widget->attributes.width;
    unsigned int height = widget->attributes.height;
    struct record record;
    unsigned int i;

    for (i = 0; i < POOL_RAWRESOURCES && !resource; i++)
    {

        if (!rawresources[i].used)
            resource = &rawresources[i];

    }

    if (!resource)
        return 0;

    buffer_clear(resource, sizeof (struct pool_rawresource));

    resource->used = 1;
    resource->target = fs_auth(source);
    resource->id = (resource->target) ? fs_walk(1, resource->target, 0, source) : 0;

    if (resource->id && width && height && width <= 0x4000 && height <= 0x4000 && fs_stat(1, resource->target, resource->id, &record) && record.size / 4 / width >= height)
    {

        resource->data = fs_mmap(1, resource->target, resource->id);

        if (resource->data)
            widget->size = util_size(width, height);

    }

    return resource;

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

        for (i = 0; i < TEXT_GLYPHS; i++)
        {

            struct text_atlas *atlas = &font->atlas[i];
            unsigned short index;
            unsigned int bdfoffset;
            unsigned int bitmapoffset;

            bdfoffset = pcf_getbdfoffset(bdfentry, &bdfencoding, i);

            fs_read_full(1, target, id, &index, sizeof (unsigned short), bdfentry->offset + 4 + sizeof (struct pcf_bdfencoding) + 2 * bdfoffset);

            index = pcf_convert16(index, bdfentry->format);

            if (index == 0xFFFF)
                continue;

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

                case ATTR_MIMETYPE_PNG:
                    widget->resource = pool_createpng(widget, strpool_getstring(widget->attributes.source));

                    break;

                case ATTR_MIMETYPE_RAW:
                    widget->resource = pool_createraw(widget, strpool_getstring(widget->attributes.source));

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

    for (i = 0; i < POOL_WIDGETS; i++)
    {

        struct list_item *item = &items[i];

        list_inititem(item, &entries[i]);
        list_inititem(&siblings[i], &entries[i]);
        list_init(&children[i]);
        list_add(&freelist, item);

    }

}

