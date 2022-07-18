#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"
#include "render.h"

#define DAMAGE_STATE_NONE               0
#define DAMAGE_STATE_MADE               1
#define WINDOW_CMAP_SHADOW              0
#define WINDOW_CMAP_MAIN_LIGHT          1
#define WINDOW_CMAP_MAIN_NORMAL         2
#define WINDOW_CMAP_AREA_NORMAL         3
#define LINESEGMENT_TYPE_RELX0X0        1
#define LINESEGMENT_TYPE_RELX0X1        2
#define LINESEGMENT_TYPE_RELX1X1        3

struct linesegment
{

    unsigned int type;
    int p0;
    int p1;
    unsigned int color;

};

static int intersects(int y, int y0, int y1)
{

    return y >= y0 && y < y1;

}

static int capvalue(int x, int min, int max)
{

    if (x < min)
        x = min;

    if (x > max)
        x = max;

    return x;

}

static void blitline(struct render_display *display, unsigned int x0, unsigned int x1, unsigned int color, int y)
{

    unsigned int *buffer = display->framebuffer;
    unsigned int x;

    x0 = capvalue(x0, 0, display->size.w);
    x1 = capvalue(x1, 0, display->size.w);

    for (x = x0; x < x1; x++)
        buffer[y * display->size.w + x] = color;

}

static void blitcmap32line(struct render_display *display, struct position *p, void *idata, unsigned int iwidth, unsigned int *cmap, int y)
{

    unsigned int *buffer = display->framebuffer;
    unsigned char *data = idata;
    unsigned int w = (p->x + iwidth >= display->size.w) ? display->size.w - p->x : iwidth;
    unsigned int x;

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

        blitline(display, p0, p1, cmap[p->color], y);

    }

}

static void paintbackground(struct render_display *display, int y)
{

    if (intersects(y, 0, display->size.h))
        blitline(display, display->damage.position0.x, display->damage.position1.x, 0xFF142434, y);

}

static void paintbutton(struct render_display *display, struct widget *widget, struct widget_button *button, int y)
{

    if (intersects(y, widget->position.y, widget->position.y + widget->size.h))
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
            {LINESEGMENT_TYPE_RELX0X1, 1, -1, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment buttonborder1[1] = {
            {LINESEGMENT_TYPE_RELX0X1, 0, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment buttonborder2[3] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 3, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X1, 3, -3, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -3, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment buttonborder3[5] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 2, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X0, 2, 2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX0X1, 4, -4, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -4, -2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -2, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment buttonbordertitle[5] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 2, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X0, 2, 3, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX0X1, 3, -3, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -3, -2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -2, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment buttonborderspacing[7] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 2, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X0, 2, 3, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX0X0, 3, 4, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX0X1, 4, -4, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX1X1, -4, -3, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -3, -2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -2, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment buttonborderarea[9] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 2, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X0, 2, 3, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX0X0, 3, 4, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX0X0, 4, 5, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X1, 5, -5, WINDOW_CMAP_AREA_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -5, -4, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX1X1, -4, -3, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -3, -2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -2, 0, WINDOW_CMAP_SHADOW}
        };

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

        else if (ly >= 4 && ly < 40)
        {

            segments = buttonbordertitle;
            nsegments = 5;

        }

        else if (ly == 40)
        {

            segments = buttonborderspacing;
            nsegments = 7;

        }

        else if (ly > 40 && ly < widget->size.h - 4)
        {

            segments = buttonborderarea;
            nsegments = 9;

        }

        else
        {

            segments = 0;
            nsegments = 0;

        }

        paintlinesegments(display, widget->position.x, widget->position.x + widget->size.w, cmap, segments, nsegments, y);

    }

}

static void paintwindow(struct render_display *display, struct widget *widget, struct widget_window *window, int y)
{

    if (intersects(y, widget->position.y, widget->position.y + widget->size.h))
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
            {LINESEGMENT_TYPE_RELX0X1, 1, -1, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment windowborder1[1] = {
            {LINESEGMENT_TYPE_RELX0X1, 0, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment windowborder2[3] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 3, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X1, 3, -3, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -3, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment windowborder3[5] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 2, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X0, 2, 2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX0X1, 4, -4, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -4, -2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -2, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment windowbordertitle[5] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 2, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X0, 2, 3, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX0X1, 3, -3, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -3, -2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -2, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment windowborderspacing[7] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 2, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X0, 2, 3, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX0X0, 3, 4, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX0X1, 4, -4, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX1X1, -4, -3, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -3, -2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -2, 0, WINDOW_CMAP_SHADOW}
        };
        static struct linesegment windowborderarea[9] = {
            {LINESEGMENT_TYPE_RELX0X0, 0, 2, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X0, 2, 3, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX0X0, 3, 4, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX0X0, 4, 5, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX0X1, 5, -5, WINDOW_CMAP_AREA_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -5, -4, WINDOW_CMAP_SHADOW},
            {LINESEGMENT_TYPE_RELX1X1, -4, -3, WINDOW_CMAP_MAIN_NORMAL},
            {LINESEGMENT_TYPE_RELX1X1, -3, -2, WINDOW_CMAP_MAIN_LIGHT},
            {LINESEGMENT_TYPE_RELX1X1, -2, 0, WINDOW_CMAP_SHADOW}
        };

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

}

static void paintimage(struct render_display *display, struct widget *widget, struct widget_image *image, int y)
{

    if (intersects(y, widget->position.y, widget->position.y + widget->size.h))
        blitcmap32line(display, &widget->position, image->data, widget->size.w, image->cmap, y - widget->position.y);

}

static void paintwidget(struct render_display *display, struct widget *widget, int y)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_WINDOW:
        paintwindow(display, widget, widget->data, y);

        break;

    case WIDGET_TYPE_BUTTON:
        paintbutton(display, widget, widget->data, y);

        break;

    case WIDGET_TYPE_IMAGE:
        paintimage(display, widget, widget->data, y);

        break;

    }

}

static void paintwidgets(struct render_display *display, struct widget *widget, int y)
{

    struct list_item *current = 0;

    paintwidget(display, widget, y);

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        paintwidget(display, child, y);

    }

}

void render_damage(struct render_display *display, int x0, int y0, int x1, int y1)
{

    x0 = capvalue(x0, 0, display->size.w);
    y0 = capvalue(y0, 0, display->size.h);
    x1 = capvalue(x1, 0, display->size.w);
    y1 = capvalue(y1, 0, display->size.h);

    switch (display->damage.state)
    {

    case DAMAGE_STATE_NONE:
        display->damage.position0.x = x0;
        display->damage.position0.y = y0;
        display->damage.position1.x = x1;
        display->damage.position1.y = y1;

        break;

    case DAMAGE_STATE_MADE:
        if (x0 < display->damage.position0.x)
            display->damage.position0.x = x0;

        if (y0 < display->damage.position0.y)
            display->damage.position0.y = y0;

        if (x1 > display->damage.position1.x)
            display->damage.position1.x = x1;

        if (y1 > display->damage.position1.y)
            display->damage.position1.y = y1;

        break;

    }

    display->damage.state = DAMAGE_STATE_MADE;

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

            paintbackground(display, y);
            paintwidgets(display, rootwidget, y);

        }

        display->damage.state = DAMAGE_STATE_NONE;

    }

}

void render_setup(struct render_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    display->framebuffer = framebuffer;
    display->size.w = w;
    display->size.h = h;
    display->bpp = bpp;

    render_damage(display, 0, 0, display->size.w, display->size.h);

}

