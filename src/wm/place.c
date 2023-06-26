#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "cache.h"
#include "attr.h"
#include "widget.h"
#include "strpool.h"
#include "pool.h"
#include "place.h"

#define INFINITY    50000

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

    util_initposition(&widget->position, x, y);
    util_initsize(&widget->size, util_clamp(w + paddingw * 2, minw, maxw), util_clamp(h + paddingh * 2, minh, maxh));
    util_initposition(&widget->clipposition, clipx, clipy);
    util_initsize(&widget->clipsize, clipw, cliph);

}

static void placechild(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph, unsigned int paddingw, unsigned int paddingh)
{

    struct util_position cpos;
    struct util_size cmax;
    struct util_size cmin;

    util_initposition(&cpos, x + paddingw, y + paddingh);
    util_initsize(&cmax, util_clamp(maxw, 0, maxw - paddingw * 2), util_clamp(maxh, 0, maxh - paddingh * 2));
    util_initsize(&cmin, util_clamp(minw, 0, cmax.w), util_clamp(minh, 0, cmax.h));
    place_widget(widget, cpos.x, cpos.y, offx, cmin.w, cmin.h, cmax.w, cmax.h, clipx, clipy, clipw, cliph);

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

        placechild(child, cpos.x, cpos.y, 0, cmin.w, cmin.h, cmax.w, cmax.h, clipx, clipy, clipw, cliph, paddingw, paddingh);

        total->w = util_max(total->w, ((child->position.x + child->size.w) - x) + marginw + paddingw);
        total->h = util_max(total->h, ((child->position.y + child->size.h) - y) + marginh + paddingh);

    }

}

static void placechildren(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph, unsigned int marginw, unsigned int marginh, unsigned int paddingw, unsigned int paddingh, unsigned int incx, unsigned int incy, struct util_size *total)
{

    unsigned int spans = getnumspans(widget);

    placechildren1(widget, x, y, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, marginw, marginh, paddingw, paddingh, incx, incy, 0, 0, total);

    if (spans)
        placechildren1(widget, x, y, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, marginw, marginh, paddingw, paddingh, incx, incy, (maxw - total->w) / spans, (maxh - total->h) / spans, total);

}

static void placetextflow(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph, unsigned int marginw, unsigned int marginh, unsigned int paddingw, unsigned int paddingh, struct util_size *total)
{

    struct list_item *current = 0;
    unsigned int offy = 0;

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

        cpos.y += offy;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;

            placechild(child, cpos.x, cpos.y, offx, cmin.w, cmin.h, cmax.w, cmax.h, clipx, clipy, clipw, cliph, paddingw, paddingh);

            total->w = util_max(total->w, ((child->position.x + child->size.w) - x) + marginw + paddingw);
            total->h = util_max(total->h, ((child->position.y + child->size.h) - y) + marginh + paddingh);

            offx = text->cachetext.lastrowx;
            offy += text->cachetext.lastrowy;

        }

        else if (child->type == WIDGET_TYPE_TEXTEDIT)
        {

            struct widget_textedit *textedit = child->data;

            placechild(child, cpos.x, cpos.y, offx, cmin.w, cmin.h, cmax.w, cmax.h, clipx, clipy, clipw, cliph, paddingw, paddingh);

            total->w = util_max(total->w, ((child->position.x + child->size.w) - x) + marginw + paddingw);
            total->h = util_max(total->h, ((child->position.y + child->size.h) - y) + marginh + paddingh);

            offx = textedit->cachetext.lastrowx;
            offy += textedit->cachetext.lastrowy;

        }

    }

}

static void clipchildren(struct widget *widget, int x, int y, unsigned int w, unsigned int h, unsigned int marginw, unsigned int marginh)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        util_initposition(&child->clipposition, x + marginw, y + marginh);
        util_initsize(&child->clipsize, util_clamp(w, 0, w - marginw * 2), util_clamp(h, 0, h - marginh * 2));
        clipchildren(child, x, y, w, h, marginw, marginh);

    }

}

static void scrollchildren(struct widget *widget, int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        util_initposition(&child->position, child->position.x + x, child->position.y + y);
        scrollchildren(child, x, y);

    }

}

static void placebutton(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_button *button = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_BOLD);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, strpool_getstring(button->label), strpool_getcstringlength(button->label), ATTR_WRAP_NONE, maxw - CONFIG_BUTTON_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width, rowinfo.lineheight, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, CONFIG_BUTTON_PADDING_WIDTH, CONFIG_BUTTON_PADDING_HEIGHT);
    cache_initrow(&button->cacherow, &rowinfo, font, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, widget->size.w, widget->size.h, 0, 0);

}

static void placechoice(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_choice *choice = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, strpool_getstring(choice->label), strpool_getcstringlength(choice->label), ATTR_WRAP_NONE, maxw - CONFIG_CHOICE_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width, rowinfo.lineheight, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, CONFIG_CHOICE_PADDING_WIDTH, CONFIG_CHOICE_PADDING_HEIGHT);
    cache_initrow(&choice->cacherow, &rowinfo, font, CONFIG_CHOICE_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, widget->size.w, widget->size.h, 0, 0);

}

static void placelayout(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
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

static void placefill(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    placewidget(widget, x, y, maxw, maxh, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);

}

static void placeimagepcx(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_image *image = widget->data;

    /* This should be done in some preload state after placement but before rendering. Left in placement for now. */
    if (!image->cacheimage.loaded)
    {

        if (call_walk_absolute(FILE_L0, strpool_getstring(image->source)))
        {

            struct pcx_header header;

            call_read_all(FILE_L0, &header, sizeof (struct pcx_header), 0);
            util_initsize(&image->cacheimage.size, header.xend - header.xstart + 1, header.yend - header.ystart + 1);

            image->cacheimage.loaded = 1;

        }

    }

    placewidget(widget, x, y, image->cacheimage.size.w, image->cacheimage.size.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);

}

static void placeimage(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_image *image = widget->data;

    switch (image->mimetype)
    {

    case ATTR_MIMETYPE_FUDGEMOUSE:
        placewidget(widget, widget->position.x, widget->position.y, widget->size.w, widget->size.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);

        break;

    case ATTR_MIMETYPE_PCX:
        placeimagepcx(widget, x, y, 0, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    }

}

static void placelistbox(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_listbox *listbox = widget->data;
    struct util_size total;

    placechildren(widget, x, y, 0, 0, maxw, INFINITY, clipx, clipy, clipw, cliph, CONFIG_FRAME_WIDTH, CONFIG_FRAME_HEIGHT, CONFIG_LISTBOX_PADDING_WIDTH, CONFIG_LISTBOX_PADDING_HEIGHT, 0, 1, &total);
    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);
    clipchildren(widget, widget->position.x, widget->position.y, widget->size.w, widget->size.h, CONFIG_FRAME_WIDTH, CONFIG_FRAME_HEIGHT);

    listbox->vscroll = util_clamp(listbox->vscroll, widget->size.h - total.h, 0);

    scrollchildren(widget, 0, listbox->vscroll);

}

static void placeselect(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_select *select = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct text_rowinfo rowinfo;
    struct util_size total;

    text_getrowinfo(&rowinfo, font, strpool_getstring(select->label), strpool_getcstringlength(select->label), ATTR_WRAP_NONE, maxw - CONFIG_SELECT_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width + CONFIG_SELECT_PADDING_WIDTH, rowinfo.lineheight, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, CONFIG_SELECT_PADDING_WIDTH, CONFIG_SELECT_PADDING_HEIGHT);
    cache_initrow(&select->cacherow, &rowinfo, font, CONFIG_SELECT_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, widget->size.w, widget->size.h, 0, 0);
    placechildren(widget, x, widget->position.y + widget->size.h, 0, 0, widget->size.w * 2, INFINITY, clipx, clipy, clipw, cliph, 0, 0, 0, 0, 0, 1, &total);

    if (widget->state != WIDGET_STATE_FOCUS)
        clipchildren(widget, widget->position.x, widget->position.y, 0, 0, 0, 0);

}

static void placetext(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    struct text_info info;

    text_gettextinfo(&info, font, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, maxw, offx);
    placewidget(widget, x, y, info.width, info.height, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);
    cache_inittext(&text->cachetext, info.rows, offx, info.lastrowx, info.lastrowy);

}

static void placetextbox(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_textbox *textbox = widget->data;
    struct util_size total;

    placetextflow(widget, x, y, offx, 0, 0, maxw, INFINITY, clipx, clipy, clipw, cliph, CONFIG_FRAME_WIDTH, CONFIG_FRAME_HEIGHT, CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_TEXTBOX_PADDING_HEIGHT, &total);
    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);
    clipchildren(widget, widget->position.x, widget->position.y, widget->size.w, widget->size.h, CONFIG_FRAME_WIDTH + CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_FRAME_HEIGHT + CONFIG_TEXTBOX_PADDING_HEIGHT);

    textbox->vscroll = util_clamp(textbox->vscroll, widget->size.h - total.h, 0);

    scrollchildren(widget, 0, textbox->vscroll);

}

static void placetextbutton(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_textbutton *textbutton = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, strpool_getstring(textbutton->label), strpool_getcstringlength(textbutton->label), ATTR_WRAP_NONE, maxw - CONFIG_TEXTBUTTON_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width, rowinfo.lineheight, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, CONFIG_TEXTBUTTON_PADDING_WIDTH, CONFIG_TEXTBUTTON_PADDING_HEIGHT);
    cache_initrow(&textbutton->cacherow, &rowinfo, font, CONFIG_TEXTBUTTON_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, widget->size.w, widget->size.h, 0, 0);

}

static void placetextedit(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_textedit *textedit = widget->data;
    struct text_font *font = pool_getfont(textedit->weight);
    struct text_info info;

    text_gettextinfo(&info, font, strpool_getstring(textedit->content), strpool_getcstringlength(textedit->content), textedit->wrap, maxw, offx);
    placewidget(widget, x, y, info.width, info.height, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);
    cache_inittext(&textedit->cachetext, info.rows, offx, info.lastrowx, info.lastrowy);

}

static void placewindow(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    struct widget_window *window = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_BOLD);
    struct text_rowinfo rowinfo;
    struct util_size total;

    text_getrowinfo(&rowinfo, font, strpool_getstring(window->title), strpool_getcstringlength(window->title), ATTR_WRAP_NONE, maxw, 0);
    placewidget(widget, widget->position.x, widget->position.y, widget->size.w, widget->size.h, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph, 0, 0);
    cache_initrow(&window->cacherow, &rowinfo, font, 0, 5, ATTR_HALIGN_CENTER, ATTR_VALIGN_TOP, widget->size.w, widget->size.h, 0, 0);
    placechildren(widget, widget->position.x, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT, 0, 0, widget->size.w, widget->size.h - CONFIG_WINDOW_BUTTON_HEIGHT, clipx, clipy, clipw, cliph, 0, 0, CONFIG_WINDOW_BORDER_WIDTH, CONFIG_WINDOW_BORDER_HEIGHT, 0, 1, &total);

}

void place_widget(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, int clipx, int clipy, unsigned int clipw, unsigned int cliph)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        placebutton(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_CHOICE:
        placechoice(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_LAYOUT:
        placelayout(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_FILL:
        placefill(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_IMAGE:
        placeimage(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_LISTBOX:
        placelistbox(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_SELECT:
        placeselect(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_TEXT:
        placetext(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_TEXTBOX:
        placetextbox(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        placetextbutton(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_TEXTEDIT:
        placetextedit(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    case WIDGET_TYPE_WINDOW:
        placewindow(widget, x, y, offx, minw, minh, maxw, maxh, clipx, clipy, clipw, cliph);

        break;

    }

}

