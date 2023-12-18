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

static struct cacherow cacherows[512];
static unsigned int nrows;

static void initcacherow(struct cacherow *cacherow, struct text_rowinfo *rowinfo, unsigned int num, struct text_font *font, unsigned int paddingx, unsigned int paddingy, unsigned int halign, unsigned int valign, struct widget *widget, int offx, int offy)
{

    cacherow->num = num;
    cacherow->rx = text_getrowx(rowinfo, halign, paddingx, widget->bb.w - offx) + offx;
    cacherow->ry = text_getrowy(rowinfo, valign, paddingy, widget->bb.h - offy) + offy;
    cacherow->istart = rowinfo->istart;
    cacherow->iend = rowinfo->iend;
    cacherow->length = rowinfo->length;
    cacherow->font = font;
    cacherow->widget = widget;

}

static struct cacherow *getcacherow(struct widget *widget, unsigned int rownum)
{

    unsigned int i;

    for (i = 0; i < nrows; i++)
    {

        struct cacherow *row = &cacherows[i];

        if (row->num == rownum && row->widget == widget)
            return row;

    }

    return 0;

}

static struct cacherow *addcacherow(struct widget *widget, unsigned int rownum)
{

    struct cacherow *row = getcacherow(widget, rownum);

    if (row)
    {

        return row;

    }

    else
    {

        struct cacherow *nrow = &cacherows[nrows];

        nrows++;

        return nrow;

    }

}

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_button *button = widget->data;
    struct cacherow *row = getcacherow(widget, 0);

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));

    if (row && row->length)
    {

        if (util_intersects(line, widget->bb.y + row->ry, widget->bb.y + row->ry + row->font->lineheight))
            blit_text(display, row->font, strpool_getstring(button->label) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

    }

}

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_choice *choice = widget->data;
    struct cacherow *row = getcacherow(widget, 0);

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));

    if (row && row->length)
    {

        if (util_intersects(line, widget->bb.y + row->ry, widget->bb.y + row->ry + row->font->lineheight))
            blit_text(display, row->font, strpool_getstring(choice->label) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

    }

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

static void renderlistbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_listbox *listbox = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, (listbox->mode == ATTR_MODE_READONLY) ? 0 : 4));

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_select *select = widget->data;
    struct cacherow *row = getcacherow(widget, 0);

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));
    blit_iconarrowdown(display, widget->bb.x + widget->bb.w - CONFIG_SELECT_PADDING_WIDTH - widget->bb.h / 2, widget->bb.y, widget->bb.h, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 12, 0));

    if (row && row->length)
    {

        if (util_intersects(line, widget->bb.y + row->ry, widget->bb.y + row->ry + row->font->lineheight))
            blit_text(display, row->font, strpool_getstring(select->label) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 13, 0));

    }

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    unsigned int rownum = (line - widget->bb.y) / font->lineheight;

    if (rownum < text->rows)
    {

        struct cacherow *row = getcacherow(widget, rownum);

        if (row && row->length)
        {

            unsigned int mstart = (text->markstart > row->istart) ? text->markstart - row->istart : 0;
            unsigned int mend = (text->markend > row->istart) ? text->markend - row->istart : 0;

            if (mend < mstart)
            {

                unsigned int temp = mstart;

                mstart = mend;
                mend = temp;

            }

            blit_text(display, row->font, strpool_getstring(text->content) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, mstart, mend, cmap_get(widget->state, widget->type, 0, 0));

        }

    }

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbox *textbox = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, (textbox->mode == ATTR_MODE_READONLY) ? 12 : 0, (textbox->mode == ATTR_MODE_READONLY) ? 0 : 4));

}

static void rendertextbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbutton *textbutton = widget->data;
    struct cacherow *row = getcacherow(widget, 0);

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));

    if (row && row->length)
    {

        if (util_intersects(line, widget->bb.y + row->ry, widget->bb.y + row->ry + row->font->lineheight))
            blit_text(display, row->font, strpool_getstring(textbutton->label) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

    }

}

static void rendertextedit(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textedit *textedit = widget->data;
    struct text_font *font = pool_getfont(textedit->weight);
    unsigned int rownum = (line - widget->bb.y) / font->lineheight;

    if (rownum < textedit->rows)
    {

        struct cacherow *row = getcacherow(widget, rownum);

        if (row && row->length)
        {

            unsigned int mstart = (textedit->markstart > row->istart) ? textedit->markstart - row->istart : 0;
            unsigned int mend = (textedit->markend > row->istart) ? textedit->markend - row->istart : 0;

            if (mend < mstart)
            {

                unsigned int temp = mstart;

                mstart = mend;
                mend = temp;

            }

            if (textedit->cursor >= row->istart && textedit->cursor < row->istart + row->length)
                blit_textedit(display, row->font, textedit->cursor - row->istart, strpool_getstring(textedit->content) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, mstart, mend, cmap_get(widget->state, widget->type, 0, 0));
            else
                blit_text(display, row->font, strpool_getstring(textedit->content) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, mstart, mend, cmap_get(widget->state, widget->type, 0, 0));

        }

    }

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_window *window = widget->data;
    struct cacherow *row = getcacherow(widget, 0);
    unsigned int onhamburger = util_intersects(mx, widget->bb.x, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH) && util_intersects(my, widget->bb.y, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onminimize = util_intersects(mx, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH * 2) && util_intersects(my, widget->bb.y, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onx = util_intersects(mx, widget->bb.x + widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.x + widget->bb.w) && util_intersects(my, widget->bb.y, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT);

    blit_frame(display, widget->bb.x, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH * 2, widget->bb.y, widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH * 3, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->bb.x + widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));
    blit_frame(display, widget->bb.x, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->bb.w, widget->bb.h - CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 4, 0));

    if (row && row->length)
    {

        if (util_intersects(line, widget->bb.y + row->ry, widget->bb.y + row->ry + row->font->lineheight))
            blit_text(display, row->font, strpool_getstring(window->title) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 11, 0));

    }

    blit_iconhamburger(display, widget->bb.x, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onhamburger) ? 1 : 0));
    blit_iconminimize(display, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onminimize) ? 1 : 0));
    blit_iconx(display, widget->bb.x + widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onx) ? 1 : 0));

}

static void renderwidget(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        renderbutton(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_CHOICE:
        renderchoice(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_FILL:
        renderfill(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_IMAGE:
        renderimage(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_LISTBOX:
        renderlistbox(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_SELECT:
        renderselect(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_TEXT:
        rendertext(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_TEXTBOX:
        rendertextbox(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        rendertextbutton(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_TEXTEDIT:
        rendertextedit(display, widget, line, x0, x2, mx, my);

        break;

    case WIDGET_TYPE_WINDOW:
        renderwindow(display, widget, line, x0, x2, mx, my);

        break;

    }

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

    unsigned int i;

    nrows = 0;

    for (i = 0; i < 1080; i++)
    {

        struct list_item *current = 0;

        while ((current = pool_next(current)))
        {

            struct widget *widget = current->data;

            if (widget_intersectsy(widget, i))
            {

                if (widget->type == WIDGET_TYPE_BUTTON)
                {

                    struct widget_button *button = widget->data;
                    struct cacherow *row = addcacherow(widget, 0);

                    if (row)
                    {

                        struct text_font *font = pool_getfont(ATTR_WEIGHT_BOLD);
                        struct text_rowinfo rowinfo;

                        text_getrowinfo(&rowinfo, font, strpool_getstring(button->label), strpool_getcstringlength(button->label), ATTR_WRAP_NONE, widget->bb.w, 0);
                        initcacherow(row, &rowinfo, 0, font, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, widget, 0, 0);

                    }

                }

                if (widget->type == WIDGET_TYPE_CHOICE)
                {

                    struct widget_choice *choice = widget->data;
                    struct cacherow *row = addcacherow(widget, 0);

                    if (row)
                    {

                        struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
                        struct text_rowinfo rowinfo;

                        text_getrowinfo(&rowinfo, font, strpool_getstring(choice->label), strpool_getcstringlength(choice->label), ATTR_WRAP_NONE, widget->bb.w, 0);
                        initcacherow(row, &rowinfo, 0, font, CONFIG_CHOICE_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, widget, 0, 0);

                    }

                }

                if (widget->type == WIDGET_TYPE_SELECT)
                {

                    struct widget_select *select = widget->data;
                    struct cacherow *row = addcacherow(widget, 0);

                    if (row)
                    {

                        struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
                        struct text_rowinfo rowinfo;

                        text_getrowinfo(&rowinfo, font, strpool_getstring(select->label), strpool_getcstringlength(select->label), ATTR_WRAP_NONE, widget->bb.w, 0);
                        initcacherow(row, &rowinfo, 0, font, CONFIG_CHOICE_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, widget, 0, 0);

                    }

                }

                if (widget->type == WIDGET_TYPE_TEXT)
                {

                    struct widget_text *text = widget->data;
                    struct text_font *font = pool_getfont(text->weight);
                    unsigned int rownum = (i - widget->bb.y) / font->lineheight;
                    struct cacherow *row = addcacherow(widget, rownum);

                    if (row)
                    {

                        unsigned int icurrent = text_getrowstart(font, strpool_getstring(text->content), strpool_getcstringlength(text->content), rownum, text->wrap, widget->bb.w, text->offx);
                        struct text_rowinfo rowinfo;

                        text_getrowinfo(&rowinfo, font, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, widget->bb.w, icurrent);
                        initcacherow(row, &rowinfo, rownum, font, 0, 0, text->halign, text->valign, widget, text->offx, 0);

                    }

                }

                if (widget->type == WIDGET_TYPE_TEXTEDIT)
                {

                    struct widget_textedit *textedit = widget->data;
                    struct text_font *font = pool_getfont(textedit->weight);
                    unsigned int rownum = (i - widget->bb.y) / font->lineheight;
                    struct cacherow *row = addcacherow(widget, rownum);

                    if (row)
                    {

                        unsigned int icurrent = text_getrowstart(font, strpool_getstring(textedit->content), strpool_getcstringlength(textedit->content), rownum, textedit->wrap, widget->bb.w, textedit->offx);
                        struct text_rowinfo rowinfo;

                        text_getrowinfo(&rowinfo, font, strpool_getstring(textedit->content), strpool_getcstringlength(textedit->content), textedit->wrap, widget->bb.w, icurrent);
                        initcacherow(row, &rowinfo, rownum, font, 0, 0, textedit->halign, textedit->valign, widget, textedit->offx, 0);

                    }

                }

                if (widget->type == WIDGET_TYPE_TEXTBUTTON)
                {

                    struct widget_textbutton *textbutton = widget->data;
                    struct cacherow *row = addcacherow(widget, 0);

                    if (row)
                    {

                        struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
                        struct text_rowinfo rowinfo;

                        text_getrowinfo(&rowinfo, font, strpool_getstring(textbutton->label), strpool_getcstringlength(textbutton->label), ATTR_WRAP_NONE, widget->bb.w, 0);
                        initcacherow(row, &rowinfo, 0, font, CONFIG_TEXTBUTTON_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, widget, 0, 0);

                    }

                }

                if (widget->type == WIDGET_TYPE_WINDOW)
                {

                    struct widget_window *window = widget->data;
                    struct cacherow *row = addcacherow(widget, 0);

                    if (row)
                    {

                        struct text_font *font = pool_getfont(ATTR_WEIGHT_BOLD);
                        struct text_rowinfo rowinfo;

                        text_getrowinfo(&rowinfo, font, strpool_getstring(window->title), strpool_getcstringlength(window->title), ATTR_WRAP_NONE, widget->bb.w, 0);
                        initcacherow(row, &rowinfo, 0, font, 0, 5, ATTR_HALIGN_CENTER, ATTR_VALIGN_TOP, widget, 0, 0);

                    }

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

                renderwidget(display, widget, line, x0, x2, mx, my);

            }

        }

        blit(display, line, area.position0.x, area.position2.x);

    }

}

