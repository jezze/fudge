#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "render.h"

#define DAMAGE_STATE_NONE               0
#define DAMAGE_STATE_MADE               1
#define LINESEGMENT_TYPE_RELX0X0        1
#define LINESEGMENT_TYPE_RELX0X1        2
#define LINESEGMENT_TYPE_RELX1X1        3
#define ROWSEGMENT_TYPE_RELY0Y0         1
#define ROWSEGMENT_TYPE_RELY0Y1         2
#define ROWSEGMENT_TYPE_RELY1Y1         3
#define CMAP_BUTTON_SHADOW              0
#define CMAP_BUTTON_MAIN                1
#define CMAP_BUTTON_LIGHT               2
#define CMAP_BUTTON_TEXT                3
#define CMAP_TEXT_TEXT                  4
#define CMAP_TEXTBOX_SHADOW             5
#define CMAP_TEXTBOX_LIGHT              6
#define CMAP_TEXTBOX_DARK               7
#define CMAP_TEXTBOX_TEXT               8
#define CMAP_WINDOW_SHADOW              9
#define CMAP_WINDOW_LIGHT               10
#define CMAP_WINDOW_FRAME               11
#define CMAP_WINDOW_MAIN                12
#define CMAP_WINDOW_TEXT                13

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

struct rowsegment
{

    unsigned int type;
    int p0;
    int p1;
    struct linesegment *segment;
    unsigned int numlines;

};

struct cmap
{

    unsigned int normal;
    unsigned int focus;
    unsigned int hover;

};

static struct cmap cmap[] =
{
    {0xFF081828, 0xFF081828, 0xFF081828},
    {0xFF485868, 0xFF48C888, 0xFF485868},
    {0xFF687888, 0xFF28A868, 0xFFA8B8C8},
    {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
    {0xFFA8B8C8, 0xFFA8B8C8, 0xFFA8B8C8},
    {0xFF081828, 0xFF081828, 0xFF081828},
    {0xFF485868, 0xFF485868, 0xFF687888},
    {0xFF081828, 0xFF081828, 0xFF081828},
    {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
    {0xFF101010, 0xFF101010, 0xFF101010},
    {0xFF687888, 0xFF48C888, 0xFF687888},
    {0xFF485868, 0xFF28A868, 0xFF485868},
    {0xFF182838, 0xFF182838, 0xFF182838},
    {0xFFA8B8C8, 0xFFFFFFFF, 0xFFA8B8C8}
};

static struct font fonts[32];
static unsigned int linebuffer[3840];

static unsigned int getcolor(unsigned int index, unsigned int state)
{

    struct cmap *c = &cmap[index];

    switch (state)
    {

    case WIDGET_STATE_NORMAL:
        return c->normal;

        break;

    case WIDGET_STATE_FOCUS:
        return c->focus;

        break;

    case WIDGET_STATE_HOVER:
        return c->hover;

        break;

    }

    return 0;

}

static unsigned int getrownum(unsigned int index, int line, int y, unsigned int padding)
{

    struct font *font = &fonts[index];

    return (line - y - padding) / font->lineheight;

}

static unsigned int getrowstart(unsigned int index, char *text, unsigned int length, unsigned int rownum, unsigned int maxw)
{

    unsigned int offset = 0;
    unsigned int rows;
    struct render_rowinfo rowinfo;

    if (!rownum)
        return 0;

    for (rows = 1; (offset = render_getrowinfo(index, text, length, &rowinfo, maxw, offset)); rows++)
    {

        if (rows == rownum)
            return offset;

    }

    return length;

}

static struct rowsegment *findrowsegment(struct widget *widget, struct rowsegment *rowsegments, unsigned int length, int line)
{

    int lline = line - widget->position.y;
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        struct rowsegment *current = &rowsegments[i];

        switch (current->type)
        {

        case ROWSEGMENT_TYPE_RELY0Y0:
            if (util_intersects(lline, current->p0, current->p1))
                return current;

            break;

        case ROWSEGMENT_TYPE_RELY0Y1:
            if (util_intersects(lline, current->p0, widget->size.h + current->p1))
                return current;

            break;

        case ROWSEGMENT_TYPE_RELY1Y1:
            if (util_intersects(lline, widget->size.h + current->p0, widget->size.h + current->p1))
                return current;

            break;

        }

    }

    return 0;

}

static void blitline(struct render_display *display, unsigned int color, int line, int x0, int x1)
{

    while (x0 < x1)
        linebuffer[x0++] = color;

}

static void blitcmap32line(struct render_display *display, struct position *p, void *idata, unsigned int iwidth, unsigned int *cmap, int line)
{

    unsigned char *data = idata;
    unsigned int w = (p->x + iwidth >= display->size.w) ? display->size.w - p->x : iwidth;
    int x;

    for (x = 0; x < w; x++)
    {

        unsigned int soffset = (line * iwidth + x);
        unsigned int toffset = p->x + x;

        if (data[soffset] != 0xFF)
            linebuffer[toffset] = cmap[data[soffset]];

    }

}

static void blitchar(struct render_display *display, unsigned char *data, unsigned int color, int rx, int line, int x0, int x1)
{

    unsigned int i;

    for (i = x0; i < x1; i++)
    {

        if (data[(i >> 3)] & (0x80 >> (i % 8)))
            blitline(display, color, line, rx + i, rx + i + 1);

    }

}

static void blitcharcursor(struct render_display *display, unsigned char *data, unsigned int color, int rx, int line, int x0, int x1)
{

    unsigned int i;

    for (i = x0; i < x1; i++)
    {

        if (!(data[(i >> 3)] & (0x80 >> (i % 8))))
            blitline(display, color, line, rx + i, rx + i + 1);

    }

}

static void blittext(struct render_display *display, unsigned int index, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x1)
{

    struct font *font = &fonts[index];
    unsigned int lline = (line - ry) % font->lineheight;
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        unsigned short index = pcf_getindex(font->data, text[i]);
        unsigned int offset = pcf_getbitmapoffset(font->data, index);
        struct pcf_metricsdata metricsdata;

        pcf_readmetricsdata(font->data, index, &metricsdata);

        if (util_intersects(lline, 0, metricsdata.ascent + metricsdata.descent))
        {

            if (util_intersects(rx, x0, x1) || util_intersects(rx + metricsdata.width, x0, x1))
            {

                unsigned char *data = font->bitmapdata + offset + lline * font->bitmapalign;
                int r0 = util_max(0, x0 - rx);
                int r1 = util_min(x1 - rx, metricsdata.width);

                blitchar(display, data, color, rx, line, r0, r1);

            }

        }

        rx += metricsdata.width;

    }

}

static void blittextcursor(struct render_display *display, unsigned int index, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x1, int cursor)
{

    struct font *font = &fonts[index];
    unsigned int lline = (line - ry) % font->lineheight;
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        unsigned short index = pcf_getindex(font->data, text[i]);
        unsigned int offset = pcf_getbitmapoffset(font->data, index);
        struct pcf_metricsdata metricsdata;

        pcf_readmetricsdata(font->data, index, &metricsdata);

        if (util_intersects(lline, 0, metricsdata.ascent + metricsdata.descent))
        {

            if (util_intersects(rx, x0, x1) || util_intersects(rx + metricsdata.width, x0, x1))
            {

                unsigned char *data = font->bitmapdata + offset + lline * font->bitmapalign;
                int r0 = util_max(0, x0 - rx);
                int r1 = util_min(x1 - rx, metricsdata.width);

                if (i == cursor)
                    blitcharcursor(display, data, color, rx, line, r0, r1);
                else
                    blitchar(display, data, color, rx, line, r0, r1);

            }

        }

        rx += metricsdata.width;

    }

}

static void blitlinesegments(struct render_display *display, int x0, int x1, unsigned int state, struct linesegment *ls, unsigned int n, int line)
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

        blitline(display, getcolor(p->color, state), line, p0, p1);

    }

}

static void paintbutton(struct render_display *display, struct widget *widget, int line, int x0, int x1)
{

    static struct linesegment border0[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 1, -1, CMAP_BUTTON_SHADOW}
    };
    static struct linesegment border2[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_BUTTON_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 2, -2, CMAP_BUTTON_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_BUTTON_SHADOW}
    };
    static struct linesegment border3[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_BUTTON_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_BUTTON_LIGHT},
        {LINESEGMENT_TYPE_RELX0X1, 2, -2, CMAP_BUTTON_MAIN},
        {LINESEGMENT_TYPE_RELX1X1, -2, -1, CMAP_BUTTON_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -1, 0, CMAP_BUTTON_SHADOW}
    };
    static struct linesegment borderlabel[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_BUTTON_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_BUTTON_LIGHT},
        {LINESEGMENT_TYPE_RELX0X1, 2, -2, CMAP_BUTTON_MAIN},
        {LINESEGMENT_TYPE_RELX1X1, -2, -1, CMAP_BUTTON_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -1, 0, CMAP_BUTTON_SHADOW}
    };
    static struct rowsegment rows[9] = {
        {ROWSEGMENT_TYPE_RELY0Y0, 0, 1, border0, 1},
        {ROWSEGMENT_TYPE_RELY0Y0, 1, 2, border2, 3},
        {ROWSEGMENT_TYPE_RELY0Y0, 2, 3, border3, 5},
        {ROWSEGMENT_TYPE_RELY0Y1, 3, -3, borderlabel, 5},
        {ROWSEGMENT_TYPE_RELY1Y1, -3, -2, border3, 5},
        {ROWSEGMENT_TYPE_RELY1Y1, -2, -1, border2, 3},
        {ROWSEGMENT_TYPE_RELY1Y1, -1, 0, border0, 1}
    };

    struct widget_button *button = widget->data;
    char *tt = pool_getstring(button->label);
    unsigned int tl = pool_getcstringlength(button->label);
    struct rowsegment *rs = findrowsegment(widget, rows, 9, line);
    struct render_rowinfo rowinfo;

    blitlinesegments(display, widget->position.x, widget->position.x + widget->size.w, widget->state, rs->segment, rs->numlines, line);

    if (render_getrowinfo(RENDER_FONTBOLD, tt, tl, &rowinfo, widget->size.w, 0))
    {

        unsigned int rx = widget->position.x + (widget->size.w / 2) - (rowinfo.width / 2);
        unsigned int ry = widget->position.y + (widget->size.h / 2) - (rowinfo.height / 2);


        if (util_intersects(line, ry, ry + rowinfo.lineheight))
            blittext(display, RENDER_FONTBOLD, getcolor(CMAP_BUTTON_TEXT, widget->state), tt, rowinfo.chars, rx, ry, line, x0, x1);

    }

}

static void paintfill(struct render_display *display, struct widget *widget, int line, int x0, int x1)
{

    struct widget_fill *fill = widget->data;

    blitline(display, fill->color, line, x0, x1);

}

/*
static void paintgradient(struct render_display *display, struct widget *widget, int line, int x0, int x1)
{

    struct widget_gradient *gradient = widget->data;
    unsigned int color0 = fill->color0;
    unsigned int color1 = fill->color1;
    unsigned int color;

    char a0 = color0 >> 24;
    char r0 = color0 >> 16;
    char g0 = color0 >> 8;
    char b0 = color0 >> 0;
    char a1 = color1 >> 24;
    char r1 = color1 >> 16;
    char g1 = color1 >> 8;
    char b1 = color1 >> 0;

    r0 = util_max(0, r0 - line / 64);
    g0 = util_max(0, g0 - line / 64);
    b0 = util_max(0, b0 - line / 64);

    color = (a0 << 24) | (r0 << 16) | (g0 << 8) | (b0 << 0);

    blitline(display, color, line, x0, x1);

}
*/

static void paintimage(struct render_display *display, struct widget *widget, int line, int x0, int x1)
{

    struct widget_image *image = widget->data;

    blitcmap32line(display, &widget->position, image->data, widget->size.w, image->cmap, line - widget->position.y);

}

static void painttext(struct render_display *display, struct widget *widget, int line, int x0, int x1)
{

    struct widget_text *text = widget->data;
    char *tt = pool_getstring(text->content);
    unsigned int tl = pool_getcstringlength(text->content);
    unsigned int rownum = getrownum(RENDER_FONTNORMAL, line, widget->position.y, 0);
    unsigned int rowstart = getrowstart(RENDER_FONTNORMAL, tt, tl, rownum, widget->size.w);
    struct render_rowinfo rowinfo;

    if (render_getrowinfo(RENDER_FONTNORMAL, tt, tl, &rowinfo, widget->size.w, rowstart))
    {

        unsigned int rx = widget->position.x;
        unsigned int ry = widget->position.y + rownum * rowinfo.lineheight;

        switch (text->align)
        {

        case WIDGET_TEXT_ALIGN_LEFT:
            break;

        case WIDGET_TEXT_ALIGN_CENTER:
            rx += widget->size.w / 2 - rowinfo.width / 2;

            break;

        case WIDGET_TEXT_ALIGN_RIGHT:
            rx += widget->size.w - rowinfo.width;

            break;

        }

        blittext(display, RENDER_FONTNORMAL, getcolor(CMAP_TEXT_TEXT, widget->state), tt + rowstart, rowinfo.chars, rx, ry, line, x0, x1);

    }

}

static void painttextbox(struct render_display *display, struct widget *widget, int line, int x0, int x1)
{

    static struct linesegment border0[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 1, -1, CMAP_TEXTBOX_SHADOW}
    };
    static struct linesegment border2[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_TEXTBOX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 2, -2, CMAP_TEXTBOX_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_TEXTBOX_SHADOW}
    };
    static struct linesegment border3[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_TEXTBOX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 1, -1, CMAP_TEXTBOX_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -1, 0, CMAP_TEXTBOX_SHADOW}
    };
    static struct linesegment border4[4] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_TEXTBOX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 3, CMAP_TEXTBOX_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -3, -1, CMAP_TEXTBOX_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -1, 0, CMAP_TEXTBOX_SHADOW}
    };
    static struct linesegment border5[4] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_TEXTBOX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 3, CMAP_TEXTBOX_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -3, -1, CMAP_TEXTBOX_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -1, 0, CMAP_TEXTBOX_SHADOW}
    };
    static struct linesegment borderlabel[6] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_TEXTBOX_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 3, CMAP_TEXTBOX_LIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 3, 4, CMAP_TEXTBOX_DARK},
        {LINESEGMENT_TYPE_RELX1X1, -4, -3, CMAP_TEXTBOX_DARK},
        {LINESEGMENT_TYPE_RELX1X1, -3, -1, CMAP_TEXTBOX_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -1, 0, CMAP_TEXTBOX_SHADOW}
    };
    static struct rowsegment rows[13] = {
        {ROWSEGMENT_TYPE_RELY0Y0, 0, 1, border0, 1},
        {ROWSEGMENT_TYPE_RELY0Y0, 1, 2, border2, 3},
        {ROWSEGMENT_TYPE_RELY0Y0, 2, 3, border3, 3},
        {ROWSEGMENT_TYPE_RELY0Y0, 3, 4, border4, 4},
        {ROWSEGMENT_TYPE_RELY0Y0, 4, 5, border5, 4},
        {ROWSEGMENT_TYPE_RELY0Y1, 5, -5, borderlabel, 6},
        {ROWSEGMENT_TYPE_RELY1Y1, -5, -4, border5, 4},
        {ROWSEGMENT_TYPE_RELY1Y1, -4, -3, border4, 4},
        {ROWSEGMENT_TYPE_RELY1Y1, -3, -2, border3, 3},
        {ROWSEGMENT_TYPE_RELY1Y1, -2, -1, border2, 3},
        {ROWSEGMENT_TYPE_RELY1Y1, -1, 0, border0, 1}
    };

    struct widget_textbox *textbox = widget->data;
    char *tt = pool_getstring(textbox->content);
    unsigned int tl = pool_getcstringlength(textbox->content);
    unsigned int rownum = getrownum(RENDER_FONTNORMAL, line, widget->position.y, RENDER_TEXTBOX_PADDING_HEIGHT);
    unsigned int rowstart = getrowstart(RENDER_FONTNORMAL, tt, tl, rownum, widget->size.w);
    struct rowsegment *rs = findrowsegment(widget, rows, 13, line);
    struct render_rowinfo rowinfo;

    blitlinesegments(display, widget->position.x, widget->position.x + widget->size.w, widget->state, rs->segment, rs->numlines, line);

    if (render_getrowinfo(RENDER_FONTNORMAL, tt, tl, &rowinfo, widget->size.w, rowstart))
    {

        unsigned int rx = widget->position.x;
        unsigned int ry = widget->position.y + rownum * rowinfo.lineheight + RENDER_TEXTBOX_PADDING_HEIGHT;

        switch (textbox->align)
        {

        case WIDGET_TEXTBOX_ALIGN_LEFT:
            rx += RENDER_TEXTBOX_PADDING_WIDTH;

            break;

        case WIDGET_TEXTBOX_ALIGN_CENTER:
            rx += widget->size.w / 2 - rowinfo.width / 2;

            break;

        case WIDGET_TEXTBOX_ALIGN_RIGHT:
            rx += widget->size.w - rowinfo.width - RENDER_TEXTBOX_PADDING_WIDTH;

            break;

        }

        if (textbox->mode == WIDGET_TEXTBOX_MODE_READONLY)
            blittext(display, RENDER_FONTNORMAL, getcolor(CMAP_TEXTBOX_TEXT, widget->state), tt + rowstart, rowinfo.chars, rx, ry, line, x0, x1);
        else
            blittextcursor(display, RENDER_FONTNORMAL, getcolor(CMAP_TEXTBOX_TEXT, widget->state), tt + rowstart, rowinfo.chars, rx, ry, line, x0, x1, textbox->cursor);

    }

}

static void paintwindow(struct render_display *display, struct widget *widget, int line, int x0, int x1)
{

    static struct linesegment border0[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 1, -1, CMAP_WINDOW_SHADOW}
    };
    static struct linesegment border1[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 0, 0, CMAP_WINDOW_SHADOW}
    };
    static struct linesegment border2[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 3, CMAP_WINDOW_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 3, -3, CMAP_WINDOW_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -3, 0, CMAP_WINDOW_SHADOW}
    };
    static struct linesegment border3[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_WINDOW_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 4, CMAP_WINDOW_LIGHT},
        {LINESEGMENT_TYPE_RELX0X1, 4, -4, CMAP_WINDOW_FRAME},
        {LINESEGMENT_TYPE_RELX1X1, -4, -2, CMAP_WINDOW_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_WINDOW_SHADOW}
    };
    static struct linesegment bordertitle[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_WINDOW_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_WINDOW_LIGHT},
        {LINESEGMENT_TYPE_RELX0X1, 3, -3, CMAP_WINDOW_FRAME},
        {LINESEGMENT_TYPE_RELX1X1, -3, -2, CMAP_WINDOW_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_WINDOW_SHADOW}
    };
    static struct linesegment borderspacing[7] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_WINDOW_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_WINDOW_LIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 3, 4, CMAP_WINDOW_FRAME},
        {LINESEGMENT_TYPE_RELX0X1, 4, -4, CMAP_WINDOW_SHADOW},
        {LINESEGMENT_TYPE_RELX1X1, -4, -3, CMAP_WINDOW_FRAME},
        {LINESEGMENT_TYPE_RELX1X1, -3, -2, CMAP_WINDOW_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_WINDOW_SHADOW}
    };
    static struct linesegment borderarea[9] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_WINDOW_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_WINDOW_LIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 3, 4, CMAP_WINDOW_FRAME},
        {LINESEGMENT_TYPE_RELX0X0, 4, 5, CMAP_WINDOW_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 5, -5, CMAP_WINDOW_MAIN},
        {LINESEGMENT_TYPE_RELX1X1, -5, -4, CMAP_WINDOW_SHADOW},
        {LINESEGMENT_TYPE_RELX1X1, -4, -3, CMAP_WINDOW_FRAME},
        {LINESEGMENT_TYPE_RELX1X1, -3, -2, CMAP_WINDOW_LIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_WINDOW_SHADOW}
    };
    static struct rowsegment rows[11] = {
        {ROWSEGMENT_TYPE_RELY0Y0, 0, 1, border0, 1},
        {ROWSEGMENT_TYPE_RELY0Y0, 1, 2, border1, 1},
        {ROWSEGMENT_TYPE_RELY0Y0, 2, 3, border2, 3},
        {ROWSEGMENT_TYPE_RELY0Y0, 3, 4, border3, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 4, 40, bordertitle, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 40, 41, borderspacing, 7},
        {ROWSEGMENT_TYPE_RELY0Y1, 41, -4, borderarea, 9},
        {ROWSEGMENT_TYPE_RELY1Y1, -4, -3, border3, 5},
        {ROWSEGMENT_TYPE_RELY1Y1, -3, -2, border2, 3},
        {ROWSEGMENT_TYPE_RELY1Y1, -2, -1, border1, 1},
        {ROWSEGMENT_TYPE_RELY1Y1, -1, 0, border0, 1}
    };

    struct widget_window *window = widget->data;
    char *tt = pool_getstring(window->title);
    unsigned int tl = pool_getcstringlength(window->title);
    struct rowsegment *rs = findrowsegment(widget, rows, 11, line);
    struct render_rowinfo rowinfo;

    blitlinesegments(display, widget->position.x, widget->position.x + widget->size.w, widget->state, rs->segment, rs->numlines, line);

    if (render_getrowinfo(RENDER_FONTBOLD, tt, tl, &rowinfo, widget->size.w, 0))
    {

        unsigned int rx = widget->position.x + (widget->size.w / 2) - (rowinfo.width / 2);
        unsigned int ry = widget->position.y + RENDER_WINDOW_BORDER_HEIGHT + (RENDER_WINDOW_TITLE_HEIGHT / 2) - (rowinfo.height / 2);

        if (util_intersects(line, ry, ry + rowinfo.lineheight))
            blittext(display, RENDER_FONTBOLD, getcolor(CMAP_WINDOW_TEXT, widget->state), tt, rowinfo.chars, rx, ry, line, x0, x1);

    }

}

static void paintwidget(struct render_display *display, struct widget *widget, int line)
{

    if (util_intersects(line, widget->position.y, widget->position.y + widget->size.h))
    {

        int x0 = util_max(widget->position.x, display->damage.position0.x);
        int x1 = util_min(widget->position.x + widget->size.w, display->damage.position1.x);

        switch (widget->type)
        {

        case WIDGET_TYPE_BUTTON:
            paintbutton(display, widget, line, x0, x1);

            break;

        case WIDGET_TYPE_FILL:
            paintfill(display, widget, line, x0, x1);

            break;

        case WIDGET_TYPE_IMAGE:
            paintimage(display, widget, line, x0, x1);

            break;

        case WIDGET_TYPE_TEXT:
            painttext(display, widget, line, x0, x1);

            break;

        case WIDGET_TYPE_TEXTBOX:
            painttextbox(display, widget, line, x0, x1);

            break;

        case WIDGET_TYPE_WINDOW:
            paintwindow(display, widget, line, x0, x1);

            break;

        }

    }

}

unsigned int render_getrowinfo(unsigned int index, char *text, unsigned int length, struct render_rowinfo *rowinfo, unsigned int maxw, unsigned int offset)
{

    struct font *font = &fonts[index];
    unsigned int i;

    if (offset >= length)
        return 0;

    rowinfo->chars = 0;
    rowinfo->width = 0;
    rowinfo->height = 0;
    rowinfo->lineheight = fonts[index].lineheight;

    for (i = offset; i < length; i++)
    {

        struct pcf_metricsdata metricsdata;
        unsigned short index;

        if (text[i] == '\n')
            break;

        index = pcf_getindex(font->data, text[i]);

        pcf_readmetricsdata(font->data, index, &metricsdata);

        if (rowinfo->width + metricsdata.width > maxw)
            break;

        rowinfo->width += metricsdata.width;
        rowinfo->height = util_max(rowinfo->height, metricsdata.ascent + metricsdata.descent);

    }

    rowinfo->chars = i - offset;

    return i + 1;

}

unsigned int render_gettextinfo(unsigned int index, char *text, unsigned int length, struct render_textinfo *textinfo, unsigned int maxw)
{

    unsigned int offset = 0;
    struct render_rowinfo rowinfo;

    textinfo->width = 0;
    textinfo->height = 0;
    textinfo->rows = 0;
    textinfo->lineheight = fonts[index].lineheight;

    while ((offset = render_getrowinfo(index, text, length, &rowinfo, maxw, offset)))
    {

        textinfo->width = util_max(textinfo->width, rowinfo.width);
        textinfo->height += rowinfo.height;
        textinfo->rows++;

    }

    return length;

}

void render_damage(struct render_display *display, int x0, int y0, int x1, int y1)
{

    x0 = util_clamp(x0, 0, display->size.w);
    y0 = util_clamp(y0, 0, display->size.h);
    x1 = util_clamp(x1, 0, display->size.w);
    y1 = util_clamp(y1, 0, display->size.h);

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

void render_paint(struct render_display *display, struct widget *rootwidget)
{

    if (!display->framebuffer)
        return;

    if (display->damage.state == DAMAGE_STATE_MADE)
    {

        int line;

        for (line = display->damage.position0.y; line < display->damage.position1.y; line++)
        {

            struct list_item *current = 0;

            while ((current = pool_next(current)))
            {

                struct widget *child = current->data;

                paintwidget(display, child, line);

            }

            buffer_copy((unsigned int *)display->framebuffer + (line * display->size.w) + display->damage.position0.x, linebuffer + display->damage.position0.x, (display->damage.position1.x - display->damage.position0.x) * display->bpp);

        }

        display->damage.state = DAMAGE_STATE_NONE;

    }

}

void render_setfont(unsigned int index, void *data, unsigned int lineheight, unsigned int padding)
{

    struct font *font = &fonts[index];

    font->data = data;
    font->bitmapdata = pcf_getbitmapdata(fonts[index].data);
    font->bitmapalign = pcf_getbitmapalign(fonts[index].data);
    font->lineheight = lineheight;
    font->padding = padding;

}

void render_setup(struct render_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    display->framebuffer = framebuffer;
    display->size.w = w;
    display->size.h = h;
    display->bpp = bpp;

    render_damage(display, 0, 0, display->size.w, display->size.h);

}

