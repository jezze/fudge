#include <abi.h>
#include <fudge.h>
#include <lib/file.h>
#include <format/pcf.h>
#include "box.h"
#include "element.h"

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

struct context
{

    struct ctrl_videosettings settings;
    unsigned char textcolor[2];
    unsigned char buffer[0x2000];

};

static unsigned int (*tests[EVENTS])(struct element *element, void *data, unsigned int line);
static void (*renderers[EVENTS])(struct context *context, struct element *element, void *data, unsigned int line);
static unsigned char data[0x8000];
static unsigned int datacount;
static unsigned char fontdata[0x8000];
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

static void paint8(struct context *context, unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *buffer = context->buffer;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        buffer[i] = color;

}

static void paint32(struct context *context, unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int *buffer = (unsigned int *)context->buffer;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        buffer[i] = color;

}

static void paint(struct context *context, unsigned int color, unsigned int offset, unsigned int count)
{

    switch (context->settings.bpp)
    {

    case 8:
        paint8(context, color, offset, count);

        break;

    case 32:
        paint32(context, colormap32[color], offset, count);

        break;

    }

}

static unsigned int testfill(struct element *element, void *data, unsigned int line)
{

    struct element_fill *fill = data;

    return line >= fill->size.y && line < fill->size.y + fill->size.h;

}

static void renderfill(struct context *context, struct element *element, void *data, unsigned int line)
{

    struct element_fill *fill = data;

    paint(context, fill->color, fill->size.x, fill->size.w);

}

static unsigned int testmouse(struct element *element, void *data, unsigned int line)
{

    struct element_mouse *mouse = data;

    return line >= mouse->y && line < mouse->y + 24;

}

static void rendermouse(struct context *context, struct element *element, void *data, unsigned int line)
{

    struct element_mouse *mouse = data;
    unsigned int i;

    line = (line - mouse->y);

    for (i = 0; i < 24; i++)
    {

        if (mousedata[line * 24 + i] != 0xFF)
            paint(context, mousedata[line * 24 + i], mouse->x + i, 1);

    }

}

static unsigned int testpanel(struct element *element, void *data, unsigned int line)
{

    struct element_panel *panel = data;

    return line >= panel->size.y && line < panel->size.y + panel->size.h;

}

static void renderpanel(struct context *context, struct element *element, void *data, unsigned int line)
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
        paint(context, COLOR_DARK, panel->size.x, panel->size.w);

        break;

    case 1:
        paint(context, COLOR_DARK, panel->size.x + 0, 1);
        paint(context, COLOR_DARK, panel->size.x + panel->size.w - 1, 1);
        paint(context, framecolor, panel->size.x + 1, panel->size.w - 2);

        break;

    default:
        paint(context, COLOR_DARK, panel->size.x + 0, 1);
        paint(context, COLOR_DARK, panel->size.x + panel->size.w - 1, 1);
        paint(context, framecolor, panel->size.x + 1, 1);
        paint(context, framecolor, panel->size.x + panel->size.w - 2, 1);
        paint(context, backgroundcolor, panel->size.x + 2, panel->size.w - 4);

        break;

    }

}

static unsigned int testtext(struct element *element, void *data, unsigned int line)
{

    struct element_text *text = data;

    return line >= text->size.y && line < text->size.y + text->size.h;

}

static unsigned int findrowcount(struct element_text *text, unsigned int count, unsigned char *string)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (string[i] == '\n')
            return i;

    }

    return count;

}

static unsigned int findrowstart(struct element_text *text, unsigned int row, unsigned int count, unsigned char *string)
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

static void rendertextglyph(struct context *context, unsigned char *data, unsigned int offset, unsigned int count, unsigned int color)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned char c = data[i];
        unsigned int k = 0;
        unsigned char b;

        for (b = 0x80; b; b >>= 1)
        {

            if (c & b)
                paint(context, color, offset + i * 8 + k, 1);

            k++;
                
        }

    }

}

static void rendertext(struct context *context, struct element *element, void *data, unsigned int line)
{

    struct element_text *text = data;
    unsigned int count = element->count - sizeof (struct element_text);
    unsigned char *string = (unsigned char *)(text + 1);
    unsigned char *bitmapdata = pcf_getbitmapdata(fontdata);
    unsigned int fontpadding = pcf_getpadding(fontdata);
    struct box size;
    unsigned int row;
    unsigned int rowline;
    unsigned int rowstart;
    unsigned int rowcount;
    unsigned int i;

    if (!count)
        return;

    line = (line - text->size.y);
    row = line / 24;
    rowline = line % 24;
    rowstart = findrowstart(text, row, count, string);

    if (row && !rowstart)
        return;

    rowcount = findrowcount(text, count - rowstart, string + rowstart);

    if (!rowcount)
        return;

    size.x = text->size.x;
    size.y = text->size.y + row * 24;

    for (i = rowstart; i < rowstart + rowcount; i++)
    {

        unsigned short index = pcf_getindex(fontdata, string[i]);
        unsigned char *data = bitmapdata + pcf_getbitmapoffset(fontdata, index);
        struct pcf_metricsdata metricsdata;

        pcf_readmetricsdata(fontdata, index, &metricsdata);

        size.w = metricsdata.width;
        size.h = metricsdata.ascent + metricsdata.descent;

        if (size.x + size.w >= text->size.x + text->size.w)
            return;

        if (size.y + size.h >= text->size.y + text->size.h)
            return;

        if (rowline < size.h)
            rendertextglyph(context, data + rowline * fontpadding, size.x, fontpadding, context->textcolor[text->type]);

        size.x += size.w;

    }

}

static unsigned int testwindow(struct element *element, void *data, unsigned int line)
{

    struct element_window *window = data;

    return line >= window->size.y && line < window->size.y + window->size.h;

}

static void renderwindow(struct context *context, struct element *element, void *data, unsigned int line)
{

    struct element_window *window = data;
    unsigned int framecolor = window->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;

    line = (line - window->size.y);

    if (line > window->size.h / 2)
        line = window->size.h - line - 1;

    switch (line)
    {

    case 0:
        paint(context, COLOR_DARK, window->size.x, window->size.w);

        break;

    case 1:
        paint(context, COLOR_DARK, window->size.x + 0, 1);
        paint(context, COLOR_DARK, window->size.x + window->size.w - 1, 1);
        paint(context, framecolor, window->size.x + 1, window->size.w - 2);

        break;

    case 2:
        paint(context, COLOR_DARK, window->size.x + 0, 1);
        paint(context, COLOR_DARK, window->size.x + window->size.w - 1, 1);
        paint(context, framecolor, window->size.x + 1, 1);
        paint(context, framecolor, window->size.x + window->size.w - 2, 1);
        paint(context, COLOR_DARK, window->size.x + 2, window->size.w - 4);

        break;

    default:
        paint(context, COLOR_DARK, window->size.x + 0, 1);
        paint(context, COLOR_DARK, window->size.x + window->size.w - 1, 1);
        paint(context, framecolor, window->size.x + 1, 1);
        paint(context, framecolor, window->size.x + window->size.w - 2, 1);
        paint(context, COLOR_DARK, window->size.x + 2, 1);
        paint(context, COLOR_DARK, window->size.x + window->size.w - 3, 1);

        break;

    }

}

static struct element *nextelement(unsigned int count, void *data, struct element *element)
{

    element = (element) ? (struct element *)((unsigned char *)(element + 1) + element->count) : data;

    if ((unsigned int)element >= (unsigned int)data + count)
        return 0;

    return element;

}

static void removeelement(struct element *element)
{

    unsigned int length = sizeof (struct element) + element->count;

    memory_copy(element, (unsigned char *)element + length, datacount - ((unsigned char *)element - data) - length);

    datacount -= length;

}

static void cleanelements(void)
{

    struct element *current = 0;

    while ((current = nextelement(datacount, data, current)))
    {

        current->damaged = 0;

        if (!current->z)
            removeelement(current);

    }

}

static void addelement(struct element *element)
{

    struct element *current = 0;

    element->damaged = 1;

    while ((current = nextelement(datacount, data, current)))
    {

        if (current->source != element->source || current->id != element->id)
            continue;

        current->z = 0;
        current->damaged = 1;

    }

    datacount += memory_write(data, 0x8000, element, sizeof (struct element) + element->count, datacount);

}

static unsigned int testline(unsigned int line)
{

    struct element *element = 0;

    while ((element = nextelement(datacount, data, element)))
    {

        if (element->damaged && tests[element->type](element, element + 1, line))
            return 1;

    }

    return 0;

}

static void render(struct context *context)
{

    unsigned int line;

    call_open(CALL_L1);

    for (line = 0; line < context->settings.h; line++)
    {

        unsigned int z;

        if (!testline(line))
            continue;

        for (z = 1; z < 4; z++)
        {

            struct element *element = 0;

            while ((element = nextelement(datacount, data, element)))
            {

                if (element->z != z)
                    continue;

                if (tests[element->type](element, element + 1, line))
                    renderers[element->type](context, element, element + 1, line);

            }

        }

        file_seekwriteall(CALL_L1, context->buffer, context->settings.w, context->settings.w * line);

    }

    call_close(CALL_L1);

}

static void setup(struct context *context)
{

    call_walk(CALL_L1, CALL_L0, 4, "ctrl");
    call_open(CALL_L1);
    file_seekreadall(CALL_L1, &context->settings, sizeof (struct ctrl_videosettings), 0);
    call_close(CALL_L1);

    if (context->settings.bpp == 8)
    {

        call_walk(CALL_L1, CALL_L0, 8, "colormap");
        call_open(CALL_L1);
        file_writeall(CALL_L1, colormap8, 3 * 11);
        call_close(CALL_L1);

    }

    context->textcolor[ELEMENT_TEXTTYPE_NORMAL] = COLOR_TEXTNORMAL;
    context->textcolor[ELEMENT_TEXTTYPE_HIGHLIGHT] = COLOR_TEXTLIGHT;

    call_walk(CALL_L1, CALL_L0, 4, "data");

}

void main(void)
{

    struct context context;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_walk(CALL_L0, CALL_PR, 18, "share/ter-118n.pcf");
    call_open(CALL_L0);
    file_read(CALL_L0, fontdata, 0x8000);
    call_close(CALL_L0);
    call_walk(CALL_L0, CALL_PR, 15, "system/video:0/");
    setup(&context);

    tests[ELEMENT_TYPE_FILL] = testfill;
    tests[ELEMENT_TYPE_MOUSE] = testmouse;
    tests[ELEMENT_TYPE_PANEL] = testpanel;
    tests[ELEMENT_TYPE_TEXT] = testtext;
    tests[ELEMENT_TYPE_WINDOW] = testwindow;
    renderers[ELEMENT_TYPE_FILL] = renderfill;
    renderers[ELEMENT_TYPE_MOUSE] = rendermouse;
    renderers[ELEMENT_TYPE_PANEL] = renderpanel;
    renderers[ELEMENT_TYPE_TEXT] = rendertext;
    renderers[ELEMENT_TYPE_WINDOW] = renderwindow;

    call_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
    {

        struct element *element = 0;

        while ((element = nextelement(count, buffer, element)))
            addelement(element);

        render(&context);
        cleanelements();

    }

    call_close(CALL_PI);

}

