#include <fudge.h>
#include <abi.h>
#include <font.h>
#include <hash.h>
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
static struct pool_rawresource rawresources[POOL_RAWRESOURCES];

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

struct widget *pool_gettextat(struct widget *widget, unsigned int index)
{

    struct list_item *current = 0;
    struct widget *text = 0;
    unsigned int base = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            unsigned int length = strpool_getcstringlength(child->attributes.label);

            text = child;

            if (index < base + length)
                break;

            base += length;

        }

    }

    return text;

}

unsigned int pool_gettextoffset(struct widget *widget, unsigned int index)
{

    struct widget *text = pool_gettextat(widget, index);
    struct list_item *current = 0;
    unsigned int start = 0;

    if (!text)
        return 0;

    while ((current = pool_nextin(current, widget)) && current->data != text)
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
            start += strpool_getcstringlength(child->attributes.label);

    }

    return util_min(index - util_min(index, start), strpool_getcstringlength(text->attributes.label));

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

        resource->data = fs_mmap(1, resource->target, resource->id, 0);

        if (resource->data)
            widget->size = util_size(width, height);

    }

    return resource;

}

struct text_font *pool_getfont(unsigned int index)
{

    return &fonts[index];

}

void pool_setfont(unsigned int index, unsigned int lineheight)
{

    struct text_font *font = &fonts[index];

    font->lineheight = lineheight;

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

            fs_read_full(1, target, id, &bitmapoffset, sizeof (unsigned int), bitmapentry->offset + 8 + 4 * index);

            bitmapoffset = pcf_convert32(bitmapoffset, bitmapentry->format);

            loadatlasdata(atlas, target, id, bitmapentry->offset + 8 + 4 * bitmapcount + 4 * 4 + bitmapoffset, bitmapalign);

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

            header.entries = pcf_convert32(header.entries, PCF_FORMAT_DEFAULT);

            if (buffer_match(header.magic, PCF_MAGIC, 4) && header.entries < 24)
            {

                struct text_font *font = &fonts[index];
                struct pcf_entry entries[24];
                unsigned int i;

                fs_read_full(1, target, id, entries, sizeof (struct pcf_entry) * header.entries, sizeof (struct pcf_header));

                for (i = 0; i < header.entries; i++)
                {

                    entries[i].type = pcf_convert32(entries[i].type, PCF_FORMAT_DEFAULT);
                    entries[i].format = pcf_convert32(entries[i].format, PCF_FORMAT_DEFAULT);
                    entries[i].size = pcf_convert32(entries[i].size, PCF_FORMAT_DEFAULT);
                    entries[i].offset = pcf_convert32(entries[i].offset, PCF_FORMAT_DEFAULT);

                }

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

