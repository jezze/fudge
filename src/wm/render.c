#include <fudge.h>
#include <abi.h>
#include <image.h>
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

#define INFINITY    50000

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

struct calls
{

    void (*place)(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph);
    void (*render)(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my);
    void (*cache)(struct widget *widget, int y0, int y2);

};

static struct calls calls[32];
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

static void cachebutton(struct widget *widget, int y0, int y2)
{

    struct widget_button *button = widget->data;

    addcacherow(widget, 0, ATTR_WEIGHT_BOLD, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, 0, 0, button->label, ATTR_WRAP_NONE);

}

static void cachechoice(struct widget *widget, int y0, int y2)
{

    struct widget_choice *choice = widget->data;

    addcacherow(widget, 0, ATTR_WEIGHT_NORMAL, CONFIG_CHOICE_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, 0, 0, choice->label, ATTR_WRAP_NONE);

}

static void cachefill(struct widget *widget, int y0, int y2)
{

}

static void cacheimage(struct widget *widget, int y0, int y2)
{

}

static void cachelayout(struct widget *widget, int y0, int y2)
{

}

static void cachelistbox(struct widget *widget, int y0, int y2)
{

}

static void cacheselect(struct widget *widget, int y0, int y2)
{

    struct widget_select *select = widget->data;

    addcacherow(widget, 0, ATTR_WEIGHT_NORMAL, CONFIG_SELECT_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, 0, 0, select->label, ATTR_WRAP_NONE);

}

static void cachetext(struct widget *widget, int y0, int y2)
{

    struct widget_text *text = widget->data;
    int line;

    for (line = y0; line < y2; line++)
    {

        unsigned int rownum = (line - widget->bb.y) / pool_getfont(text->weight)->lineheight;

        if (!getcacherow(widget, rownum))
            addcacherow(widget, rownum, text->weight, 0, 0, text->halign, text->valign, text->offx, 0, text->content, text->wrap);

    }

}

static void cachetextbox(struct widget *widget, int y0, int y2)
{

}

static void cachetextbutton(struct widget *widget, int y0, int y2)
{

    struct widget_textbutton *textbutton = widget->data;

    addcacherow(widget, 0, ATTR_WEIGHT_NORMAL, CONFIG_TEXTBUTTON_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, 0, 0, textbutton->label, ATTR_WRAP_NONE);

}

static void cachewindow(struct widget *widget, int y0, int y2)
{

    struct widget_window *window = widget->data;

    addcacherow(widget, 0, ATTR_WEIGHT_BOLD, 0, 5, ATTR_HALIGN_CENTER, ATTR_VALIGN_TOP, 0, 0, window->title, ATTR_WRAP_NONE);

}

static unsigned int getnumspans(struct widget *widget)
{

    struct list_item *current = 0;
    unsigned int totalspans = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->span)
            totalspans += child->span;

    }

    return totalspans;

}

static void placewidget(struct widget *widget, int x, int y, int w, int h, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph, unsigned int paddingw, unsigned int paddingh)
{

    util_initbox(&widget->bb, x, y, util_clamp(w + paddingw * 2, minw, maxw), util_clamp(h + paddingh * 2, minh, maxh));
    util_initbox(&widget->clip, clipx, clipy, clipw, cliph);

}

static void placechild(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph, unsigned int paddingw, unsigned int paddingh)
{

    struct util_position cpos;
    struct util_size cmax;
    struct util_size cmin;

    util_initposition(&cpos, x + paddingw, y + paddingh);
    util_initsize(&cmax, util_clamp(maxw, 0, maxw - paddingw * 2), util_clamp(maxh, 0, maxh - paddingh * 2));
    util_initsize(&cmin, util_clamp(minw, 0, cmax.w), util_clamp(minh, 0, cmax.h));

    calls[widget->type].place(widget, cpos.x, cpos.y, cmin.w, cmin.h, cmax.w, cmax.h, clipx, clipy, clipw, cliph);

}

static void placechildren1(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph, unsigned int marginw, unsigned int marginh, unsigned int paddingw, unsigned int paddingh, unsigned int incx, unsigned int incy, unsigned int spanw, unsigned int spanh, struct util_size *total)
{

    struct list_item *current = 0;

    util_initsize(total, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_position cpos;
        struct util_size cmax;
        struct util_size cmin;

        util_initposition(&cpos, x + marginw, y + marginh);
        util_initsize(&cmax, util_clamp(maxw, 0, maxw - marginw * 2), util_clamp(maxh, 0, maxh - marginh * 2));
        util_initsize(&cmin, util_clamp(minw, 0, cmax.w), util_clamp(minh, 0, cmax.h));

        if (incx)
        {

            cpos.x += total->w;
            cmax.w -= total->w;

            if (child->span)
            {

                cmin.w = spanw * child->span;
                cmax.w = spanw * child->span;

            }

        }

        if (incy)
        {

            cpos.y += total->h;
            cmax.h -= total->h;

            if (child->span)
            {

                cmin.h = spanh * child->span;
                cmax.h = spanh * child->span;

            }

        }

        placechild(child, cpos.x, cpos.y, cmin.w, cmin.h, cmax.w, cmax.h, clipx, clipy, clipw, cliph, paddingw, paddingh);

        total->w = util_max(total->w, ((child->bb.x + child->bb.w) - x) + marginw + paddingw);
        total->h = util_max(total->h, ((child->bb.y + child->bb.h) - y) + marginh + paddingh);

    }

}

static void placechildren(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph, unsigned int marginw, unsigned int marginh, unsigned int paddingw, unsigned int paddingh, unsigned int incx, unsigned int incy, struct util_size *total)
{

    unsigned int spans = getnumspans(widget);

    placechildren1(widget, x, y, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, marginw, marginh, paddingw, paddingh, incx, incy, 0, 0, total);

    if (spans)
        placechildren1(widget, x, y, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, marginw, marginh, paddingw, paddingh, incx, incy, (maxw - total->w) / spans, (maxh - total->h) / spans, total);

}

static void placetextflow(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph, unsigned int marginw, unsigned int marginh, unsigned int paddingw, unsigned int paddingh, struct util_size *total)
{

    struct list_item *current = 0;
    unsigned int offx = 0;
    unsigned int offy = 0;

    util_initsize(total, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_position cpos;
        struct util_size cmax;
        struct util_size cmin;

        util_initposition(&cpos, x + marginw, y + marginh + offy);
        util_initsize(&cmax, util_clamp(maxw, 0, maxw - marginw * 2), util_clamp(maxh, 0, maxh - marginh * 2));
        util_initsize(&cmin, util_clamp(minw, 0, cmax.w), util_clamp(minh, 0, cmax.h));

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;

            text->offx = offx;

            placechild(child, cpos.x, cpos.y, cmin.w, cmin.h, cmax.w, cmax.h, clipx, clipy, clipw, cliph, paddingw, paddingh);

            total->w = util_max(total->w, ((child->bb.x + child->bb.w) - x) + marginw + paddingw);
            total->h = util_max(total->h, ((child->bb.y + child->bb.h) - y) + marginh + paddingh);

            offx = text->lastrowx;
            offy += text->lastrowy;

        }

    }

}

static void clipchildren(struct widget *widget, int x, int y, unsigned int w, unsigned int h, unsigned int marginw, unsigned int marginh)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        util_initbox(&child->clip, x + marginw, y + marginh, util_clamp(w, 0, w - marginw * 2), util_clamp(h, 0, h - marginh * 2));
        clipchildren(child, x, y, w, h, marginw, marginh);

    }

}

static void scrollchildren(struct widget *widget, int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        child->bb.x += x;
        child->bb.y += y;

        scrollchildren(child, x, y);

    }

}

static void placebutton(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_button *button = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_BOLD);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, strpool_getstring(button->label), strpool_getcstringlength(button->label), ATTR_WRAP_NONE, maxw - CONFIG_BUTTON_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width, rowinfo.lineheight, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, CONFIG_BUTTON_PADDING_WIDTH, CONFIG_BUTTON_PADDING_HEIGHT);

}

static void placechoice(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_choice *choice = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, strpool_getstring(choice->label), strpool_getcstringlength(choice->label), ATTR_WRAP_NONE, maxw - CONFIG_CHOICE_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width, rowinfo.lineheight, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, CONFIG_CHOICE_PADDING_WIDTH, CONFIG_CHOICE_PADDING_HEIGHT);

}

static void placefill(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    placewidget(widget, x, y, maxw, maxh, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);

}

static void placeimagepcx(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_image *image = widget->data;

    /* This should be done in some preload state after placement but before rendering. Left in placement for now. */
    if (!image->loaded)
    {

        if (call_walk_absolute(FILE_L0, strpool_getstring(image->source)))
        {

            struct pcx_header header;

            call_read_all(FILE_L0, &header, sizeof (struct pcx_header), 0);
            util_initsize(&image->size, header.xend - header.xstart + 1, header.yend - header.ystart + 1);

            image->loaded = 1;

        }

    }

    placewidget(widget, x, y, image->size.w, image->size.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);

}

static void placeimage(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_image *image = widget->data;

    switch (image->mimetype)
    {

    case ATTR_MIMETYPE_FUDGEMOUSE:
        placewidget(widget, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);

        break;

    case ATTR_MIMETYPE_PCX:
        placeimagepcx(widget, x, y, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    }

}

static void placelayout(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_layout *layout = widget->data;
    struct util_size total;

    switch (layout->flow)
    {

    case ATTR_FLOW_DEFAULT:
        placechildren(widget, x, y, 0, 0, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0, layout->padding * CONFIG_LAYOUT_PADDING_WIDTH, layout->padding * CONFIG_LAYOUT_PADDING_HEIGHT, 0, 0, &total);

        break;

    case ATTR_FLOW_HORIZONTAL:
        placechildren(widget, x, y, 0, 0, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0, layout->padding * CONFIG_LAYOUT_PADDING_WIDTH, layout->padding * CONFIG_LAYOUT_PADDING_HEIGHT, 1, 0, &total);

        break;

    case ATTR_FLOW_HORIZONTALSTRETCH:
        placechildren(widget, x, y, 0, maxh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0, layout->padding * CONFIG_LAYOUT_PADDING_WIDTH, layout->padding * CONFIG_LAYOUT_PADDING_HEIGHT, 1, 0, &total);

        break;

    case ATTR_FLOW_VERTICAL:
        placechildren(widget, x, y, 0, 0, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0, layout->padding * CONFIG_LAYOUT_PADDING_WIDTH, layout->padding * CONFIG_LAYOUT_PADDING_HEIGHT, 0, 1, &total);

        break;

    case ATTR_FLOW_VERTICALSTRETCH:
        placechildren(widget, x, y, maxw, 0, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0, layout->padding * CONFIG_LAYOUT_PADDING_WIDTH, layout->padding * CONFIG_LAYOUT_PADDING_HEIGHT, 0, 1, &total);

        break;

    }

    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);

}

static void placelistbox(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_listbox *listbox = widget->data;
    struct util_size total;

    placechildren(widget, x, y, 0, 0, maxw, INFINITY, clipx, clipy, clipw, cliph, CONFIG_FRAME_WIDTH, CONFIG_FRAME_HEIGHT, CONFIG_LISTBOX_PADDING_WIDTH, CONFIG_LISTBOX_PADDING_HEIGHT, 0, 1, &total);
    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);
    clipchildren(widget, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, CONFIG_FRAME_WIDTH, CONFIG_FRAME_HEIGHT);

    listbox->vscroll = util_clamp(listbox->vscroll, widget->bb.h - total.h, 0);

    scrollchildren(widget, 0, listbox->vscroll);

}

static void placeselect(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_select *select = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct text_rowinfo rowinfo;
    struct util_size total;

    text_getrowinfo(&rowinfo, font, strpool_getstring(select->label), strpool_getcstringlength(select->label), ATTR_WRAP_NONE, maxw - CONFIG_SELECT_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width + CONFIG_SELECT_PADDING_WIDTH, rowinfo.lineheight, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, CONFIG_SELECT_PADDING_WIDTH, CONFIG_SELECT_PADDING_HEIGHT);
    placechildren(widget, x, widget->bb.y + widget->bb.h, 0, 0, widget->bb.w * 2, INFINITY, clipx, clipy, clipw, cliph, 0, 0, 0, 0, 0, 1, &total);

    if (widget->state != WIDGET_STATE_FOCUS)
        clipchildren(widget, widget->bb.x, widget->bb.y, 0, 0, 0, 0);

}

static void placetext(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    struct text_info info;

    text_gettextinfo(&info, font, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, maxw, text->offx);

    text->rows = info.rows;
    text->lastrowx = info.lastrowx;
    text->lastrowy = info.lastrowy;

    placewidget(widget, x, y, info.width, info.height, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);

}

static void placetextbox(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_textbox *textbox = widget->data;
    struct list_item *current = 0;
    struct util_size total;

    placetextflow(widget, x, y, 0, 0, maxw, INFINITY, clipx, clipy, clipw, cliph, CONFIG_FRAME_WIDTH, CONFIG_FRAME_HEIGHT, CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_TEXTBOX_PADDING_HEIGHT, &total);
    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);
    clipchildren(widget, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, CONFIG_FRAME_WIDTH + CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_FRAME_HEIGHT + CONFIG_TEXTBOX_PADDING_HEIGHT);

    textbox->vscroll = util_clamp(textbox->vscroll, widget->bb.h - total.h, 0);

    scrollchildren(widget, 0, textbox->vscroll);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;
            struct text_font *font = pool_getfont(text->weight);
            struct text_info info;

            text_gettextinfo(&info, font, strpool_getstring(text->content), textbox->cursor, text->wrap, maxw, text->offx);

            textbox->cursorheight = font->lineheight - font->lineheight / 4;
            textbox->cursorwidth = 2;
            textbox->cursorx = child->bb.x + info.lastrowx;
            textbox->cursory = child->bb.y + info.lastrowy + font->lineheight / 2 - textbox->cursorheight / 2;

        }

    }

}

static void placetextbutton(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_textbutton *textbutton = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, strpool_getstring(textbutton->label), strpool_getcstringlength(textbutton->label), ATTR_WRAP_NONE, maxw - CONFIG_TEXTBUTTON_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width, rowinfo.lineheight, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, CONFIG_TEXTBUTTON_PADDING_WIDTH, CONFIG_TEXTBUTTON_PADDING_HEIGHT);

}

static void placewindow(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_window *window = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_BOLD);
    struct text_rowinfo rowinfo;
    struct util_size total;

    text_getrowinfo(&rowinfo, font, strpool_getstring(window->title), strpool_getcstringlength(window->title), ATTR_WRAP_NONE, maxw, 0);
    placewidget(widget, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);
    placechildren(widget, widget->bb.x, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT, 0, 0, widget->bb.w, widget->bb.h - CONFIG_WINDOW_BUTTON_HEIGHT, clipx, clipy, clipw, cliph, 0, 0, CONFIG_WINDOW_BORDER_WIDTH, CONFIG_WINDOW_BORDER_HEIGHT, 0, 1, &total);

}

static void rendercacherow(struct blit_display *display, struct widget *widget, unsigned int num, unsigned int line, unsigned int text, int x0, int x2, unsigned int markstart, unsigned int markend, unsigned int *cmap)
{

    struct cacherow *row = getcacherow(widget, num);

    if (row && row->length)
    {

        if (util_intersects(line, widget->bb.y + row->ry, widget->bb.y + row->ry + row->font->lineheight))
        {

            unsigned int mstart = util_max(0, util_min(markstart, markend) - row->istart);
            unsigned int mend = util_max(0, util_max(markstart, markend) - row->istart);

            blit_text(display, row->font, strpool_getstring(text) + row->istart, row->length, widget->bb.x + row->rx, widget->bb.y + row->ry, line, x0, x2, mstart, mend, cmap);

        }

    }

}

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_button *button = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));
    rendercacherow(display, widget, 0, line, button->label, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

}

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_choice *choice = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));
    rendercacherow(display, widget, 0, line, choice->label, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

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
    rendercacherow(display, widget, 0, line, select->label, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 13, 0));

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    unsigned int rownum = (line - widget->bb.y) / font->lineheight;

    if (rownum < text->rows)
        rendercacherow(display, widget, rownum, line, text->content, x0, x2, text->markstart, text->markend, cmap_get(widget->state, widget->type, 0, 0));

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbox *textbox = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, (textbox->mode == ATTR_MODE_READONLY) ? 12 : 0, (textbox->mode == ATTR_MODE_READONLY) ? 0 : 4));
    blit_iconcursor(display, textbox->cursorx, textbox->cursory, textbox->cursorwidth, textbox->cursorheight, line, x0, x2, cmap_get(WIDGET_STATE_NORMAL, WIDGET_TYPE_TEXT, 0, 0));

}

static void rendertextbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbutton *textbutton = widget->data;

    blit_frame(display, widget->bb.x, widget->bb.y, widget->bb.w, widget->bb.h, line, x0, x2, cmap_get(widget->state, widget->type, 0, 4));
    rendercacherow(display, widget, 0, line, textbutton->label, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 12, 0));

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
    rendercacherow(display, widget, 0, line, window->title, x0, x2, 0, 0, cmap_get(widget->state, widget->type, 11, 0));
    blit_iconhamburger(display, widget->bb.x, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onhamburger) ? 1 : 0));
    blit_iconminimize(display, widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onminimize) ? 1 : 0));
    blit_iconx(display, widget->bb.x + widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT, line, x0, x2, cmap_get(widget->state, widget->type, 8, (onx) ? 1 : 0));

}

void render_place(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    calls[widget->type].place(widget, x, y, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

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

void render_cache(void)
{

    struct list_item *current = 0;

    nrows = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        calls[widget->type].cache(widget, area.position0.y, area.position2.y);

    }

}

void render_update(struct blit_display *display, int mx, int my)
{

    int line;

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

                calls[widget->type].render(display, widget, line, x0, x2, mx, my);

            }

        }

        blit(display, line, area.position0.x, area.position2.x);

    }

}

static void setupcall(unsigned int type, void (*place)(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph), void (*render)(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my), void (*cache)(struct widget *widget, int y0, int y2))
{

    struct calls *call = &calls[type];

    call->place = place;
    call->cache = cache;
    call->render = render;

}

void render_init(void)
{

    setupcall(WIDGET_TYPE_BUTTON, placebutton, renderbutton, cachebutton);
    setupcall(WIDGET_TYPE_CHOICE, placechoice, renderchoice, cachechoice);
    setupcall(WIDGET_TYPE_FILL, placefill, renderfill, cachefill);
    setupcall(WIDGET_TYPE_IMAGE, placeimage, renderimage, cacheimage);
    setupcall(WIDGET_TYPE_LAYOUT, placelayout, renderlayout, cachelayout);
    setupcall(WIDGET_TYPE_LISTBOX, placelistbox, renderlistbox, cachelistbox);
    setupcall(WIDGET_TYPE_SELECT, placeselect, renderselect, cacheselect);
    setupcall(WIDGET_TYPE_TEXT, placetext, rendertext, cachetext);
    setupcall(WIDGET_TYPE_TEXTBOX, placetextbox, rendertextbox, cachetextbox);
    setupcall(WIDGET_TYPE_TEXTBUTTON, placetextbutton, rendertextbutton, cachetextbutton);
    setupcall(WIDGET_TYPE_WINDOW, placewindow, renderwindow, cachewindow);

}

