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

static void fill8(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *buffer = drawdata;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        buffer[i] = color;

}

static void fill32(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int *buffer = (unsigned int *)drawdata;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        buffer[i] = color;

}

static void fill(unsigned int color, unsigned int offset, unsigned int count)
{

    switch (settings.bpp)
    {

    case 8:
        fill8(color, offset, count);

        break;

    case 32:
        fill32(colormap32[color], offset, count);

        break;

    }

}

static unsigned int testmouse(struct element *element, void *data, unsigned int line)
{

    struct element_mouse *mouse = data;

    return line >= mouse->y && line < mouse->y + 24;

}

static void rendermouse(struct element *element, void *data, unsigned int line)
{

    struct element_mouse *mouse = data;
    unsigned int offset = (line - mouse->y) * 24;
    unsigned int i;

    for (i = 0; i < 24; i++)
    {

        if (mousedata[offset + i] != 0xFF)
            fill(mousedata[offset + i], mouse->x + i, 1);

    }

}

static unsigned int testpanel(struct element *element, void *data, unsigned int line)
{

    struct element_panel *panel = data;

    return line > panel->size.y && line < panel->size.y + panel->size.h;

}

static void renderpanel(struct element *element, void *data, unsigned int line)
{

    struct element_panel *panel = data;
    unsigned int offset = (line - panel->size.y);
    unsigned int framecolor = panel->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;
    unsigned int backgroundcolor = panel->active ? COLOR_ACTIVEBACK : COLOR_PASSIVEBACK;

    if (offset > panel->size.h / 2)
        offset = panel->size.h - offset - 1;

    switch (offset)
    {

    case 0:
        fill(COLOR_DARK, panel->size.x, panel->size.w);

        break;

    case 1:
        fill(COLOR_DARK, panel->size.x + 0, 1);
        fill(COLOR_DARK, panel->size.x + panel->size.w - 1, 1);
        fill(framecolor, panel->size.x + 1, panel->size.w - 2);

        break;

    default:
        fill(COLOR_DARK, panel->size.x + 0, 1);
        fill(COLOR_DARK, panel->size.x + panel->size.w - 1, 1);
        fill(framecolor, panel->size.x + 1, 1);
        fill(framecolor, panel->size.x + panel->size.w - 2, 1);
        fill(backgroundcolor, panel->size.x + 2, panel->size.w - 4);

        break;

    }

}

static unsigned int testtext(struct element *element, void *data, unsigned int line)
{

    struct element_text *text = data;

    return line > text->size.y && line < text->size.y + text->size.h;

}

static void rendertext(struct element *element, void *data, unsigned int line)
{

    struct element_text *text = data;
    unsigned int padding = pcf_getpadding(fontdata);
    unsigned int count = element->count - sizeof (struct element_text);
    unsigned char *string = (unsigned char *)(text + 1);
    struct box size;
    unsigned int color;
    unsigned int i;

    size.x = text->size.x;
    size.y = text->size.y;

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
        unsigned int offset = (line - size.y) * padding;
        unsigned char *data;
        struct pcf_metricsdata metricsdata;
        unsigned int x;

        pcf_readmetricsdata(fontdata, index, &metricsdata);

        data = pcf_getbitmapdata(fontdata) + pcf_getbitmapoffset(fontdata, index);
        size.w = metricsdata.width;
        size.h = metricsdata.ascent + metricsdata.descent;

        for (x = 0; x < padding; x++)
        {

            unsigned char c = data[offset + x];
            unsigned int k = 0;
            unsigned char b;

            for (b = 0x80; b; b >>= 1)
            {

                if (c & b)
                    fill(color, size.x + x * 8 + k, 1);

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
    unsigned int offset = (line - window->size.y);
    unsigned int framecolor = window->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;

    if (offset > window->size.h / 2)
        offset = window->size.h - offset - 1;

    switch (offset)
    {

    case 0:
        fill(COLOR_DARK, window->size.x, window->size.w);

        break;

    case 1:
        fill(COLOR_DARK, window->size.x + 0, 1);
        fill(COLOR_DARK, window->size.x + window->size.w - 1, 1);
        fill(framecolor, window->size.x + 1, window->size.w - 2);

        break;

    case 2:
        fill(COLOR_DARK, window->size.x + 0, 1);
        fill(COLOR_DARK, window->size.x + window->size.w - 1, 1);
        fill(framecolor, window->size.x + 1, 1);
        fill(framecolor, window->size.x + window->size.w - 2, 1);
        fill(COLOR_DARK, window->size.x + 2, window->size.w - 4);

        break;

    default:
        fill(COLOR_DARK, window->size.x + 0, 1);
        fill(COLOR_DARK, window->size.x + window->size.w - 1, 1);
        fill(framecolor, window->size.x + 1, 1);
        fill(framecolor, window->size.x + window->size.w - 2, 1);
        fill(COLOR_DARK, window->size.x + 2, 1);
        fill(COLOR_DARK, window->size.x + window->size.w - 3, 1);

        break;

    }

}

static struct element *nextelement(unsigned int count, void *data, struct element *element)
{

    if (element == 0)
        element = data;
    else
        element = (struct element *)((unsigned char *)(element + 1) + element->count);

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

static void removeelements()
{

    struct element *current = 0;

    while ((current = nextelement(datacount, data, current)))
    {

        if (current->z)
            continue;

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

static void render(struct box *damage)
{

    struct element *element;
    unsigned int line;

    if (damage->x + damage->w > settings.w)
        damage->w = settings.w - damage->x;

    if (damage->y + damage->h > settings.h)
        damage->h = settings.h - damage->y;

    for (line = damage->y; line < damage->y + damage->h; line++)
    {

        unsigned int drawline = 0;
        unsigned int z;

        element = 0;

        while ((element = nextelement(datacount, data, element)))
        {

            if (element->damaged && tests[element->type](element, element + 1, line))
            {

                drawline = 1;

                break;

            }

        }

        if (!drawline)
            continue;

        fill(COLOR_BODY, damage->x, damage->w);

        for (z = 1; z < 4; z++)
        {

            element = 0;

            while ((element = nextelement(datacount, data, element)))
            {

                if (element->z != z)
                    continue;

                if (tests[element->type](element, element + 1, line))
                    renderers[element->type](element, element + 1, line);

            }

        }

        video_draw(CALL_L0, settings.w * line + damage->x, damage->w, drawdata + damage->x * settings.bpp / 8);

    }

    while ((element = nextelement(datacount, data, element)))
        element->damaged = 0;

}

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    struct box screen;

    call_walk(CALL_L0, CALL_PR, 18, "share/ter-u18n.pcf");
    call_open(CALL_L0);
    call_read(CALL_L0, 0x8000, fontdata);
    call_close(CALL_L0);

    tests[ELEMENT_TYPE_MOUSE] = testmouse;
    tests[ELEMENT_TYPE_PANEL] = testpanel;
    tests[ELEMENT_TYPE_TEXT] = testtext;
    tests[ELEMENT_TYPE_WINDOW] = testwindow;
    renderers[ELEMENT_TYPE_MOUSE] = rendermouse;
    renderers[ELEMENT_TYPE_PANEL] = renderpanel;
    renderers[ELEMENT_TYPE_TEXT] = rendertext;
    renderers[ELEMENT_TYPE_WINDOW] = renderwindow;

    video_getmode(CALL_L0, &settings);
    video_setcolormap(CALL_L0, 0, 3 * 11, colormap8);
    video_open(CALL_L0);
    call_open(CALL_PI);
    box_setsize(&screen, 0, 0, settings.w, settings.h);
    render(&screen);

    while ((count = call_read(CALL_PI, FUDGE_BSIZE, buffer)))
    {

        struct element *element = 0;

        while ((element = nextelement(count, buffer, element)))
        {

            destroyelements(element->source, element->id);
            addelement(element);

        }

        render(&screen);
        removeelements();

    }

    call_close(CALL_PI);
    video_close(CALL_L0);

}

