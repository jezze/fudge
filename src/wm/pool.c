#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "text.h"
#include "widget.h"
#include "pool.h"

#define MAX_WIDGETS                     1024
#define MAX_STRINGS                     512
#define MAX_FONTS                       32
#define STRINGDATA_SIZE                 0x4000
#define FONTDATA_SIZE                   0x8000

struct strindex
{

    unsigned int offset;
    unsigned int length;

};

struct entry
{

    struct widget widget;
    union
    {

        struct widget_button button;
        struct widget_choice choice;
        struct widget_layout layout;
        struct widget_fill fill;
        struct widget_grid grid;
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
static char strdata[STRINGDATA_SIZE];
static unsigned int strdataoffset;
static struct strindex strindex[MAX_STRINGS];
static struct text_font fonts[MAX_FONTS];
static unsigned char fontnormal[FONTDATA_SIZE];
static unsigned char fontbold[FONTDATA_SIZE];

struct list_item *pool_next(struct list_item *current)
{

    return (current) ? current->next : widgetlist.head;

}

struct list_item *pool_nextin(struct list_item *current, struct widget *parent)
{

    if (!pool_getcstringlength(parent->id))
        return 0;

    while ((current = pool_next(current)))
    {

        struct entry *entry = current->data;
        struct widget *widget = &entry->widget;

        if (!parent->source || widget->source == parent->source)
        {

            if (cstring_match(pool_getstring(widget->in), pool_getstring(parent->id)))
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

        if (cstring_match(pool_getstring(entry->widget.id), id))
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

static unsigned int findslot(void)
{

    unsigned int i;

    for (i = 1; i < MAX_STRINGS; i++)
    {

        struct strindex *s = &strindex[i];

        if (s->offset == 0 && s->length == 0)
            return i;

    }

    return 0;

}

char *pool_getstring(unsigned int index)
{

    struct strindex *s = &strindex[index];

    return strdata + s->offset;

}

unsigned int pool_getcstringlength(unsigned int index)
{

    struct strindex *s = &strindex[index];

    return (s->length) ? s->length - 1 : 0;

}

static unsigned int savedata(unsigned int count, void *data)
{

    unsigned int slot = findslot();

    if (slot)
    {

        struct strindex *s = &strindex[slot];

        s->offset = strdataoffset;
        s->length = count;
        strdataoffset += buffer_write(strdata, STRINGDATA_SIZE, data, s->length, s->offset);

    }

    return slot;

}

static unsigned int freedata(unsigned int index)
{

    struct strindex *s = &strindex[index];
    unsigned int offset = s->offset;
    unsigned int length = s->length;
    unsigned int next = s->offset + s->length;
    unsigned int count = strdataoffset - next;
    unsigned int i;

    buffer_write(strdata, STRINGDATA_SIZE, strdata + next, count, offset);
    strdataoffset -= length;

    s->offset = 0;
    s->length = 0;

    for (i = 0; i < MAX_STRINGS; i++)
    {

        struct strindex *current = &strindex[i];

        if (current->offset > offset)
            current->offset -= length;

    }

    return 0;

}

unsigned int pool_updatestring(unsigned int index, char *cstring)
{

    if (index)
        index = freedata(index);

    if (cstring)
        index = savedata(cstring_lengthzero(cstring), cstring);

    return index;

}

void pool_pcxload(struct pool_pcxresource *pcxresource, char *source)
{

    if (pcxresource->cached)
        return;

    if (file_walk2(FILE_L0, source))
    {

        struct pcx_header header;
        struct record record;
        unsigned char magic;

        file_stat(FILE_L0, &record);
        file_readall(FILE_L0, &header, sizeof (struct pcx_header));
        file_seekread(FILE_L0, pcxresource->data, 0x10000, 128);

        pcxresource->width = header.xend - header.xstart + 1;
        pcxresource->height = header.yend - header.ystart + 1;

        file_seekreadall(FILE_L0, &magic, 1, record.size - 768 - 1);

        if (magic == PCX_COLORMAP_MAGIC)
            file_readall(FILE_L0, pcxresource->colormap, 768);

        pcxresource->cached = 1;

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

void pool_setfont(unsigned int index, void *data, unsigned int lineheight, unsigned int padding)
{

    struct text_font *font = pool_getfont(index);

    font->data = data;
    font->bitmapdata = pcf_getbitmapdata(font->data);
    font->bitmapalign = pcf_getbitmapalign(font->data);
    font->lineheight = lineheight;
    font->padding = padding;

}

void pool_loadfont(unsigned int factor)
{

    unsigned int lineheight = 12 + factor * 4;
    unsigned int padding = 4 + factor * 2;

    switch (factor)
    {

    case 0:
        file_walk2(FILE_L0, "/data/font/ter-112n.pcf");
        file_walk2(FILE_L1, "/data/font/ter-112b.pcf");

        break;

    case 1:
        file_walk2(FILE_L0, "/data/font/ter-114n.pcf");
        file_walk2(FILE_L1, "/data/font/ter-114b.pcf");

        break;

    case 2:
        file_walk2(FILE_L0, "/data/font/ter-116n.pcf");
        file_walk2(FILE_L1, "/data/font/ter-116b.pcf");

        break;

    default:
        file_walk2(FILE_L0, "/data/font/ter-118n.pcf");
        file_walk2(FILE_L1, "/data/font/ter-118b.pcf");

        break;

    }

    file_read(FILE_L0, fontnormal, FONTDATA_SIZE);
    pool_setfont(POOL_FONTNORMAL, fontnormal, lineheight, padding);
    file_read(FILE_L1, fontbold, FONTDATA_SIZE);
    pool_setfont(POOL_FONTBOLD, fontbold, lineheight, padding);

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

