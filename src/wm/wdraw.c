#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include <format/pcf.h>
#include "box.h"
#include "renderable.h"

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
static void (*renderers[16])(struct renderable *renderable, unsigned int line);
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

static void rendermouse(struct renderable *renderable, unsigned int line)
{

    unsigned int offset = (line - renderable->size.y) * renderable->size.w;
    unsigned int i;

    for (i = 0; i < renderable->size.w; i++)
    {

        if (mousedata[offset + i] != 0xFF)
            fill(mousedata[offset + i], renderable->size.x + i, 1);

    }

}

static void renderpanel(struct renderable *renderable, unsigned int line)
{

    struct renderable_panelheader *panel = (struct renderable_panelheader *)(renderable + 1);
    unsigned int offset = (line - renderable->size.y);
    unsigned int framecolor = panel->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;
    unsigned int backgroundcolor = panel->active ? COLOR_ACTIVEBACK : COLOR_PASSIVEBACK;

    if (offset > renderable->size.h / 2)
        offset = renderable->size.h - offset - 1;

    switch (offset)
    {

    case 0:
        fill(COLOR_DARK, renderable->size.x, renderable->size.w);

        break;

    case 1:
        fill(COLOR_DARK, renderable->size.x + 0, 1);
        fill(COLOR_DARK, renderable->size.x + renderable->size.w - 1, 1);
        fill(framecolor, renderable->size.x + 1, renderable->size.w - 2);

        break;

    default:
        fill(COLOR_DARK, renderable->size.x + 0, 1);
        fill(COLOR_DARK, renderable->size.x + renderable->size.w - 1, 1);
        fill(framecolor, renderable->size.x + 1, 1);
        fill(framecolor, renderable->size.x + renderable->size.w - 2, 1);
        fill(backgroundcolor, renderable->size.x + 2, renderable->size.w - 4);

        break;

    }

}

static void rendertext(struct renderable *renderable, unsigned int line)
{

    struct renderable_textheader *text = (struct renderable_textheader *)(renderable + 1);
    unsigned int padding = pcf_getpadding(fontdata);
    unsigned int count = renderable->count - sizeof (struct renderable_textheader);
    unsigned char *string = (unsigned char *)(text + 1);
    struct box size;
    unsigned int color;
    unsigned int i;

    size.x = renderable->size.x;
    size.y = renderable->size.y;

    switch (text->type)
    {

    case RENDERABLE_TEXTTYPE_HIGHLIGHT:
        color = COLOR_TEXTLIGHT;

        break;

    case RENDERABLE_TEXTTYPE_NORMAL:
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

static void renderwindow(struct renderable *renderable, unsigned int line)
{

    struct renderable_windowheader *window = (struct renderable_windowheader *)(renderable + 1);
    unsigned int offset = (line - renderable->size.y);
    unsigned int framecolor = window->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;

    if (offset > renderable->size.h / 2)
        offset = renderable->size.h - offset - 1;

    switch (offset)
    {

    case 0:
        fill(COLOR_DARK, renderable->size.x, renderable->size.w);

        break;

    case 1:
        fill(COLOR_DARK, renderable->size.x + 0, 1);
        fill(COLOR_DARK, renderable->size.x + renderable->size.w - 1, 1);
        fill(framecolor, renderable->size.x + 1, renderable->size.w - 2);

        break;

    case 2:
        fill(COLOR_DARK, renderable->size.x + 0, 1);
        fill(COLOR_DARK, renderable->size.x + renderable->size.w - 1, 1);
        fill(framecolor, renderable->size.x + 1, 1);
        fill(framecolor, renderable->size.x + renderable->size.w - 2, 1);
        fill(COLOR_DARK, renderable->size.x + 2, renderable->size.w - 4);

        break;

    default:
        fill(COLOR_DARK, renderable->size.x + 0, 1);
        fill(COLOR_DARK, renderable->size.x + renderable->size.w - 1, 1);
        fill(framecolor, renderable->size.x + 1, 1);
        fill(framecolor, renderable->size.x + renderable->size.w - 2, 1);
        fill(COLOR_DARK, renderable->size.x + 2, 1);
        fill(COLOR_DARK, renderable->size.x + renderable->size.w - 3, 1);

        break;

    }

}

static struct renderable *nextrenderable(unsigned int count, void *data, struct renderable *renderable)
{

    if (renderable == 0)
        renderable = data;
    else
        renderable = (struct renderable *)((unsigned char *)renderable + sizeof (struct renderable) + renderable->count);

    if ((unsigned int)renderable >= (unsigned int)data + count)
        return 0;

    return renderable;

}

static void addrenderable(struct renderable *renderable)
{

    struct renderable *current = 0;

    while ((current = nextrenderable(datacount, data, current)))
    {

        unsigned int length;

        if (current->source != renderable->source || current->id != renderable->id)
            continue;

        length = sizeof (struct renderable) + current->count;

        memory_copy(current, (unsigned char *)current + length, datacount - ((unsigned char *)current - data) - length);

        datacount -= length;

    }

    datacount += memory_write(data, 0x8000, renderable, sizeof (struct renderable) + renderable->count, datacount);

}

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    call_walk(CALL_L0, CALL_PR, 18, "share/ter-u18n.pcf");
    call_open(CALL_L0);
    call_read(CALL_L0, 0x8000, fontdata);
    call_close(CALL_L0);

    renderers[RENDERABLE_TYPE_MOUSE] = rendermouse;
    renderers[RENDERABLE_TYPE_PANEL] = renderpanel;
    renderers[RENDERABLE_TYPE_TEXT] = rendertext;
    renderers[RENDERABLE_TYPE_WINDOW] = renderwindow;

    video_getmode(CALL_L0, &settings);
    video_setcolormap(CALL_L0, 0, 3 * 11, colormap8);
    video_open(CALL_L0);
    call_open(CALL_PI);

    while ((count = call_read(CALL_PI, FUDGE_BSIZE, buffer)))
    {

        struct renderable *renderable;
        unsigned int line;

        renderable = 0;

        while ((renderable = nextrenderable(count, buffer, renderable)))
            addrenderable(renderable);

        for (line = 0; line < settings.h; line++)
        {

            fill(COLOR_BODY, 0, settings.w);

            renderable = 0;

            while ((renderable = nextrenderable(datacount, data, renderable)))
            {

                if (renderable->z != 1)
                    continue;

                if (line >= renderable->size.y && line < renderable->size.y + renderable->size.h)
                    renderers[renderable->type](renderable, line);

            }

            renderable = 0;

            while ((renderable = nextrenderable(datacount, data, renderable)))
            {

                if (renderable->z != 2)
                    continue;

                if (line >= renderable->size.y && line < renderable->size.y + renderable->size.h)
                    renderers[renderable->type](renderable, line);

            }

            video_draw(CALL_L0, settings.w * line, settings.w, drawdata);

        }

    }

    call_close(CALL_PI);
    video_close(CALL_L0);

}

