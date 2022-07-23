#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "render.h"

#define DAMAGE_STATE_NONE               0
#define DAMAGE_STATE_MADE               1
#define CMAP_INDEX_SHADOW               0
#define CMAP_INDEX_MAIN_LIGHT           1
#define CMAP_INDEX_MAIN_NORMAL          2
#define CMAP_INDEX_AREA_NORMAL          3
#define LINESEGMENT_TYPE_RELX0X0        1
#define LINESEGMENT_TYPE_RELX0X1        2
#define LINESEGMENT_TYPE_RELX1X1        3

struct font
{

    unsigned char *data;
    unsigned char *bitmapdata;
    unsigned int bitmapalign;
    unsigned int lineheight;
    unsigned int padding;

};

struct linesegment
{

    unsigned int type;
    int p0;
    int p1;
    unsigned int color;

};

static struct font fonts[1];

static unsigned short getfontindex(struct font *font, unsigned short c)
{

    switch (c)
    {

    case '\n':
        return pcf_getindex(font->data, ' ');

    }

    return pcf_getindex(font->data, c);

}

static void blitline(struct render_display *display, int x0, int x1, unsigned int color, int y)
{

    unsigned int *buffer = display->framebuffer;
    int x;

    x0 = util_between(x0, 0, display->size.w);
    x1 = util_between(x1, 0, display->size.w);

    for (x = x0; x < x1; x++)
        buffer[y * display->size.w + x] = color;

}

static void blitcmap32line(struct render_display *display, struct position *p, void *idata, unsigned int iwidth, unsigned int *cmap, int y)
{

    unsigned int *buffer = display->framebuffer;
    unsigned char *data = idata;
    unsigned int w = (p->x + iwidth >= display->size.w) ? display->size.w - p->x : iwidth;
    int x;

    for (x = 0; x < w; x++)
    {

        unsigned int soffset = (y * iwidth + x);
        unsigned int toffset = (p->y * display->size.w + p->x) + (y * display->size.w + x);

        if (data[soffset] != 0xFF)
            buffer[toffset] = cmap[data[soffset]];

    }

}

static void paintlinesegments(struct render_display *display, int x0, int x1, unsigned int *cmap, struct linesegment *ls, unsigned int n, int y)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct linesegment *p = &ls[i];
        int p0;
        int p1;

        switch (p->type)
        {

        case LINESEGMENT_TYPE_RELX0X0:
            p0 = x0 + p->p0;
            p1 = x0 + p->p1;

            break;

        case LINESEGMENT_TYPE_RELX0X1:
            p0 = x0 + p->p0;
            p1 = x1 + p->p1;

            break;

        case LINESEGMENT_TYPE_RELX1X1:
            p0 = x1 + p->p0;
            p1 = x1 + p->p1;

            break;

        default:
            p0 = x0;
            p1 = x1;

            break;

        }

        p0 = util_max(p0, display->damage.position0.x);
        p1 = util_min(p1, display->damage.position1.x);

        blitline(display, p0, p1, cmap[p->color], y);

    }

}

static void paintfill(struct render_display *display, struct widget *widget, int y)
{

    struct widget_fill *fill = widget->data;
    int x0 = util_max(widget->position.x, display->damage.position0.x);
    int x1 = util_min(widget->position.x + widget->size.w, display->damage.position1.x);

    blitline(display, x0, x1, fill->color, y);

}

static void paintbutton(struct render_display *display, struct widget *widget, int y)
{

    static unsigned int buttoncmapnormal[] = {
        0xFF101010,
        0xFF687888,
        0xFF485868,
        0xFF182838
    };
    static unsigned int buttoncmapfocus[] = {
        0xFF101010,
        0xFF48C888,
        0xFF28A868,
        0xFF182838
    };
    static struct linesegment buttonborder0[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 1, -1, CMAP_INDEX_SHADOW}
    };
    static struct linesegment buttonborder1[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 0, 0, CMAP_INDEX_SHADOW}
    };
    static struct linesegment buttonborder2[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 3, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 3, -3, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -3, 0, CMAP_INDEX_SHADOW}
    };
    static struct linesegment buttonborder3[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 2, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX0X1, 4, -4, CMAP_INDEX_MAIN_NORMAL},
        {LINESEGMENT_TYPE_RELX1X1, -4, -2, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_INDEX_SHADOW}
    };
    static struct linesegment buttonborderlabel[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX0X1, 3, -3, CMAP_INDEX_MAIN_NORMAL},
        {LINESEGMENT_TYPE_RELX1X1, -3, -2, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_INDEX_SHADOW}
    };

    struct widget_button *button = widget->data;
    unsigned int *cmap = (button->focus) ? buttoncmapfocus : buttoncmapnormal;
    int ly = y - widget->position.y;
    struct linesegment *segments;
    unsigned int nsegments;

    if (ly == 0 || ly == widget->size.h - 1)
    {

        segments = buttonborder0;
        nsegments = 1;

    }

    else if (ly == 1 || ly == widget->size.h - 2)
    {

        segments = buttonborder1;
        nsegments = 1;

    }

    else if (ly == 2 || ly == widget->size.h - 3)
    {

        segments = buttonborder2;
        nsegments = 3;

    }

    else if (ly == 3 || ly == widget->size.h - 4)
    {

        segments = buttonborder3;
        nsegments = 5;

    }

    else if (ly > 3 && ly < widget->size.h - 4)
    {

        segments = buttonborderlabel;
        nsegments = 5;

    }

    else
    {

        segments = 0;
        nsegments = 0;

    }

    paintlinesegments(display, widget->position.x, widget->position.x + widget->size.w, cmap, segments, nsegments, y);

}

static void paintchar(struct render_display *display, struct font *font, struct pcf_metricsdata *metricsdata, unsigned int color, int y, unsigned int inverted, unsigned int offset, unsigned int localline, int x, int x0, int x1)
{

    if (localline < metricsdata->ascent + metricsdata->descent)
    {

        unsigned char *data = font->bitmapdata + offset + localline * font->bitmapalign;

        if (inverted)
        {

            unsigned int i;

            for (i = x0; i < x1; i++)
            {

                if (!(data[(i >> 3)] & (0x80 >> (i % 8))))
                    blitline(display, x + i, x + i + 1, color, y);

            }

        }

        else
        {

            unsigned int i;

            for (i = x0; i < x1; i++)
            {

                if (data[(i >> 3)] & (0x80 >> (i % 8)))
                    blitline(display, x + i, x + i + 1, color, y);

            }

        }

    }

}

static void painttextbox(struct render_display *display, struct widget *widget, int y)
{

    struct widget_textbox *textbox = widget->data;
    int x0 = util_max(widget->position.x, display->damage.position0.x);
    int x1 = util_min(widget->position.x + widget->size.w, display->damage.position1.x);
    struct font *font = &fonts[0];
    unsigned int color = 0xFFFFFFFF;

    if (y >= widget->position.y && y < widget->position.y + font->lineheight)
    {

        unsigned int localline = (y - widget->position.y) % font->lineheight;
        int x = widget->position.x;
        unsigned int i;

        for (i = 0; i < textbox->length; i++)
        {

            unsigned short index = getfontindex(font, textbox->content[i]);
            unsigned int offset = pcf_getbitmapoffset(font->data, index);
            struct pcf_metricsdata metricsdata;

            pcf_readmetricsdata(font->data, index, &metricsdata);

            if (util_intersects(x, x0, x1) && util_intersects(x + metricsdata.width, x0, x1))
                paintchar(display, font, &metricsdata, color, y, i == 2, offset, localline, x, 0, metricsdata.width);
            else if (util_intersects(x, x0, x1))
                paintchar(display, font, &metricsdata, color, y, i == 2, offset, localline, x, 0, x1 - x);
            else if (util_intersects(x + metricsdata.width, x0, x1))
                paintchar(display, font, &metricsdata, color, y, i == 2, offset, localline, x, x0 - x, metricsdata.width);

            x += metricsdata.width;

        }

    }

}

static void paintwindow(struct render_display *display, struct widget *widget, int y)
{

    static unsigned int windowcmapnormal[] = {
        0xFF101010,
        0xFF687888,
        0xFF485868,
        0xFF182838
    };
    static unsigned int windowcmapfocus[] = {
        0xFF101010,
        0xFF48C888,
        0xFF28A868,
        0xFF182838
    };
    static struct linesegment windowborder0[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 1, -1, CMAP_INDEX_SHADOW}
    };
    static struct linesegment windowborder1[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 0, 0, CMAP_INDEX_SHADOW}
    };
    static struct linesegment windowborder2[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 3, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 3, -3, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -3, 0, CMAP_INDEX_SHADOW}
    };
    static struct linesegment windowborder3[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 2, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX0X1, 4, -4, CMAP_INDEX_MAIN_NORMAL},
        {LINESEGMENT_TYPE_RELX1X1, -4, -2, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_INDEX_SHADOW}
    };
    static struct linesegment windowbordertitle[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX0X1, 3, -3, CMAP_INDEX_MAIN_NORMAL},
        {LINESEGMENT_TYPE_RELX1X1, -3, -2, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_INDEX_SHADOW}
    };
    static struct linesegment windowborderspacing[7] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 3, 4, CMAP_INDEX_MAIN_NORMAL},
        {LINESEGMENT_TYPE_RELX0X1, 4, -4, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX1X1, -4, -3, CMAP_INDEX_MAIN_NORMAL},
        {LINESEGMENT_TYPE_RELX1X1, -3, -2, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_INDEX_SHADOW}
    };
    static struct linesegment windowborderarea[9] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 3, 4, CMAP_INDEX_MAIN_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 4, 5, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 5, -5, CMAP_INDEX_AREA_NORMAL},
        {LINESEGMENT_TYPE_RELX1X1, -5, -4, CMAP_INDEX_SHADOW},
        {LINESEGMENT_TYPE_RELX1X1, -4, -3, CMAP_INDEX_MAIN_NORMAL},
        {LINESEGMENT_TYPE_RELX1X1, -3, -2, CMAP_INDEX_MAIN_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_INDEX_SHADOW}
    };

    struct widget_window *window = widget->data;
    unsigned int *cmap = (window->focus) ? windowcmapfocus : windowcmapnormal;
    int ly = y - widget->position.y;
    struct linesegment *segments;
    unsigned int nsegments;

    if (ly == 0 || ly == widget->size.h - 1)
    {

        segments = windowborder0;
        nsegments = 1;

    }

    else if (ly == 1 || ly == widget->size.h - 2)
    {

        segments = windowborder1;
        nsegments = 1;

    }

    else if (ly == 2 || ly == widget->size.h - 3)
    {

        segments = windowborder2;
        nsegments = 3;

    }

    else if (ly == 3 || ly == widget->size.h - 4)
    {

        segments = windowborder3;
        nsegments = 5;

    }

    else if (ly >= 4 && ly < 40)
    {

        segments = windowbordertitle;
        nsegments = 5;

    }

    else if (ly == 40)
    {

        segments = windowborderspacing;
        nsegments = 7;

    }

    else if (ly > 40 && ly < widget->size.h - 4)
    {

        segments = windowborderarea;
        nsegments = 9;

    }

    else
    {

        segments = 0;
        nsegments = 0;

    }

    paintlinesegments(display, widget->position.x, widget->position.x + widget->size.w, cmap, segments, nsegments, y);

}

static void paintimage(struct render_display *display, struct widget *widget, int y)
{

    struct widget_image *image = widget->data;

    blitcmap32line(display, &widget->position, image->data, widget->size.w, image->cmap, y - widget->position.y);

}

static void paintwidget(struct render_display *display, struct widget *widget, int y)
{

    if (util_intersects(y, widget->position.y, widget->position.y + widget->size.h))
    {

        switch (widget->type)
        {

        case WIDGET_TYPE_BUTTON:
            paintbutton(display, widget, y);

            break;

        case WIDGET_TYPE_FILL:
            paintfill(display, widget, y);

            break;

        case WIDGET_TYPE_IMAGE:
            paintimage(display, widget, y);

            break;

        case WIDGET_TYPE_TEXTBOX:
            painttextbox(display, widget, y);

            break;

        case WIDGET_TYPE_WINDOW:
            paintwindow(display, widget, y);

            break;

        }

    }

}

void render_damage(struct render_display *display, int x0, int y0, int x1, int y1)
{

    x0 = util_between(x0, 0, display->size.w);
    y0 = util_between(y0, 0, display->size.h);
    x1 = util_between(x1, 0, display->size.w);
    y1 = util_between(y1, 0, display->size.h);

    switch (display->damage.state)
    {

    case DAMAGE_STATE_NONE:
        display->damage.position0.x = x0;
        display->damage.position0.y = y0;
        display->damage.position1.x = x1;
        display->damage.position1.y = y1;

        break;

    case DAMAGE_STATE_MADE:
        display->damage.position0.x = util_min(x0, display->damage.position0.x);
        display->damage.position0.y = util_min(y0, display->damage.position0.y);
        display->damage.position1.x = util_max(x1, display->damage.position1.x);
        display->damage.position1.y = util_max(y1, display->damage.position1.y);

        break;

    }

    display->damage.state = DAMAGE_STATE_MADE;

}

void render_damagebywidget(struct render_display *display, struct widget *widget)
{

    render_damage(display, widget->position.x, widget->position.y, widget->position.x + widget->size.w, widget->position.y + widget->size.h);

}

void render_paint(struct render_display *display, struct widget *rootwidget, struct widget_image *image)
{

    if (!display->framebuffer)
        return;

    if (display->damage.state == DAMAGE_STATE_MADE)
    {

        int y;

        for (y = display->damage.position0.y; y < display->damage.position1.y; y++)
        {

            struct list_item *current = 0;

            while ((current = pool_next(current)))
            {

                struct widget *child = current->data;

                paintwidget(display, child, y);

            }

        }

        display->damage.state = DAMAGE_STATE_NONE;

    }

}

void render_setfont(unsigned int index, void *data, unsigned int lineheight, unsigned int padding)
{

    fonts[index].data = data;
    fonts[index].bitmapdata = pcf_getbitmapdata(fonts[index].data);
    fonts[index].bitmapalign = pcf_getbitmapalign(fonts[index].data);
    fonts[index].lineheight = lineheight;
    fonts[index].padding = padding;

}

void render_setup(struct render_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    display->framebuffer = framebuffer;
    display->size.w = w;
    display->size.h = h;
    display->bpp = bpp;

    render_damage(display, 0, 0, display->size.w, display->size.h);

}

