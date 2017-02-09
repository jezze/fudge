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

static struct ev_handlers handlers;
static struct ctrl_videosettings settings;
static struct drawable drawables[5];
static unsigned char textcolor[2];
static unsigned char drawdata[0x2000];
static unsigned char layerdata1[0x8000];
static unsigned int layercount1;
static unsigned char fontdata[0x8000];
static unsigned char *fontbitmapdata;
static unsigned int fontpadding;
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
    0xFF, 0xFF, 0xFF,
    0x03, 0x02, 0x02,
    0x05, 0x04, 0x04,
    0x07, 0x06, 0x06,
    0x08, 0x10, 0x18,
    0x0C, 0x14, 0x1C,
    0x28, 0x10, 0x18,
    0x38, 0x20, 0x28,
    0x1C, 0x18, 0x18,
    0xFF, 0xFF, 0xFF
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
    row = line / text->lineheight;
    rowline = line % text->lineheight;
    rowtop = row * text->lineheight;
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

static struct element *nextelement(unsigned char *data, unsigned int count, struct element *element)
{

    element = (element) ? (struct element *)((unsigned char *)(element + 1) + element->count) : (struct element *)data;

    if ((unsigned int)element >= (unsigned int)data + count)
        return 0;

    return element;

}

static void insertelement(struct element *element)
{

    struct element *current = 0;

    while ((current = nextelement(layerdata1, layercount1, current)))
    {

        if (current->source == element->source && current->id == element->id)
        {

            current->z = 0;
            current->damaged = 1;

        }

    }

    if (element->z)
    {

        element->damaged = 1;

        layercount1 += memory_write(layerdata1, 0x8000, element, sizeof (struct element) + element->count, layercount1);

    }

}

static void cleanelements(void)
{

    struct element *current = 0;

    while ((current = nextelement(layerdata1, layercount1, current)))
    {

        current->damaged = 0;

        if (!current->z)
        {

            unsigned int length = sizeof (struct element) + current->count;

            memory_copy(current, (unsigned char *)current + length, layercount1 - ((unsigned char *)current - layerdata1) - length);

            layercount1 -= length;

        }

    }

}

static unsigned int testline(unsigned int line)
{

    struct element *element = 0;

    while ((element = nextelement(layerdata1, layercount1, element)))
    {

        if (element->damaged && drawables[element->type].test(element, element + 1, line))
            return 1;

    }

    return 0;

}

static void renderline(unsigned int line)
{

    unsigned int z;

    for (z = 1; z < 4; z++)
    {

        struct element *element = 0;

        while ((element = nextelement(layerdata1, layercount1, element)))
        {

            if (element->z != z)
                continue;

            if (drawables[element->type].test(element, element + 1, line))
                drawables[element->type].render(element, element + 1, line);

        }

    }

}

static void renderscreen(unsigned int descriptor)
{

    unsigned int line;

    for (line = 0; line < settings.h; line++)
    {

        if (!testline(line))
            continue;

        renderline(line);
        file_seekwriteall(descriptor, drawdata, settings.w, settings.w * line);

    }

}

static void renderflush(unsigned int datadescriptor, unsigned int renderdescriptor)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    while ((count = file_read(datadescriptor, buffer, FUDGE_BSIZE)))
    {

        struct element *element = 0;

        while ((element = nextelement(buffer, count, element)))
            insertelement(element);

        renderscreen(renderdescriptor);
        cleanelements();

    }

}

static void renderinitgraphics(unsigned int ctrldescriptor, unsigned int colormapdescriptor, unsigned int fontdescriptor)
{

    file_open(ctrldescriptor);
    ctrl_setvideosettings(&settings, 1920, 1080, 32);
    file_seekwriteall(ctrldescriptor, &settings, sizeof (struct ctrl_videosettings), 0);
    file_seekreadall(ctrldescriptor, &settings, sizeof (struct ctrl_videosettings), 0);
    file_close(ctrldescriptor);

    switch (settings.bpp)
    {

    case 8:
        paint = paint8;

        break;

    case 32:
        paint = paint32;

        break;

    }

    file_open(colormapdescriptor);
    file_writeall(colormapdescriptor, colormap8, 3 * 11);
    file_close(colormapdescriptor);
    file_open(fontdescriptor);
    file_read(fontdescriptor, fontdata, 0x8000);
    file_close(fontdescriptor);

    fontbitmapdata = pcf_getbitmapdata(fontdata);
    fontpadding = pcf_getpadding(fontdata);

}

static void renderinit()
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

static void onwmmap(struct event_header *header)
{

    renderinitgraphics(CALL_L3, CALL_L4, CALL_L5);

    handlers.wmmap = 0;

}

static void onwmflush(struct event_header *header)
{

    renderflush(CALL_L0, CALL_L2);

}

void main(void)
{

    handlers.wmmap = onwmmap;
    handlers.wmflush = onwmflush;

    renderinit();

    if (!file_walk(CALL_L0, "/system/wm/data"))
        return;

    if (!file_walk(CALL_L1, "/system/wm/event"))
        return;

    if (!file_walkfrom(CALL_L2, CALL_PO, "data"))
        return;

    if (!file_walkfrom(CALL_L3, CALL_PO, "ctrl"))
        return;

    if (!file_walkfrom(CALL_L4, CALL_PO, "colormap"))
        return;

    if (!file_walk(CALL_L5, "/share/ter-118n.pcf"))
        return;

    file_open(CALL_L0);
    file_open(CALL_L1);
    file_open(CALL_L2);

    while (ev_read(&handlers, CALL_L1));

    file_close(CALL_L2);
    file_close(CALL_L1);
    file_close(CALL_L0);

}

