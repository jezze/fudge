#include <abi.h>
#include <fudge.h>
#include <pcf/pcf.h>
#include "box.h"
#include "widget.h"
#include "ev.h"

#define COLOR_DARK                      0x00
#define COLOR_LIGHT                     0x01
#define COLOR_BODY                      0x02
#define COLOR_PASSIVEBACK               0x03
#define COLOR_PASSIVEFRAME              0x04
#define COLOR_ACTIVEBACK                0x05
#define COLOR_ACTIVEFRAME               0x06
#define COLOR_POINTERBACK               0x07
#define COLOR_POINTERFRAME              0x08
#define COLOR_TEXTNORMAL                0x09
#define COLOR_TEXTLIGHT                 0x0A
#define LAYERS                          2

struct layer
{

    unsigned char *data;
    unsigned int count;
    unsigned int total;

};

struct font
{

    unsigned char *data;
    unsigned char *bitmapdata;
    unsigned int padding;
    unsigned int lineheight;

};

static void (*drawables[6])(void *data, unsigned int line);
static void (*paint)(unsigned int color, unsigned int offset, unsigned int count);
static unsigned char textcolor[2];
static unsigned char drawdata[0x2000];
static unsigned char layerdata1[0x8000];
static unsigned char layerdata2[0x1000];
static struct layer layers[LAYERS] = {
    {layerdata1, 0, 0x8000},
    {layerdata2, 0, 0x1000}
};
static unsigned char fontdata[0x8000];
static struct font font = {fontdata, 0, 0, 0};
static unsigned char mousedata24[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
unsigned char mousedata16[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00,
    0x00, 0x08, 0x07, 0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x08, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static unsigned char colormap8[] = {
    0x00, 0x00, 0x00,
    0x3F, 0x3F, 0x3F,
    0x04, 0x02, 0x02,
    0x06, 0x04, 0x04,
    0x08, 0x06, 0x06,
    0x08, 0x10, 0x18,
    0x0C, 0x14, 0x1C,
    0x28, 0x10, 0x18,
    0x38, 0x20, 0x28,
    0x1C, 0x18, 0x18,
    0x3F, 0x3F, 0x3F
};
static unsigned int colormap32[] = {
    0xFF000000,
    0xFFFFFFFF,
    0xFF181014,
    0xFF20181C,
    0xFF30282C,
    0xFF105070,
    0xFF307090,
    0xFFB05070,
    0xFFF898B8,
    0xFF80787C,
    0xFFFFFFFF
};

static unsigned int findrowcount(unsigned int offset, unsigned int count, char *string)
{

    unsigned int i;

    for (i = offset; i < count; i++)
    {

        if (string[i] == '\n')
            return i + 1;

    }

    return count;

}

static unsigned int findrowstart(unsigned int row, unsigned int count, char *string)
{

    unsigned int i;

    if (!row)
        return 0;

    for (i = 0; i < count; i++)
    {

        if (string[i] == '\n')
        {

            if (!--row)
                return i + 1;

        }

    }

    return 0;

}

static void paint8(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *buffer = drawdata + offset;

    while (count--)
        *buffer++ = color;

}

static void paint32(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int *buffer = (unsigned int *)drawdata + offset;

    while (count--)
        *buffer++ = colormap32[color];

}

static void paintbuffer(unsigned char *buffer, unsigned int x, unsigned int w, unsigned char transparent)
{

    unsigned int i;

    for (i = 0; i < w; i++)
    {

        if (buffer[i] != transparent)
            paint(buffer[i], x + i, 1);

    }

}

static void paintcharline(unsigned int x, unsigned int w, unsigned char color, unsigned char *data)
{

    unsigned int i;

    for (i = 0; i < w; i++)
    {

        if (data[(i >> 3)] & (0x80 >> (i % 8)))
            paint(color, x + i, 1);

    }

}

static void paintcharlineinverted(unsigned int x, unsigned int w, unsigned char color, unsigned char *data)
{

    unsigned int i;

    for (i = 0; i < w; i++)
    {

        if (!(data[(i >> 3)] & (0x80 >> (i % 8))))
            paint(color, x + i, 1);

    }

}

static void painttext(char *string, unsigned int length, unsigned char color, unsigned int line, struct box *from, unsigned int flow, unsigned int cursor)
{

    unsigned int rowindex = line / font.lineheight;
    unsigned int rowline = line % font.lineheight;
    unsigned int rowstart;
    unsigned int rowcount;
    struct box size;
    unsigned int i;

    rowstart = findrowstart(rowindex, length, string);

    if (!rowstart && rowindex > 0)
        return;

    rowcount = findrowcount(rowstart, length, string);

    if (!rowcount)
        return;

    size.x = from->x;
    size.y = from->y + rowindex * font.lineheight;

    for (i = rowstart; i < rowcount; i++)
    {

        unsigned short index = (string[i] == '\n') ? pcf_getindex(font.data, ' ') : pcf_getindex(font.data, string[i]);
        unsigned char *data = font.bitmapdata + pcf_getbitmapoffset(font.data, index) + rowline * font.padding;
        struct pcf_metricsdata metricsdata;

        pcf_readmetricsdata(font.data, index, &metricsdata);

        size.w = metricsdata.width;
        size.h = metricsdata.ascent + metricsdata.descent;

        if (size.x + size.w > from->x + from->w)
            return;

        if (size.y + size.h > from->y + from->h)
            return;

        if (rowline < size.h)
        {

            if (flow == WIDGET_TEXTFLOW_INPUT && i == cursor)
                paintcharlineinverted(size.x, size.w, color, data);
            else
                paintcharline(size.x, size.w, color, data);

        }

        size.x += size.w;

    }

}

static void paintframe(unsigned int framecolor, struct box *size, unsigned int line)
{

    if (line > size->h / 2)
        line = size->h - line - 1;

    switch (line)
    {

    case 0:
        paint(COLOR_DARK, size->x, size->w);

        break;

    case 1:
        paint(COLOR_DARK, size->x + 0, 1);
        paint(COLOR_DARK, size->x + size->w - 1, 1);
        paint(framecolor, size->x + 1, size->w - 2);

        break;

    default:
        paint(COLOR_DARK, size->x + 0, 1);
        paint(COLOR_DARK, size->x + size->w - 1, 1);
        paint(framecolor, size->x + 1, 1);
        paint(framecolor, size->x + size->w - 2, 1);

        break;

    }

}

static void rendernull(void *data, unsigned int line)
{

}

static void renderfill(void *data, unsigned int line)
{

    struct widget_fill *fill = data;

    paint(fill->color, fill->widget.size.x, fill->widget.size.w);

}

static void rendermouse(void *data, unsigned int line)
{

    struct widget_mouse *mouse = data;
    unsigned char *mousedata = (mouse->widget.size.h == 16) ? mousedata16 : mousedata24;

    paintbuffer(mousedata + line * mouse->widget.size.w, mouse->widget.size.x, mouse->widget.size.w, 0xFF);

}

static void renderpanel(void *data, unsigned int line)
{

    struct widget_panel *panel = data;
    unsigned int framecolor = panel->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;
    unsigned int backgroundcolor = panel->active ? COLOR_ACTIVEBACK : COLOR_PASSIVEBACK;

    paint(backgroundcolor, panel->widget.size.x, panel->widget.size.w);
    paintframe(framecolor, &panel->widget.size, line);

}

static void rendertext(void *data, unsigned int line)
{

    struct widget_text *text = data;
    char *string = (char *)(text + 1);
 
    painttext(string, text->length, textcolor[text->type], line, &text->widget.size, text->flow, text->cursor);

}

static void renderwindow(void *data, unsigned int line)
{

    struct widget_window *window = data;
    unsigned int framecolor = window->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;

    paintframe(framecolor, &window->widget.size, line);

}

static struct widget *nextwidget(struct widget *widget, struct layer *layer)
{

    widget = (widget) ? (struct widget *)(((unsigned char *)widget) + widget->count) : (struct widget *)layer->data;

    return ((unsigned int)widget < (unsigned int)layer->data + layer->count) ? widget : 0;

}

static void insertwidget(struct widget *widget, struct layer *layer)
{

    struct widget *current = 0;

    while ((current = nextwidget(current, layer)))
    {

        if (current->source == widget->source && current->id == widget->id)
            current->damage = WIDGET_DAMAGE_REMOVE;

    }

    if (widget->damage == WIDGET_DAMAGE_UPDATE)
        layer->count += memory_write(layer->data, layer->total, widget, widget->count, layer->count);

}

static void removewidget(struct widget *widget, struct layer *layer)
{

    unsigned int length = widget->count;

    memory_copy(widget, (unsigned char *)widget + length, layer->count - ((unsigned char *)widget - layer->data) - length);

    layer->count -= length;

}

static unsigned int isoverlap(unsigned int line, struct box *size)
{

    return line >= size->y && line < size->y + size->h;

}

static unsigned int testline(unsigned int line)
{

    unsigned int i;

    for (i = 0; i < LAYERS; i++)
    {

        struct widget *current = 0;

        while ((current = nextwidget(current, &layers[i])))
        {

            if (current->damage != WIDGET_DAMAGE_NONE && isoverlap(line, &current->size))
                return 1;

        }

    }

    return 0;

}

static void renderline(unsigned int line)
{

    unsigned int i;

    for (i = 0; i < LAYERS; i++)
    {

        struct widget *current = 0;

        while ((current = nextwidget(current, &layers[i])))
        {

            if (current->damage != WIDGET_DAMAGE_REMOVE && isoverlap(line, &current->size))
                drawables[current->type](current, line - current->size.y);

        }

    }

}

void render_update(unsigned int descriptor, unsigned int w, unsigned int h)
{

    unsigned int line;

    file_open(descriptor);

    for (line = 0; line < h; line++)
    {

        if (testline(line))
        {

            renderline(line);
            file_seekwriteall(descriptor, drawdata, w, w * line);

        }

    }

    file_close(descriptor);

}

void render_begin(unsigned int descriptor)
{

    unsigned char buffer[FUDGE_BSIZE];
    struct layer insert;

    insert.data = buffer;
    insert.count = 0;
    insert.total = FUDGE_BSIZE;

    while ((insert.count = file_read(descriptor, insert.data, insert.total)))
    {

        struct widget *current = 0;

        while ((current = nextwidget(current, &insert)))
            insertwidget(current, &layers[current->z - 1]);

    }

}

void render_complete(void)
{

    unsigned int i;

    for (i = 0; i < LAYERS; i++)
    {

        struct widget *current = 0;

        while ((current = nextwidget(current, &layers[i])))
        {

            if (current->damage == WIDGET_DAMAGE_REMOVE)
                removewidget(current, &layers[i]);
            else
                current->damage = WIDGET_DAMAGE_NONE;

        }

    }

}

void render_setvideo(unsigned int descriptor, unsigned int w, unsigned int h, unsigned int bpp)
{

    struct ctrl_videosettings settings;

    ctrl_setvideosettings(&settings, w, h, bpp);
    file_open(descriptor);
    file_writeall(descriptor, &settings, sizeof (struct ctrl_videosettings));
    file_close(descriptor);

}

void render_setpaint(unsigned int bpp)
{

    switch (bpp)
    {

    case 8:
        paint = paint8;

        break;

    case 32:
        paint = paint32;

        break;

    }

}

void render_setcolormap(unsigned int descriptor)
{

    file_open(descriptor);
    file_writeall(descriptor, colormap8, 3 * 11);
    file_close(descriptor);

}

void render_setmouse(struct widget_mouse *mouse, unsigned int size)
{

    switch (size)
    {

    case 16:
        mouse->widget.size.w = 12;
        mouse->widget.size.h = 16;

        break;

    case 24:
        mouse->widget.size.w = 18;
        mouse->widget.size.h = 24;

        break;

    }

}

void render_setfont(unsigned int descriptor, unsigned int lineheight)
{

    file_open(descriptor);
    file_read(descriptor, font.data, 0x8000);
    file_close(descriptor);

    font.bitmapdata = pcf_getbitmapdata(font.data);
    font.padding = pcf_getpadding(font.data);
    font.lineheight = lineheight;

}

void render_init()
{

    textcolor[WIDGET_TEXTTYPE_NORMAL] = COLOR_TEXTNORMAL;
    textcolor[WIDGET_TEXTTYPE_HIGHLIGHT] = COLOR_TEXTLIGHT;
    drawables[WIDGET_TYPE_NULL] = rendernull;
    drawables[WIDGET_TYPE_FILL] = renderfill;
    drawables[WIDGET_TYPE_MOUSE] = rendermouse;
    drawables[WIDGET_TYPE_PANEL] = renderpanel;
    drawables[WIDGET_TYPE_TEXT] = rendertext;
    drawables[WIDGET_TYPE_WINDOW] = renderwindow;

}

