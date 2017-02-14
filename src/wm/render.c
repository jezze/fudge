#include <abi.h>
#include <fudge.h>
#include <format/pcf.h>
#include "box.h"
#include "element.h"
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

struct drawable
{

    unsigned int (*test)(struct element *element, void *data, unsigned int line);
    void (*render)(struct element *element, void *data, unsigned int line);

};

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

struct mouse
{

    unsigned char *data;
    unsigned int width;
    unsigned int height;

};

static void (*paint)(unsigned int color, unsigned int offset, unsigned int count);
static struct drawable drawables[5];
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
static struct mouse mousecursor = {0, 0, 0};
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

static unsigned int isoverlap(unsigned int line, unsigned int y, unsigned int h)
{

    return line >= y && line < y + h;

}

static unsigned int testfill(struct element *element, void *data, unsigned int line)
{

    struct element_fill *fill = data;

    return isoverlap(line, fill->size.y, fill->size.h);

}

static void renderfill(struct element *element, void *data, unsigned int line)
{

    struct element_fill *fill = data;

    paint(fill->color, fill->size.x, fill->size.w);

}

static unsigned int testmouse(struct element *element, void *data, unsigned int line)
{

    struct element_mouse *mouse = data;

    return isoverlap(line, mouse->y, mousecursor.height);

}

static void rendermouse(struct element *element, void *data, unsigned int line)
{

    struct element_mouse *mouse = data;
    unsigned char *md = mousecursor.data + (line - mouse->y) * mousecursor.width;
    unsigned int i;

    for (i = 0; i < mousecursor.width; i++)
    {

        if (md[i] != 0xFF)
            paint(md[i], mouse->x + i, 1);

    }

}

static unsigned int testpanel(struct element *element, void *data, unsigned int line)
{

    struct element_panel *panel = data;

    return isoverlap(line, panel->size.y, panel->size.h);

}

static void renderpanel(struct element *element, void *data, unsigned int line)
{

    struct element_panel *panel = data;
    unsigned int framecolor = panel->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;
    unsigned int backgroundcolor = panel->active ? COLOR_ACTIVEBACK : COLOR_PASSIVEBACK;

    line = (line - panel->size.y);

    if (line > panel->size.h / 2)
        line = panel->size.h - line - 1;

    switch (line)
    {

    case 0:
        paint(COLOR_DARK, panel->size.x, panel->size.w);

        break;

    case 1:
        paint(COLOR_DARK, panel->size.x + 0, 1);
        paint(COLOR_DARK, panel->size.x + panel->size.w - 1, 1);
        paint(framecolor, panel->size.x + 1, panel->size.w - 2);

        break;

    default:
        paint(COLOR_DARK, panel->size.x + 0, 1);
        paint(COLOR_DARK, panel->size.x + panel->size.w - 1, 1);
        paint(framecolor, panel->size.x + 1, 1);
        paint(framecolor, panel->size.x + panel->size.w - 2, 1);
        paint(backgroundcolor, panel->size.x + 2, panel->size.w - 4);

        break;

    }

}

static unsigned int testtext(struct element *element, void *data, unsigned int line)
{

    struct element_text *text = data;

    return isoverlap(line, text->size.y, text->size.h);

}

static unsigned int findrowcount(struct element_text *text, unsigned int offset, unsigned int count, char *string)
{

    unsigned int i;

    for (i = offset; i < count; i++)
    {

        if (string[i] == '\n')
            return i + 1;

    }

    return count;

}

static unsigned int findrowstart(struct element_text *text, unsigned int row, unsigned int count, char *string)
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

static void rendercharline(unsigned int x, unsigned int w, unsigned char color, unsigned char *data)
{

    unsigned int p;

    for (p = 0; p < w; p++)
    {

        if (data[(p >> 3)] & (0x80 >> (p % 8)))
            paint(color, x + p, 1);

    }

}

static void rendercharlineinverted(unsigned int x, unsigned int w, unsigned char color, unsigned char *data)
{

    unsigned int p;

    for (p = 0; p < w; p++)
    {

        if (!(data[(p >> 3)] & (0x80 >> (p % 8))))
            paint(color, x + p, 1);

    }

}

static void rendertext(struct element *element, void *data, unsigned int line)
{

    struct element_text *text = data;
    unsigned int stringcount = element->count - sizeof (struct element_text);
    char *string = (char *)(text + 1);
    unsigned char color = textcolor[text->type];
    unsigned int localline = line - text->size.y;
    unsigned int rowindex = localline / font.lineheight;
    unsigned int rowline = localline % font.lineheight;
    unsigned int rowstart;
    unsigned int rowcount;
    struct box size;
    unsigned int i;

    rowstart = findrowstart(text, rowindex, stringcount, string);

    if (!rowstart && rowindex > 0)
        return;

    rowcount = findrowcount(text, rowstart, stringcount, string);

    if (!rowcount)
        return;

    size.x = text->size.x;
    size.y = text->size.y + rowindex * font.lineheight;

    for (i = rowstart; i < rowcount; i++)
    {

        unsigned short index = (string[i] == '\n') ? pcf_getindex(font.data, ' ') : pcf_getindex(font.data, string[i]);
        unsigned char *data = font.bitmapdata + pcf_getbitmapoffset(font.data, index) + rowline * font.padding;
        struct pcf_metricsdata metricsdata;

        pcf_readmetricsdata(font.data, index, &metricsdata);

        size.w = metricsdata.width;
        size.h = metricsdata.ascent + metricsdata.descent;

        if (size.x + size.w > text->size.x + text->size.w)
            return;

        if (size.y + size.h > text->size.y + text->size.h)
            return;

        if (rowline < size.h)
        {

            if (text->flow == ELEMENT_TEXTFLOW_INPUT && i == text->cursor)
                rendercharlineinverted(size.x, size.w, color, data);
            else
                rendercharline(size.x, size.w, color, data);

        }

        size.x += size.w;

    }

}

static unsigned int testwindow(struct element *element, void *data, unsigned int line)
{

    struct element_window *window = data;

    return isoverlap(line, window->size.y, window->size.h);

}

static void renderwindow(struct element *element, void *data, unsigned int line)
{

    struct element_window *window = data;
    unsigned int framecolor = window->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;

    line = (line - window->size.y);

    if (line > window->size.h / 2)
        line = window->size.h - line - 1;

    switch (line)
    {

    case 0:
        paint(COLOR_DARK, window->size.x, window->size.w);

        break;

    case 1:
        paint(COLOR_DARK, window->size.x + 0, 1);
        paint(COLOR_DARK, window->size.x + window->size.w - 1, 1);
        paint(framecolor, window->size.x + 1, window->size.w - 2);

        break;

    default:
        paint(COLOR_DARK, window->size.x + 0, 1);
        paint(COLOR_DARK, window->size.x + window->size.w - 1, 1);
        paint(framecolor, window->size.x + 1, 1);
        paint(framecolor, window->size.x + window->size.w - 2, 1);

        break;

    }

}

static struct element *nextelement(struct element *element, unsigned char *data, unsigned int count)
{

    element = (element) ? (struct element *)((unsigned char *)(element + 1) + element->count) : (struct element *)data;

    return ((unsigned int)element < (unsigned int)data + count) ? element : 0;

}

static void insertelement(struct element *element, struct layer *layer)
{

    struct element *current = 0;

    while ((current = nextelement(current, layer->data, layer->count)))
    {

        if (current->source == element->source && current->id == element->id)
            current->damage = ELEMENT_DAMAGE_REMOVE;

    }

    if (element->damage == ELEMENT_DAMAGE_UPDATE)
        layer->count += memory_write(layer->data, layer->total, element, sizeof (struct element) + element->count, layer->count);

}

static void removeelement(struct element *element, struct layer *layer)
{

    unsigned int length = sizeof (struct element) + element->count;

    memory_copy(element, (unsigned char *)element + length, layer->count - ((unsigned char *)element - layer->data) - length);

    layer->count -= length;

}

static unsigned int testline(unsigned int line)
{

    unsigned int i;

    for (i = 0; i < LAYERS; i++)
    {

        struct element *current = 0;

        while ((current = nextelement(current, layers[i].data, layers[i].count)))
        {

            if (current->damage && drawables[current->type].test(current, current + 1, line))
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

        struct element *current = 0;

        while ((current = nextelement(current, layers[i].data, layers[i].count)))
        {

            if (current->damage != ELEMENT_DAMAGE_REMOVE && drawables[current->type].test(current, current + 1, line))
                drawables[current->type].render(current, current + 1, line);

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

void render_parse(unsigned int descriptor)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    while ((count = file_read(descriptor, buffer, FUDGE_BSIZE)))
    {

        struct element *current = 0;

        while ((current = nextelement(current, buffer, count)))
            insertelement(current, &layers[current->z - 1]);

    }

}

void render_complete(void)
{

    unsigned int i;

    for (i = 0; i < LAYERS; i++)
    {

        struct element *current = 0;

        while ((current = nextelement(current, layers[i].data, layers[i].count)))
        {

            if (current->damage == ELEMENT_DAMAGE_REMOVE)
                removeelement(current, &layers[i]);
            else
                current->damage = ELEMENT_DAMAGE_NONE;

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

void render_setmouse(unsigned int size)
{

    switch (size)
    {

    case 16:
        mousecursor.data = mousedata16;
        mousecursor.width = 12;
        mousecursor.height = 16;

        break;

    case 24:
        mousecursor.data = mousedata24;
        mousecursor.width = 18;
        mousecursor.height = 24;

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

    textcolor[ELEMENT_TEXTTYPE_NORMAL] = COLOR_TEXTNORMAL;
    textcolor[ELEMENT_TEXTTYPE_HIGHLIGHT] = COLOR_TEXTLIGHT;
    drawables[ELEMENT_TYPE_FILL].test = testfill;
    drawables[ELEMENT_TYPE_FILL].render = renderfill;
    drawables[ELEMENT_TYPE_MOUSE].test = testmouse;
    drawables[ELEMENT_TYPE_MOUSE].render = rendermouse;
    drawables[ELEMENT_TYPE_PANEL].test = testpanel;
    drawables[ELEMENT_TYPE_PANEL].render = renderpanel;
    drawables[ELEMENT_TYPE_TEXT].test = testtext;
    drawables[ELEMENT_TYPE_TEXT].render = rendertext;
    drawables[ELEMENT_TYPE_WINDOW].test = testwindow;
    drawables[ELEMENT_TYPE_WINDOW].render = renderwindow;

}

