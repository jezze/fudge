#include <abi.h>
#include <fudge.h>
#include <video/video.h>
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

static struct ctrl_videosettings settings;
static unsigned int (*tests[EVENTS])(struct element *element, void *data, unsigned int line);
static void (*renderers[EVENTS])(struct element *element, void *data, unsigned int line);
static unsigned char fontdata[0x8000];
static unsigned char drawdata[0x2000];
static unsigned char data[0x8000];
static unsigned int datacount;
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

    unsigned char *buffer = drawdata;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        buffer[i] = color;

}

static void paint32(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int *buffer = (unsigned int *)drawdata;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        buffer[i] = color;

}

static void paint(unsigned int color, unsigned int offset, unsigned int count)
{

    switch (settings.bpp)
    {

    case 8:
        paint8(color, offset, count);

        break;

    case 32:
        paint32(colormap32[color], offset, count);

        break;

    }

}

static unsigned int testfill(struct element *element, void *data, unsigned int line)
{

    struct element_fill *fill = data;

    return line >= fill->size.y && line < fill->size.y + fill->size.h;

}

static void renderfill(struct element *element, void *data, unsigned int line)
{

    struct element_fill *fill = data;

    paint(fill->color, fill->size.x, fill->size.w);

}

static unsigned int testmouse(struct element *element, void *data, unsigned int line)
{

    struct element_mouse *mouse = data;

    return line >= mouse->y && line < mouse->y + 24;

}

static void rendermouse(struct element *element, void *data, unsigned int line)
{

    struct element_mouse *mouse = data;
    unsigned int i;

    line = (line - mouse->y);

    for (i = 0; i < 24; i++)
    {

        if (mousedata[line * 24 + i] != 0xFF)
            paint(mousedata[line * 24 + i], mouse->x + i, 1);

    }

}

static unsigned int testpanel(struct element *element, void *data, unsigned int line)
{

    struct element_panel *panel = data;

    return line >= panel->size.y && line < panel->size.y + panel->size.h;

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

    return line >= text->size.y && line < text->size.y + text->size.h && line < text->size.y + text->rows * 18;

}

static void rendertext(struct element *element, void *data, unsigned int line)
{

    struct element_text *text = data;
    unsigned int count = element->count - sizeof (struct element_text);
    unsigned char *string = (unsigned char *)(text + 1);
    unsigned int padding = pcf_getpadding(fontdata);
    struct box size;
    unsigned int row;
    unsigned int color;
    unsigned int i;

    line = (line - text->size.y);
    row = line / 18;
    size.x = text->size.x;
    size.y = text->size.y + row * 18;

    switch (text->type)
    {

    case ELEMENT_TEXTTYPE_HIGHLIGHT:
        color = COLOR_TEXTLIGHT;

        break;

    case ELEMENT_TEXTTYPE_NORMAL:
    default:
        color = COLOR_TEXTNORMAL;

        break;

    }

    for (i = 0; i < count; i++)
    {

        unsigned short index = pcf_getindex(fontdata, string[i]);
        unsigned char *data = pcf_getbitmapdata(fontdata) + pcf_getbitmapoffset(fontdata, index);
        struct pcf_metricsdata metricsdata;
        unsigned int x;

        pcf_readmetricsdata(fontdata, index, &metricsdata);

        size.w = metricsdata.width;
        size.h = metricsdata.ascent + metricsdata.descent;

        for (x = 0; x < padding; x++)
        {

            unsigned char c = data[(line % 18) * padding + x];
            unsigned int k = 0;
            unsigned char b;

            for (b = 0x80; b; b >>= 1)
            {

                if (c & b)
                    paint(color, size.x + x * 8 + k, 1);

                k++;
                
            }

        }

        size.x += size.w;

    }

}

static unsigned int testwindow(struct element *element, void *data, unsigned int line)
{

    struct element_window *window = data;

    return line > window->size.y && line < window->size.y + window->size.h;

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

static void destroyelements(unsigned int source, unsigned int id)
{

    struct element *current = 0;

    while ((current = nextelement(datacount, data, current)))
    {

        if (current->source != source || current->id != id)
            continue;

        current->z = 0;
        current->damaged = 1;

    }

}

static void addelement(struct element *element)
{

    element->damaged = 1;

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

static void render(unsigned int width, unsigned int height)
{

    unsigned int line;

    for (line = 0; line < height; line++)
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
                    renderers[element->type](element, element + 1, line);

            }

        }

        video_draw(CALL_L0, settings.w * line, width, drawdata);

    }

}

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_walk(CALL_L0, CALL_PR, 18, "share/ter-u18n.pcf");
    call_open(CALL_L0);
    call_read(CALL_L0, 0x8000, fontdata);
    call_close(CALL_L0);

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

    video_getmode(CALL_L0, &settings);
    video_setcolormap(CALL_L0, 0, 3 * 11, colormap8);
    video_open(CALL_L0);
    call_open(CALL_PI);

    while ((count = call_read(CALL_PI, FUDGE_BSIZE, buffer)))
    {

        struct element *element = 0;

        while ((element = nextelement(count, buffer, element)))
        {

            destroyelements(element->source, element->id);
            addelement(element);

        }

        render(settings.w, settings.h);
        cleanelements();

    }

    call_close(CALL_PI);
    video_close(CALL_L0);

}

