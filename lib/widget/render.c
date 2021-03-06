#include <fudge.h>
#include "box.h"
#include "widget.h"

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
    unsigned int bitmapalign;
    unsigned int lineheight;
    unsigned int padding;

};

static unsigned int currentw;
static unsigned int currenth;
static unsigned int currentbpp;
static struct font font;
static void (*drawables[7])(void *canvas, void *data, unsigned int line);
static void (*paint)(void *canvas, unsigned int color, unsigned int offset, unsigned int count);
static unsigned char textcolor[2];
static unsigned char layerdata1[0x8000];
static unsigned char layerdata2[0x1000];
static struct layer layers[LAYERS] = {
    {layerdata1, 0, 0x8000},
    {layerdata2, 0, 0x1000}
};
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
static unsigned char colormap24[] = {
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0x18, 0x10, 0x14,
    0x20, 0x18, 0x1C,
    0x30, 0x28, 0x2C,
    0x10, 0x50, 0x70,
    0x30, 0x70, 0x90,
    0xB0, 0x50, 0x70,
    0xF8, 0x98, 0xB8,
    0x80, 0x78, 0x7C,
    0xFF, 0xFF, 0xFF
};
/*
static unsigned char colormap32[] = {
    0xFF, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x18, 0x10, 0x14,
    0xFF, 0x20, 0x18, 0x1C,
    0xFF, 0x30, 0x28, 0x2C,
    0xFF, 0x10, 0x50, 0x70,
    0xFF, 0x30, 0x70, 0x90,
    0xFF, 0xB0, 0x50, 0x70,
    0xFF, 0xF8, 0x98, 0xB8,
    0xFF, 0x80, 0x78, 0x7C,
    0xFF, 0xFF, 0xFF, 0xFF
};
*/
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

static unsigned int findrowtotal(unsigned char *string, unsigned int count)
{

    unsigned int i;
    unsigned int total = 0;

    for (i = 0; i < count; i++)
    {

        if (string[i] == '\n')
            total++;

    }

    return total;

}

static unsigned int findrowstart(unsigned char *string, unsigned int count, unsigned int row)
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

static unsigned int findrowcount(unsigned char *string, unsigned int count, unsigned int offset)
{

    unsigned int i;

    for (i = offset; i < count; i++)
    {

        if (string[i] == '\n')
            return i + 1;

    }

    return count;

}

static unsigned short getfontindex(unsigned short c)
{

    switch (c)
    {

    case '\n':
        return pcf_getindex(font.data, ' ');

    }

    return pcf_getindex(font.data, c);

}

static void paint8(void *canvas, unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *buffer = canvas;

    buffer += offset;

    while (count--)
        *buffer++ = color;

}

static void paint24(void *canvas, unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *buffer = canvas;

    buffer += offset * 3;

    while (count--)
    {

        unsigned int x = color * 3;

        *buffer++ = colormap24[x + 2];
        *buffer++ = colormap24[x + 1];
        *buffer++ = colormap24[x + 0];

    }

}

/*
static void paint32(void *canvas, unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *buffer = canvas;

    buffer += offset * 4;

    while (count--)
    {

        unsigned int x = color * 4;

        *buffer++ = colormap32[x + 3];
        *buffer++ = colormap32[x + 2];
        *buffer++ = colormap32[x + 1];
        *buffer++ = colormap32[x + 0];

    }

}
*/

static void paint32(void *canvas, unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int *buffer = canvas;

    buffer += offset;

    while (count--)
        *buffer++ = colormap32[color];

}

static void paintcharline(void *canvas, unsigned int x, unsigned int w, unsigned char color, unsigned char *data)
{

    unsigned int i;

    for (i = 0; i < w; i++)
    {

        if (data[(i >> 3)] & (0x80 >> (i % 8)))
            paint(canvas, color, x + i, 1);

    }

}

static void paintcharlineinverted(void *canvas, unsigned int x, unsigned int w, unsigned char color, unsigned char *data)
{

    unsigned int i;

    for (i = 0; i < w; i++)
    {

        if (!(data[(i >> 3)] & (0x80 >> (i % 8))))
            paint(canvas, color, x + i, 1);

    }

}

static void painttext(void *canvas, unsigned char *string, unsigned int length, unsigned int x, unsigned int w, unsigned char color, unsigned int row, unsigned int cursor)
{

    unsigned int i;

    for (i = 0; i < length; i++)
    {

        unsigned short index = getfontindex(string[i]);
        unsigned int offset = pcf_getbitmapoffset(font.data, index);
        struct pcf_metricsdata metricsdata;

        pcf_readmetricsdata(font.data, index, &metricsdata);

        if (x + metricsdata.width > w)
            return;

        if (row < metricsdata.ascent + metricsdata.descent)
        {

            if (i == cursor)
                paintcharlineinverted(canvas, x, metricsdata.width, color, font.bitmapdata + offset + row * font.bitmapalign);
            else
                paintcharline(canvas, x, metricsdata.width, color, font.bitmapdata + offset + row * font.bitmapalign);

        }

        x += metricsdata.width;

    }

}

static void paintframe(void *canvas, unsigned int framecolor, struct box *size, unsigned int line)
{

    if (line > size->h / 2)
        line = size->h - line - 1;

    switch (line)
    {

    case 0:
        paint(canvas, COLOR_DARK, size->x, size->w);

        break;

    case 1:
        paint(canvas, COLOR_DARK, size->x + 0, 1);
        paint(canvas, COLOR_DARK, size->x + size->w - 1, 1);
        paint(canvas, framecolor, size->x + 1, size->w - 2);

        break;

    default:
        paint(canvas, COLOR_DARK, size->x + 0, 1);
        paint(canvas, COLOR_DARK, size->x + size->w - 1, 1);
        paint(canvas, framecolor, size->x + 1, 1);
        paint(canvas, framecolor, size->x + size->w - 2, 1);

        break;

    }

}

static void rendernull(void *canvas, void *data, unsigned int line)
{

}

static void renderfill(void *canvas, void *data, unsigned int line)
{

    struct widget_fill *fill = data;

    paint(canvas, fill->color, fill->size.x, fill->size.w);

}

static void rendermouse(void *canvas, void *data, unsigned int line)
{

    struct widget_mouse *mouse = data;
    unsigned char *mousedata = (mouse->size.h == 16) ? mousedata16 : mousedata24;
    unsigned char *buffer = mousedata + line * mouse->size.w;
    unsigned int w;
    unsigned int i;

    if (mouse->size.x + mouse->size.w < currentw)
        w = mouse->size.w;
    else if (mouse->size.x + mouse->size.w < currentw + mouse->size.w)
        w = currentw - mouse->size.x;
    else
        w = 0;

    for (i = 0; i < w; i++)
    {

        if (buffer[i] != 0xFF)
            paint(canvas, buffer[i], mouse->size.x + i, 1);

    }

}

static void renderpanel(void *canvas, void *data, unsigned int line)
{

    struct widget_panel *panel = data;
    unsigned char *string = (unsigned char *)(panel + 1);
    unsigned int stringcolor = panel->active ? textcolor[WIDGET_TEXTTYPE_HIGHLIGHT] : textcolor[WIDGET_TEXTTYPE_NORMAL];
    unsigned int framecolor = panel->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;
    unsigned int backgroundcolor = panel->active ? COLOR_ACTIVEBACK : COLOR_PASSIVEBACK;
    struct box textbox;

    box_setsize(&textbox, panel->size.x, panel->size.y, panel->size.w, panel->size.h);
    box_resize(&textbox, font.padding);
    paint(canvas, backgroundcolor, panel->size.x, panel->size.w);
    paintframe(canvas, framecolor, &panel->size, line);

    if (line >= font.padding && ((line - font.padding) / font.lineheight == 0))
        painttext(canvas, string, panel->length, textbox.x, textbox.x + textbox.w, stringcolor, (line - font.padding) % font.lineheight, panel->length);

}

static void rendertext(void *canvas, void *data, unsigned int line)
{

    struct widget_text *text = data;
    unsigned char *string = (unsigned char *)(text + 1);
    unsigned int rowindex = text->offset + line / font.lineheight;
    unsigned int rowtotal = findrowtotal(string, text->length);

    if (rowindex < rowtotal)
    {

        unsigned int rowstart = findrowstart(string, text->length, rowindex);
        unsigned int rowcount = findrowcount(string, text->length, rowstart);

        painttext(canvas, string + rowstart, rowcount - rowstart, text->size.x, text->size.x + text->size.w, textcolor[text->type], line % font.lineheight, rowcount - rowstart);

    }

}

static void rendertextbox(void *canvas, void *data, unsigned int line)
{

    struct widget_textbox *textbox = data;
    unsigned char *string = (unsigned char *)(textbox + 1);
    unsigned int rowindex = textbox->offset + line / font.lineheight;
    unsigned int rowtotal = findrowtotal(string, textbox->length);

    if (rowindex < rowtotal)
    {

        unsigned int rowstart = findrowstart(string, textbox->length, rowindex);
        unsigned int rowcount = findrowcount(string, textbox->length, rowstart);

        painttext(canvas, string + rowstart, rowcount - rowstart, textbox->size.x, textbox->size.x + textbox->size.w, textcolor[WIDGET_TEXTTYPE_NORMAL], line % font.lineheight, textbox->cursor - rowstart);

    }

}

static void renderwindow(void *canvas, void *data, unsigned int line)
{

    struct widget_window *window = data;
    unsigned int framecolor = window->active ? COLOR_ACTIVEFRAME : COLOR_PASSIVEFRAME;

    paintframe(canvas, framecolor, &window->size, line);

}

static struct widget *nextwidget(struct layer *layer, void *current)
{

    unsigned char *position = current;
    struct widget *widget = current;

    current = (current) ? position + widget->count : layer->data;
    position = current;

    return (position < layer->data + layer->count) ? current : 0;

}

static void markwidgets(struct layer *layer, unsigned int source, unsigned int id, unsigned int damage)
{

    struct widget *current = 0;

    while ((current = nextwidget(layer, current)))
    {

        if (current->source == source && current->id == id)
            current->damage = damage;

    }

}

static void markwidgetsall(struct layer *layer, unsigned int source, unsigned int damage)
{

    struct widget *current = 0;

    while ((current = nextwidget(layer, current)))
    {

        if (current->source == source)
            current->damage = damage;

    }

}

static void addwidget(struct layer *layer, struct widget *widget)
{

    layer->count += buffer_write(layer->data, layer->total, widget, widget->count, layer->count);

}

static void removedata(struct layer *layer, void *data, unsigned int count)
{

    unsigned char *position = data;

    buffer_copy(position, position + count, layer->count - (position - layer->data) - count);

    layer->count -= count;

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

        while ((current = nextwidget(&layers[i], current)))
        {

            if (current->damage != WIDGET_DAMAGE_NONE && isoverlap(line, &current->size))
                return 1;

        }

    }

    return 0;

}

static void renderline(void *canvas, unsigned int line)
{

    unsigned int i;

    for (i = 0; i < LAYERS; i++)
    {

        struct widget *current = 0;

        while ((current = nextwidget(&layers[i], current)))
        {

            if (current->damage != WIDGET_DAMAGE_REMOVE && isoverlap(line, &current->size))
                drawables[current->type](canvas, current + 1, line - current->size.y);

        }

    }

}

void render_flush(unsigned char *canvasdata, unsigned int size, void (*draw)(void *data, unsigned int count, unsigned int offset))
{

    unsigned int linesize = currentw * currentbpp;
    unsigned int chunksize = size / linesize;
    unsigned int chunk = 0;
    unsigned int chunkstart = 0;
    unsigned int line;
    char lines[BUFFER_SIZE];

    for (line = 0; line < currenth; line++)
        lines[line] = testline(line);

    for (line = 0; line < currenth; line++)
    {

        if (lines[line])
        {

            renderline(canvasdata + chunk * linesize, line);

            if (!chunk)
                chunkstart = line;

            chunk++;

            if (chunk < chunksize && line < currenth - 1)
                continue;

        }

        if (chunk)
        {

            draw(canvasdata, linesize * chunk, chunkstart * linesize);

            chunk = 0;

        }

    }

}

void render_write(unsigned int source, void *buffer, unsigned int count)
{

    struct widget *current = 0;
    struct layer layer;

    layer.data = buffer;
    layer.count = count;
    layer.total = count;

    while ((current = nextwidget(&layer, current)))
    {

        struct layer *target = &layers[current->z];

        current->source = source;

        switch (current->damage)
        {

        case WIDGET_DAMAGE_REMOVE:
            markwidgets(target, current->source, current->id, WIDGET_DAMAGE_REMOVE);

            break;

        case WIDGET_DAMAGE_REPLACE:
            markwidgets(target, current->source, current->id, WIDGET_DAMAGE_REMOVE);
            addwidget(target, current);

            break;

        }

    }

}

void render_resize(unsigned int source, int x, int y, int w, int h, unsigned int padding, unsigned int lineheight, unsigned int steplength)
{

    struct widget *current = 0;

    while ((current = nextwidget(&layers[WIDGET_Z_BOTTOM], current)))
    {

        if (current->source != source)
            continue;

        if (current->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = (struct widget_text *)(current + 1);

            current->size.x = x;
            current->size.y = y;
            current->size.w = w;
            current->size.h = h;
            text->size.x = x;
            text->size.y = y;
            text->size.w = w;

            /* make dynamic */
            text->size.h = h;

        }

        if (current->type == WIDGET_TYPE_TEXTBOX)
        {

            struct widget_textbox *textbox = (struct widget_textbox *)(current + 1);

            current->size.x = x;
            current->size.y = y;
            current->size.w = w;
            current->size.h = h;
            textbox->size.x = x;
            textbox->size.y = y;
            textbox->size.w = w;

            /* make dynamic */
            textbox->size.h = h;

        }

    }

}

void render_clean(unsigned int source)
{

    unsigned int i;

    for (i = 0; i < LAYERS; i++)
        markwidgetsall(&layers[i], source, WIDGET_DAMAGE_REMOVE);

}

void render_complete(void)
{

    unsigned int i;

    for (i = 0; i < LAYERS; i++)
    {

        struct widget *current = 0;

        while ((current = nextwidget(&layers[i], current)))
        {

            if (current->damage == WIDGET_DAMAGE_REMOVE)
                removedata(&layers[i], current, current->count);
            else
                current->damage = WIDGET_DAMAGE_NONE;

        }

    }

}

void render_setdraw(unsigned int w, unsigned int h, unsigned int bpp)
{

    currentw = w;
    currenth = h;
    currentbpp = bpp;

    switch (bpp)
    {

    case 1:
        paint = paint8;

        break;

    case 3:
        paint = paint24;

        break;

    case 4:
        paint = paint32;

        break;

    }

}

void render_setfont(void *data, unsigned int lineheight, unsigned int padding)
{

    font.data = data;
    font.bitmapdata = pcf_getbitmapdata(font.data);
    font.bitmapalign = pcf_getbitmapalign(font.data);
    font.lineheight = lineheight;
    font.padding = padding;

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
    drawables[WIDGET_TYPE_TEXTBOX] = rendertextbox;
    drawables[WIDGET_TYPE_WINDOW] = renderwindow;

}

