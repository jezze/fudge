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
#define MOUSE_WIDTH                     24
#define MOUSE_HEIGHT                    24

struct drawable
{

    unsigned int (*test)(struct element *element, void *data, unsigned int line);
    void (*render)(struct element *element, void *data, unsigned int line);

};

static struct drawable drawables[5];
static unsigned char textcolor[2];
static unsigned char drawdata[0x2000];
static unsigned char layerdata1[0x8000];
static unsigned int layercount1;
static unsigned char layerdata2[0x1000];
static unsigned int layercount2;
static unsigned char fontdata[0x8000];
static unsigned char *fontbitmapdata;
static unsigned int fontpadding;
static unsigned int fontlineheight;
static void (*paint)(unsigned int color, unsigned int offset, unsigned int count);
static unsigned char mousedata[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x07, 0x08, 0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x07, 0x08, 0x00, 0x00, 0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x08, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x08, 0x07, 0x07, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x08, 0x07, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x08, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
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

    return isoverlap(line, mouse->y, MOUSE_HEIGHT);

}

static void rendermouse(struct element *element, void *data, unsigned int line)
{

    struct element_mouse *mouse = data;
    unsigned char *md = mousedata + (line - mouse->y) * MOUSE_WIDTH;
    unsigned int i;

    for (i = 0; i < MOUSE_WIDTH; i++)
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

static void rendertextline(struct element_text *text, char *string, unsigned char color, unsigned int rowtop, unsigned int rowline, unsigned int rowstart, unsigned int rowcount)
{

    struct box size;
    unsigned int i;

    size.x = text->size.x;
    size.y = text->size.y + rowtop;

    for (i = rowstart; i < rowcount; i++)
    {

        unsigned short index = (string[i] == '\n') ? pcf_getindex(fontdata, ' ') : pcf_getindex(fontdata, string[i]);
        unsigned char *data = fontbitmapdata + pcf_getbitmapoffset(fontdata, index) + rowline * fontpadding;
        struct pcf_metricsdata metricsdata;

        pcf_readmetricsdata(fontdata, index, &metricsdata);

        size.w = metricsdata.width;
        size.h = metricsdata.ascent + metricsdata.descent;

        if (size.x + size.w >= text->size.x + text->size.w)
            return;

        if (size.y + size.h >= text->size.y + text->size.h)
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

static void rendertext(struct element *element, void *data, unsigned int line)
{

    struct element_text *text = data;
    unsigned int stringcount = element->count - sizeof (struct element_text);
    char *string = (char *)(text + 1);
    unsigned char color = textcolor[text->type];
    unsigned int row;
    unsigned int rowline;
    unsigned int rowtop;
    unsigned int rowtotal;
    unsigned int rowstart;
    unsigned int rowcount;

    if (!stringcount)
        return;

    line = (line - text->size.y);
    row = line / fontlineheight;
    rowline = line % fontlineheight;
    rowtop = row * fontlineheight;
    rowtotal = ascii_count(string, stringcount, '\n') + 1;

    if (row >= rowtotal)
        return;

    rowstart = findrowstart(text, row, stringcount, string);
    rowcount = findrowcount(text, rowstart, stringcount, string);

    if (!rowcount)
        return;

    rendertextline(text, string, color, rowtop, rowline, rowstart, rowcount);

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

    case 2:
        paint(COLOR_DARK, window->size.x + 0, 1);
        paint(COLOR_DARK, window->size.x + window->size.w - 1, 1);
        paint(framecolor, window->size.x + 1, 1);
        paint(framecolor, window->size.x + window->size.w - 2, 1);
        paint(COLOR_DARK, window->size.x + 2, window->size.w - 4);

        break;

    default:
        paint(COLOR_DARK, window->size.x + 0, 1);
        paint(COLOR_DARK, window->size.x + window->size.w - 1, 1);
        paint(framecolor, window->size.x + 1, 1);
        paint(framecolor, window->size.x + window->size.w - 2, 1);
        paint(COLOR_DARK, window->size.x + 2, 1);
        paint(COLOR_DARK, window->size.x + window->size.w - 3, 1);

        break;

    }

}

static struct element *nextelement(struct element *element, unsigned char *data, unsigned int count)
{

    element = (element) ? (struct element *)((unsigned char *)(element + 1) + element->count) : (struct element *)data;

    return ((unsigned int)element < (unsigned int)data + count) ? element : 0;

}

static unsigned int insertelement(struct element *element, unsigned char *data, unsigned int count)
{

    struct element *current = 0;

    while ((current = nextelement(current, data, count)))
    {

        if (current->source == element->source && current->id == element->id)
            current->damage = ELEMENT_DAMAGE_REMOVE;

    }

    return (element->damage == ELEMENT_DAMAGE_UPDATE) ? memory_write(data, 0x8000, element, sizeof (struct element) + element->count, count) : 0;

}

static unsigned int removeelement(struct element *element, unsigned char *data, unsigned int count)
{

    unsigned int length = sizeof (struct element) + element->count;

    memory_copy(element, (unsigned char *)element + length, count - ((unsigned char *)element - data) - length);

    return length;

}

static unsigned int testlayerline(unsigned int line, unsigned char *data, unsigned int count)
{

    struct element *current = 0;

    while ((current = nextelement(current, data, count)))
    {

        if (current->damage && drawables[current->type].test(current, current + 1, line))
            return 1;

    }

    return 0;

}

static unsigned int testline(unsigned int line)
{

    if (testlayerline(line, layerdata1, layercount1))
        return 1;

    if (testlayerline(line, layerdata2, layercount2))
        return 1;

    return 0;

}

static void renderlayerline(unsigned int line, unsigned char *data, unsigned int count)
{

    struct element *current = 0;

    while ((current = nextelement(current, data, count)))
    {

        if (current->damage != ELEMENT_DAMAGE_REMOVE && drawables[current->type].test(current, current + 1, line))
            drawables[current->type].render(current, current + 1, line);

    }

}

void render_update(unsigned int descriptor, struct ctrl_videosettings *settings)
{

    unsigned int line;

    file_open(descriptor);

    for (line = 0; line < settings->h; line++)
    {

        if (testline(line))
        {

            renderlayerline(line, layerdata1, layercount1);
            renderlayerline(line, layerdata2, layercount2);
            file_seekwriteall(descriptor, drawdata, settings->w, settings->w * line);

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

        struct element *element = 0;

        while ((element = nextelement(element, buffer, count)))
        {

            switch (element->z)
            {

            case 1:
                layercount1 += insertelement(element, layerdata1, layercount1);

                break;

            case 2:
                layercount2 += insertelement(element, layerdata2, layercount2);

                break;

            }

        }

    }

}

void render_complete(void)
{

    struct element *current;

    current = 0;

    while ((current = nextelement(current, layerdata1, layercount1)))
    {

        if (current->damage == ELEMENT_DAMAGE_REMOVE)
            layercount1 -= removeelement(current, layerdata1, layercount1);
        else
            current->damage = ELEMENT_DAMAGE_NONE;

    }

    current = 0;

    while ((current = nextelement(current, layerdata2, layercount2)))
    {

        if (current->damage == ELEMENT_DAMAGE_REMOVE)
            layercount2 -= removeelement(current, layerdata2, layercount2);
        else
            current->damage = ELEMENT_DAMAGE_NONE;

    }

}

void render_initvideo(unsigned int descriptor, struct ctrl_videosettings *settings)
{

    file_open(descriptor);
    file_seekwriteall(descriptor, settings, sizeof (struct ctrl_videosettings), 0);
    file_seekreadall(descriptor, settings, sizeof (struct ctrl_videosettings), 0);
    file_close(descriptor);

    switch (settings->bpp)
    {

    case 8:
        paint = paint8;

        break;

    case 32:
        paint = paint32;

        break;

    }

}

void render_initcolormap(unsigned int descriptor)
{

    file_open(descriptor);
    file_writeall(descriptor, colormap8, 3 * 11);
    file_close(descriptor);

}

void render_initfont(unsigned int descriptor, unsigned int lineheight)
{

    file_open(descriptor);
    file_read(descriptor, fontdata, 0x8000);
    file_close(descriptor);

    fontbitmapdata = pcf_getbitmapdata(fontdata);
    fontpadding = pcf_getpadding(fontdata);
    fontlineheight = lineheight;

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

