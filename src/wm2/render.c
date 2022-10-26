#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "blit.h"
#include "render.h"

#define DAMAGE_STATE_NONE               0
#define DAMAGE_STATE_MADE               1
#define CMAP_TEXT_COLOR                 0

static struct blit_font fonts[32];

static unsigned int *getcmap(unsigned int state, unsigned int *normal, unsigned int *hover, unsigned int *focus)
{

    switch (state)
    {

    case WIDGET_STATE_FOCUS:
        return focus;

    case WIDGET_STATE_HOVER:
        return hover;

    case WIDGET_STATE_NORMAL:
    default:
        return normal;

    }

}

static unsigned int getrownum(unsigned int fontindex, int line, int y)
{

    struct blit_font *font = &fonts[fontindex];

    return (line - y) / font->lineheight;

}

static int getrowx(struct render_rowinfo *rowinfo, unsigned int halign, int x, int w)
{

    switch (halign)
    {

    case WIDGET_TEXT_HALIGN_LEFT:
        return x;

    case WIDGET_TEXT_HALIGN_CENTER:
        return x + w / 2 - rowinfo->width / 2;

    case WIDGET_TEXT_HALIGN_RIGHT:
        return x + w - rowinfo->width;

    }

    return 0;

}

static int getrowy(struct render_rowinfo *rowinfo, unsigned int valign, int y, int h)
{

    switch (valign)
    {

    case WIDGET_TEXT_VALIGN_TOP:
        return y;

    case WIDGET_TEXT_VALIGN_MIDDLE:
        return y + h / 2 - rowinfo->height / 2 - (rowinfo->lineheight - rowinfo->height) / 2;

    case WIDGET_TEXT_VALIGN_BOTTOM:
        return y + h - rowinfo->height;

    }

    return 0;

}

static unsigned int getrowstart(unsigned int fontindex, char *text, unsigned int length, unsigned int rownum, unsigned int wrap, unsigned int maxw)
{

    unsigned int offset = 0;
    unsigned int rows;
    struct render_rowinfo rowinfo;

    if (!rownum)
        return 0;

    for (rows = 1; (offset = render_getrowinfo(fontindex, text, length, &rowinfo, wrap, maxw, offset)); rows++)
    {

        if (rows == rownum)
            return offset;

    }

    return length;

}

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x1)
{

    struct widget_button *button = widget->data;
    struct render_rowinfo rowinfo;
    static unsigned int cmapnormal[3] = {
        0xE0181818,
        0xE0484848,
        0xE0787878,
    };
    static unsigned int cmaphover[3] = {
        0xE0181818,
        0xE0505050,
        0xE0808080,
    };
    static unsigned int cmapfocus[3] = {
        0xE0181818,
        0xE0585858,
        0xE0888888,
    };
    static unsigned int cmaptext[1] = {
        0xE0FFFFFF,
    };

    blit_panel(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x1, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus));

    if (render_getrowinfo(RENDER_FONTBOLD, pool_getstring(button->label), pool_getcstringlength(button->label), &rowinfo, WIDGET_TEXT_WRAP_NONE, 0, 0))
    {

        int rx = getrowx(&rowinfo, WIDGET_TEXT_HALIGN_CENTER, widget->position.x, widget->size.w);
        int ry = getrowy(&rowinfo, WIDGET_TEXT_VALIGN_MIDDLE, widget->position.y, widget->size.h);

        if (util_intersects(line, ry, ry + rowinfo.height))
            blit_textnormal(display, &fonts[RENDER_FONTBOLD], getcmap(widget->state, cmaptext, cmaptext, cmaptext)[CMAP_TEXT_COLOR], pool_getstring(button->label), rowinfo.chars, rx, ry, line, x0, x1);

    }

}

static void renderfill(struct blit_display *display, struct widget *widget, int line, int x0, int x1)
{

    struct widget_fill *fill = widget->data;

    blit_line(display, fill->color, line, x0, x1);

}

/*
static void rendergradient(struct blit_display *display, struct widget *widget, int line, int x0, int x1)
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

    blit_line(display, color, line, x0, x1);

}
*/

static void renderimage(struct blit_display *display, struct widget *widget, int line, int x0, int x1)
{

    struct widget_image *image = widget->data;

    switch (image->type)
    {

    case WIDGET_IMAGE_TYPE_INTERNAL:
        blit_cmap32line(display, widget->position.x, image->data, widget->size.w, image->cmap, line - widget->position.y);

        break;

    case WIDGET_IMAGE_TYPE_PCX:
        blit_pcx(display, line, pool_getstring(image->source), widget->position.x, widget->position.y, x0, x1);

        break;

    }

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x1)
{

    struct widget_select *select = widget->data;
    struct render_rowinfo rowinfo;
    unsigned int extra;
    static unsigned int cmapnormal[3] = {
        0xE0181818,
        0xE0484848,
        0xE0787878,
    };
    static unsigned int cmaphover[3] = {
        0xE0181818,
        0xE0505050,
        0xE0808080,
    };
    static unsigned int cmapfocus[3] = {
        0xE0181818,
        0xE0585858,
        0xE0888888,
    };
    static unsigned int cmaptext[1] = {
        0xE0FFFFFF,
    };

    render_getrowinfo(RENDER_FONTNORMAL, "X", 1, &rowinfo, 0, 0, 0);

    extra = rowinfo.width + RENDER_SELECT_PADDING_WIDTH * 2;

    blit_panel(display, widget->position.x, widget->position.y, extra, widget->size.h, line, x0, x1, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus));
    blit_panel(display, widget->position.x + extra, widget->position.y, widget->size.w - extra, widget->size.h, line, x0, x1, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus));

    if (render_getrowinfo(RENDER_FONTNORMAL, pool_getstring(select->label), pool_getcstringlength(select->label), &rowinfo, WIDGET_TEXT_WRAP_NONE, 0, 0))
    {

        int rx = getrowx(&rowinfo, WIDGET_TEXT_HALIGN_CENTER, widget->position.x + extra, widget->size.w - extra);
        int ry = getrowy(&rowinfo, WIDGET_TEXT_VALIGN_MIDDLE, widget->position.y, widget->size.h);

        if (util_intersects(line, ry, ry + rowinfo.height))
            blit_textnormal(display, &fonts[RENDER_FONTNORMAL], getcmap(widget->state, cmaptext, cmaptext, cmaptext)[CMAP_TEXT_COLOR], pool_getstring(select->label), rowinfo.chars, rx, ry, line, x0, x1);

    }

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x1)
{

    struct widget_text *text = widget->data;
    unsigned int fontindex = (text->weight == WIDGET_TEXT_WEIGHT_BOLD) ? RENDER_FONTBOLD : RENDER_FONTNORMAL;
    unsigned int rownum = getrownum(fontindex, line, widget->position.y);
    struct render_rowinfo rowinfo;
    unsigned int roff = (rownum) ? 0 : text->firstrowoffset;
    unsigned int rw = widget->size.w - roff;
    static unsigned int cmapnormal[1] = {
        0xE0E0E0E0,
    };
    static unsigned int cmaphover[1] = {
        0xE0E0E0E0,
    };
    static unsigned int cmapfocus[1] = {
        0xE0F0F0F0,
    };

    /* Rudimentary caching */
    if (text->rownum == 0x00FFFFFF || text->rownum != rownum)
    {

        if (rownum > text->rownum)
            text->rowstart = render_getrowinfo(fontindex, pool_getstring(text->content), pool_getcstringlength(text->content), &rowinfo, text->wrap, rw, text->rowstart);
        else
            text->rowstart = getrowstart(fontindex, pool_getstring(text->content), pool_getcstringlength(text->content), rownum, text->wrap, widget->size.w);

        text->rownum = rownum;

    }

    if (render_getrowinfo(fontindex, pool_getstring(text->content), pool_getcstringlength(text->content), &rowinfo, text->wrap, rw, text->rowstart))
    {

        int rx = getrowx(&rowinfo, text->halign, widget->position.x + roff, rw);
        int ry = getrowy(&rowinfo, text->valign, widget->position.y + rownum * rowinfo.lineheight, widget->size.h);

        switch (text->mode)
        {

        case WIDGET_TEXT_MODE_NORMAL:
            blit_textnormal(display, &fonts[fontindex], getcmap(widget->state, cmapnormal, cmaphover, cmapfocus)[CMAP_TEXT_COLOR], pool_getstring(text->content) + text->rowstart, rowinfo.chars, rx, ry, line, x0, x1);

            break;

        case WIDGET_TEXT_MODE_INVERTED:
            blit_textinverted(display, &fonts[fontindex], getcmap(widget->state, cmapnormal, cmaphover, cmapfocus)[CMAP_TEXT_COLOR], pool_getstring(text->content) + text->rowstart, rowinfo.chars, rx, ry, line, x0, x1);

            break;

        }

    }

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x1)
{

    static unsigned int cmapnormal[3] = {
        0xE0181818,
        0xE0282828,
        0xE0585858,
    };
    static unsigned int cmaphover[3] = {
        0xE0181818,
        0xE0282828,
        0xE0606060,
    };
    static unsigned int cmapfocus[3] = {
        0xE0181818,
        0xE0282828,
        0xE0686868,
    };

    blit_frame(display, widget->position.x, widget->position.y, widget->size.w, widget->size.h, line, x0, x1, getcmap(widget->state, cmapnormal, cmaphover, cmapfocus));

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x1)
{

    static unsigned int cmaptop[3] = {
        0xE0181818,
        0xE05818B8,
        0xE09858F8,
    };
    static unsigned int cmapmain[3] = {
        0xE0181818,
        0xE0303030,
        0xE0585858,
    };

    blit_panel(display, widget->position.x, widget->position.y, 36, 36, line, x0, x1, getcmap(widget->state, cmaptop, cmaptop, cmaptop));
    blit_panel(display, widget->position.x + 36, widget->position.y, 36, 36, line, x0, x1, getcmap(widget->state, cmaptop, cmaptop, cmaptop));
    blit_panel(display, widget->position.x + 72, widget->position.y, widget->size.w - 108, 36, line, x0, x1, getcmap(widget->state, cmaptop, cmaptop, cmaptop));
    blit_panel(display, widget->position.x + widget->size.w - 36, widget->position.y, 36, 36, line, x0, x1, getcmap(widget->state, cmaptop, cmaptop, cmaptop));
    blit_panel(display, widget->position.x, widget->position.y + 36, widget->size.w, widget->size.h - 36, line, x0, x1, getcmap(widget->state, cmapmain, cmapmain, cmapmain));

}

static void renderwidget(struct blit_display *display, struct widget *widget, int line, int x0, int x1)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        renderbutton(display, widget, line, x0, x1);

        break;

    case WIDGET_TYPE_FILL:
        renderfill(display, widget, line, x0, x1);

        break;

    case WIDGET_TYPE_IMAGE:
        renderimage(display, widget, line, x0, x1);

        break;

    case WIDGET_TYPE_SELECT:
        renderselect(display, widget, line, x0, x1);

        break;

    case WIDGET_TYPE_TEXT:
        rendertext(display, widget, line, x0, x1);

        break;

    case WIDGET_TYPE_TEXTBOX:
        rendertextbox(display, widget, line, x0, x1);

        break;

    case WIDGET_TYPE_WINDOW:
        renderwindow(display, widget, line, x0, x1);

        break;

    }

}

unsigned int render_getrowinfo(unsigned int fontindex, char *text, unsigned int length, struct render_rowinfo *rowinfo, unsigned int wrap, unsigned int maxw, unsigned int offset)
{

    struct blit_font *font = &fonts[fontindex];
    unsigned int si = 0;
    unsigned int w = 0;
    unsigned int sw = 0;
    unsigned int h = 0;
    unsigned int sh = 0;
    unsigned int i;

    if (offset >= length)
        return 0;

    rowinfo->chars = 0;
    rowinfo->width = 0;
    rowinfo->height = 0;
    rowinfo->newline = 0;
    rowinfo->lineheight = fonts[fontindex].lineheight;

    for (i = offset; i < length; i++)
    {

        struct pcf_metricsdata metricsdata;
        unsigned short index;

        if (text[i] == ' ')
        {

            si = i;
            sw = w;
            sh = h;

        }

        if (text[i] == '\n')
        {

            rowinfo->newline = 1;

            break;

        }

        index = pcf_getindex(font->data, text[i]);

        pcf_readmetricsdata(font->data, index, &metricsdata);

        if (wrap != WIDGET_TEXT_WRAP_NONE && w + metricsdata.width >= maxw)
        {

            if (wrap == WIDGET_TEXT_WRAP_WORD && si)
            {

                i = si;
                w = sw;
                h = sh;

            }

            else
            {

                i--;

            }

            break;

        }

        w += metricsdata.width;
        h = util_max(h, metricsdata.ascent + metricsdata.descent);

    }

    rowinfo->width = w;
    rowinfo->height = h;
    rowinfo->chars = i - offset;

    return i + 1;

}

unsigned int render_gettextinfo(unsigned int fontindex, char *text, unsigned int length, struct render_textinfo *textinfo, unsigned int wrap, unsigned int offw, unsigned int maxw)
{

    unsigned int offset = 0;

    if ((offset = render_getrowinfo(fontindex, text, length, &textinfo->last, wrap, maxw - offw, offset)))
    {

        textinfo->last.width += offw;
        textinfo->lineheight = fonts[fontindex].lineheight;
        textinfo->width = textinfo->last.width;
        textinfo->height = textinfo->last.height;
        textinfo->rows = 1;

        while ((offset = render_getrowinfo(fontindex, text, length, &textinfo->last, wrap, maxw, offset)))
        {

            textinfo->width = util_max(textinfo->width, textinfo->last.width);
            textinfo->height += textinfo->last.height;
            textinfo->rows++;

        }

    }

    return length;

}

void render_damage(struct blit_display *display, int x0, int y0, int x1, int y1)
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

void render_render(struct blit_display *display)
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

                struct widget *widget = current->data;

                if (util_intersects(line, widget->position.y, widget->position.y + widget->size.h))
                {

                    int x0 = util_max(widget->position.x, display->damage.position0.x);
                    int x1 = util_min(widget->position.x + widget->size.w, display->damage.position1.x);

                    renderwidget(display, widget, line, x0, x1);

                }

            }

            blit(display, line);

        }

        display->damage.state = DAMAGE_STATE_NONE;

    }

}

void render_setfont(unsigned int fontindex, void *data, unsigned int lineheight, unsigned int padding)
{

    struct blit_font *font = &fonts[fontindex];

    font->data = data;
    font->bitmapdata = pcf_getbitmapdata(font->data);
    font->bitmapalign = pcf_getbitmapalign(font->data);
    font->lineheight = lineheight;
    font->padding = padding;

}

void render_setup(struct blit_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    display->framebuffer = framebuffer;
    display->size.w = w;
    display->size.h = h;
    display->bpp = bpp;

    render_damage(display, 0, 0, display->size.w, display->size.h);

}

