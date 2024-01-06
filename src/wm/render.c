#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"
#include "strpool.h"
#include "pool.h"
#include "blit.h"
#include "cmap.h"
#include "render.h"

struct
{

    unsigned int hasdamage;
    struct util_position position0;
    struct util_position position2;

} area;

struct cacherow
{

    int num;
    int rx;
    int ry;
    unsigned int istart;
    unsigned int iend;
    unsigned int length;
    struct text_font *font;
    struct widget *widget;

};

static void (*calls[32])(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my);
static struct cacherow cacherows[512];
static unsigned int nrows;

static struct cacherow *getcacherow(struct widget *widget, unsigned int num)
{

    unsigned int i;

    for (i = 0; i < nrows; i++)
    {

        struct cacherow *row = &cacherows[i];

        if (row->num == num && row->widget == widget)
            return row;

    }

    return 0;

}

static void addcacherow(struct widget *widget, unsigned int num, unsigned int weight, unsigned int paddingx, unsigned int paddingy, unsigned int halign, unsigned int valign, int offx, int offy, int text, unsigned int wrap)
{

    struct cacherow *row = getcacherow(widget, num);

    if (!row)
    {

        struct cacherow *cacherow = &cacherows[nrows++];
        struct text_font *font = pool_getfont(weight);
        char *textstring = strpool_getstring(text);
        unsigned int textlength = strpool_getcstringlength(text);
        unsigned int icurrent = text_getrowstart(font, textstring, textlength, num, wrap, widget->bb.w, offx);
        struct text_rowinfo rowinfo;

        text_getrowinfo(&rowinfo, font, textstring, textlength, wrap, widget->bb.w, icurrent);

        cacherow->num = num;
        cacherow->rx = text_getrowx(&rowinfo, halign, paddingx, widget->bb.w - offx) + offx;
        cacherow->ry = text_getrowy(&rowinfo, valign, paddingy, widget->bb.h - offy) + offy + rowinfo.lineheight * num;
        cacherow->istart = rowinfo.istart;
        cacherow->iend = rowinfo.iend;
        cacherow->length = rowinfo.length;
        cacherow->font = font;
        cacherow->widget = widget;

    }

}

static void rendercacherow(struct blit_display *display, struct widget *widget, unsigned int num, unsigned int line, unsigned int text, int x0, int x2, unsigned int markstart, unsigned int markend, unsigned int enablecursor, unsigned int cursor, unsigned int *cmap)
{

    struct cacherow *row = getcacherow(widget, num);

    if (row && row->length)
    {

        if (util_intersects(line, widget->bb.y + row->ry, widget->bb.y + row->ry + row->font->lineheight))
        {

            unsigned int mstart = (markstart > row->istart) ? markstart - row->istart : 0;
            unsigned int mend = (markend > row->istart) ? markend - row->istart : 0;

            if (mend < mstart)
            {

                unsigned int temp = mstart;

                mstart = mend;
                mend = temp;

            }

            blit_text(display, row->font, strpool_getstring(text) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, mstart, mend, enablecursor, cursor, cmap);

        }

    }

}

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_button *button = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));
    rendercacherow(display, widget, 0, line, button->label, x0, x2, 0, 0, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

}

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_choice *choice = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));
    rendercacherow(display, widget, 0, line, choice->label, x0, x2, 0, 0, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

}

static void renderfill(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_fill *fill = widget->data;

    blit_line(display, fill->color, x0, x2);

}

/*
static void rendergradient(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
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

    blit_line(display, color, x0, x2);

}
*/

static void renderimage(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_image *image = widget->data;

    switch (image->mimetype)
    {

    case ATTR_MIMETYPE_FUDGEMOUSE:
        blit_mouse(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));

        break;

    case ATTR_MIMETYPE_PCX:
        blit_pcx(display, line, strpool_getstring(image->source), widget->bb.x, widget->bb.y, x0, x2);

        break;

    }

}

static void renderlayout(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

}

static void renderlistbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_listbox *listbox = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, (listbox->mode == ATTR_MODE_READONLY) ? 0 : 4));

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_select *select = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));
    blit_iconarrowdown(display, widget->bb.x + widget->bb.w - CONFIG_SELECT_PADDING_WIDTH - widget->bb.h / 2, widget->bb.y, widget->bb.h, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 12, 0));
    rendercacherow(display, widget, 0, line, select->label, x0, x2, 0, 0, 0, 0, cmap_get(widget->state, widget->type, 13, 0));

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    unsigned int rownum = (line - widget->bb.y) / font->lineheight;

    if (rownum < text->rows)
        rendercacherow(display, widget, rownum, line, text->content, x0, x2, text->markstart, text->markend, text->enablecursor, text->cursor, cmap_get(widget->state, widget->type, 0, 0));

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbox *textbox = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, (textbox->mode == ATTR_MODE_READONLY) ? 12 : 0, (textbox->mode == ATTR_MODE_READONLY) ? 0 : 4));

}

static void rendertextbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbutton *textbutton = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));
    rendercacherow(display, widget, 0, line, textbutton->label, x0, x2, 0, 0, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_window *window = widget->data;
    unsigned int onhamburger = util_intersects(mx, widget->bb.x, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH) && util_intersects(my, widget->bb.y, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onminimize = util_intersects(mx, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH * 2) && util_intersects(my, widget->bb.y, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onx = util_intersects(mx, widget->bb.x + widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.x + widget->bb.w) && util_intersects(my, widget->bb.y, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT);

    blit_frame(display, widget->bb.x, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH * 2, widget->bb.y, widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH * 3, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->bb.x + widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->bb.x, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->bb.w, widget->bb.h - CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 4, 0));
    rendercacherow(display, widget, 0, line, window->title, x0, x2, 0, 0, 0, 0, cmap_get(widget->state, widget->type, 11, 0));
    blit_iconhamburger(display, widget->bb.x, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onhamburger) ? 1 : 0));
    blit_iconminimize(display, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onminimize) ? 1 : 0));
    blit_iconx(display, widget->bb.x + widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onx) ? 1 : 0));

}

void render_damage(int x0, int y0, int x2, int y2)
{

    if (area.hasdamage)
    {

        area.position0.x = util_min(x0, area.position0.x);
        area.position0.y = util_min(y0, area.position0.y);
        area.position2.x = util_max(x2, area.position2.x);
        area.position2.y = util_max(y2, area.position2.y);

    }

    else
    {

        area.position0.x = x0;
        area.position0.y = y0;
        area.position2.x = x2;
        area.position2.y = y2;

    }

    area.hasdamage = 1;

}

void render_undamage(void)
{

    area.hasdamage = 0;

}

static void updatecache(struct blit_display *display)
{

    int line;

    nrows = 0;

    for (line = 0; line < display->size.h; line++)
    {

        struct list_item *current = 0;

        while ((current = pool_next(current)))
        {

            struct widget *widget = current->data;

            if (widget_intersectsy(widget, line))
            {

                if (widget->type == WIDGET_TYPE_BUTTON)
                {

                    struct widget_button *button = widget->data;

                    addcacherow(widget, 0, ATTR_WEIGHT_BOLD, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, 0, 0, button->label, ATTR_WRAP_NONE);

                }

                if (widget->type == WIDGET_TYPE_CHOICE)
                {

                    struct widget_choice *choice = widget->data;

                    addcacherow(widget, 0, ATTR_WEIGHT_NORMAL, CONFIG_CHOICE_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, 0, 0, choice->label, ATTR_WRAP_NONE);

                }

                if (widget->type == WIDGET_TYPE_SELECT)
                {

                    struct widget_select *select = widget->data;

                    addcacherow(widget, 0, ATTR_WEIGHT_NORMAL, CONFIG_SELECT_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, 0, 0, select->label, ATTR_WRAP_NONE);

                }

                if (widget->type == WIDGET_TYPE_TEXT)
                {

                    struct widget_text *text = widget->data;

                    addcacherow(widget, (line - widget->bb.y) / pool_getfont(text->weight)->lineheight, text->weight, 0, 0, text->halign, text->valign, text->offx, 0, text->content, text->wrap);

                }

                if (widget->type == WIDGET_TYPE_TEXTBUTTON)
                {

                    struct widget_textbutton *textbutton = widget->data;

                    addcacherow(widget, 0, ATTR_WEIGHT_NORMAL, CONFIG_TEXTBUTTON_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, 0, 0, textbutton->label, ATTR_WRAP_NONE);

                }

                if (widget->type == WIDGET_TYPE_WINDOW)
                {

                    struct widget_window *window = widget->data;

                    addcacherow(widget, 0, ATTR_WEIGHT_BOLD, 0, 5, ATTR_HALIGN_CENTER, ATTR_VALIGN_TOP, 0, 0, window->title, ATTR_WRAP_NONE);

                }

            }

        }

    }

}

void render(struct blit_display *display, int mx, int my)
{

    int line;

    updatecache(display);

    for (line = area.position0.y; line < area.position2.y; line++)
    {

        struct list_item *current = 0;

        while ((current = pool_next(current)))
        {

            struct widget *widget = current->data;

            if (widget_intersectsy(widget, line))
            {

                int x0 = util_max(widget->bb.x, area.position0.x);
                int x2 = util_min(widget->bb.x + widget->bb.w, area.position2.x);

                calls[widget->type](display, widget, line, x0, x2, mx, my);

            }

        }

        blit(display, line, area.position0.x, area.position2.x);

    }

}

void render_init(void)
{

    calls[WIDGET_TYPE_BUTTON] = renderbutton;
    calls[WIDGET_TYPE_CHOICE] = renderchoice;
    calls[WIDGET_TYPE_FILL] = renderfill;
    calls[WIDGET_TYPE_IMAGE] = renderimage;
    calls[WIDGET_TYPE_LAYOUT] = renderlayout;
    calls[WIDGET_TYPE_LISTBOX] = renderlistbox;
    calls[WIDGET_TYPE_SELECT] = renderselect;
    calls[WIDGET_TYPE_TEXT] = rendertext;
    calls[WIDGET_TYPE_TEXTBOX] = rendertextbox;
    calls[WIDGET_TYPE_TEXTBUTTON] = rendertextbutton;
    calls[WIDGET_TYPE_WINDOW] = renderwindow;

}

