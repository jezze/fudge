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

static void hideall(struct widget *widget)
{

    struct list_item *current = 0;

    widget->display = WIDGET_DISPLAY_HIDDEN;

    while ((current = pool_nextin(current, widget)))
        hideall(current->data);

}

static void addtotal(struct util_size *total, struct widget *widget, int x, int y, int paddingw, int paddingh)
{

    total->w = util_max(total->w, ((widget->position.x + widget->size.w) - x) + paddingw);
    total->h = util_max(total->h, ((widget->position.y + widget->size.h) - y) + paddingh);

}

static void placewidget(struct widget *widget, int x, int y, int w, int h, int minw, int minh, int maxw, int maxh, int paddingw, int paddingh)
{

    util_initposition(&widget->position, x, y);
    util_initsize(&widget->size, util_clamp(w + paddingw * 2, minw, maxw), util_clamp(h + paddingh * 2, minh, maxh));

    widget->display = WIDGET_DISPLAY_NORMAL;

}

static void placewidget2(struct widget *widget, int x, int y, int w, int h, int minw, int minh, int maxw, int maxh, int paddingw, int paddingh)
{

    placewidget(widget, x, y, w, h, minw, minh, maxw, maxh, paddingw, paddingh);

    if (maxw < w + paddingw * 2 || maxh < h + paddingh * 2)
        widget->display = WIDGET_DISPLAY_HIDDEN;

}

static void placechild(struct widget *widget, int x, int y, int offx, int minw, int minh, int maxw, int maxh, int paddingw, int paddingh)
{

    struct util_position cpos;
    struct util_size cmax;
    struct util_size cmin;

    util_initposition(&cpos, x + paddingw, y + paddingh);
    util_initsize(&cmin, util_max(0, minw - paddingw * 2), util_max(0, minh - paddingh * 2));
    util_initsize(&cmax, util_max(cmin.w, maxw - paddingw * 2), util_max(cmin.h, maxh - paddingh * 2));
    place_widget(widget, cpos.x, cpos.y, offx, cmin.w, cmin.h, cmax.w, cmax.h);

}

static void placebutton(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_button *button = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_BOLD);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, strpool_getstring(button->label), strpool_getcstringlength(button->label), ATTR_WRAP_NONE, maxw - CONFIG_BUTTON_PADDING_WIDTH * 2, 0);
    placewidget2(widget, x, y, rowinfo.width, rowinfo.lineheight, minw, minh, maxw, maxh, CONFIG_BUTTON_PADDING_WIDTH, CONFIG_BUTTON_PADDING_HEIGHT);
    cache_initrow(&button->cacherow, &rowinfo, font, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, widget->size.w, widget->size.h, 0, 0);

}

static void placechoice(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_choice *choice = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, strpool_getstring(choice->label), strpool_getcstringlength(choice->label), ATTR_WRAP_NONE, maxw - CONFIG_CHOICE_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width, rowinfo.lineheight, minw, minh, maxw, maxh, CONFIG_CHOICE_PADDING_WIDTH, CONFIG_CHOICE_PADDING_HEIGHT);
    cache_initrow(&choice->cacherow, &rowinfo, font, CONFIG_CHOICE_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, widget->size.w, widget->size.h, 0, 0);

}

static void placelayout(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_layout *layout = widget->data;
    struct list_item *current = 0;
    struct util_size total;
    unsigned int totalfits = 0;

    util_initsize(&total, 0, 0);

    switch (layout->form)
    {

    case ATTR_FORM_FLOAT:
        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            placechild(child, x, y, 0, 0, 0, maxw, maxh, layout->padding, layout->padding);
            addtotal(&total, child, x, y, layout->padding, layout->padding);

        }

        break;

    case ATTR_FORM_HORIZONTAL:
        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            if (child->fit)
            {

                totalfits += child->fit;

            }

            else
            {

                placechild(child, x + total.w, y, 0, 0, 0, maxw - total.w, maxh, layout->padding, layout->padding);
                addtotal(&total, child, x, y, layout->padding, layout->padding);

            }

        }

        if (totalfits)
        {

            unsigned int sharedw = (maxw - total.w) / totalfits;

            util_initsize(&total, 0, 0);

            while ((current = pool_nextin(current, widget)))
            {

                struct widget *child = current->data;
                unsigned int cminw = 0;
                unsigned int cmaxw = maxw - total.w;

                if (child->fit)
                {

                    cminw = sharedw * child->fit;
                    cmaxw = sharedw * child->fit;

                }

                placechild(child, x + total.w, y, 0, cminw, 0, cmaxw, maxh, layout->padding, layout->padding);
                addtotal(&total, child, x, y, layout->padding, layout->padding);

            }

        }

        break;

    case ATTR_FORM_MAXIMIZE:
        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            placechild(child, x, y, 0, maxw, maxh, maxw, maxh, layout->padding, layout->padding);
            addtotal(&total, child, x, y, layout->padding, layout->padding);

        }

        break;

    case ATTR_FORM_VERTICAL:
        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            if (child->fit)
            {

                totalfits += child->fit;

            }

            else
            {

                placechild(child, x, y + total.h, 0, 0, 0, maxw, maxh - total.h, layout->padding, layout->padding);
                addtotal(&total, child, x, y, layout->padding, layout->padding);

            }

        }

        if (totalfits)
        {

            unsigned int sharedh = (maxh - total.h) / totalfits;

            util_initsize(&total, 0, 0);

            while ((current = pool_nextin(current, widget)))
            {

                struct widget *child = current->data;
                unsigned int cminh = 0;
                unsigned int cmaxh = maxh - total.h;

                if (child->fit)
                {

                    cminh = sharedh * child->fit;
                    cmaxh = sharedh * child->fit;

                }

                placechild(child, x, y + total.h, 0, 0, cminh, maxw, cmaxh, layout->padding, layout->padding);
                addtotal(&total, child, x, y, layout->padding, layout->padding);

            }

        }

        break;

    }

    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh, 0, 0);

}

static void placefill(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    placewidget(widget, x, y, maxw, maxh, minw, minh, maxw, maxh, 0, 0);

}

static void placeimagepcx(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_image *image = widget->data;

    /* This should be done in some preload state after placement but before rendering. Left in placement for now. */
    if (!image->cacheimage.loaded)
    {

        if (file_walk2(FILE_L0, strpool_getstring(image->source)))
        {

            struct pcx_header header;

            file_readall(FILE_L0, &header, sizeof (struct pcx_header));
            util_initsize(&image->cacheimage.size, header.xend - header.xstart + 1, header.yend - header.ystart + 1);

            image->cacheimage.loaded = 1;

        }

    }

    placewidget(widget, x, y, image->cacheimage.size.w, image->cacheimage.size.h, minw, minh, maxw, maxh, 0, 0);

}

static void placeimage(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_image *image = widget->data;

    switch (image->mimetype)
    {

    case ATTR_MIMETYPE_FUDGEMOUSE:
        break;

    case ATTR_MIMETYPE_PCX:
        placeimagepcx(widget, x, y, 0, minw, minh, maxw, maxh);

        break;

    }

}

static void placelistbox(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_listbox *listbox = widget->data;
    struct list_item *current = 0;
    struct util_size total;

    util_initsize(&total, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        placechild(child, x, y + CONFIG_LISTBOX_PADDING_HEIGHT + total.h, 0, 0, 0, maxw, INFINITY, CONFIG_LISTBOX_PADDING_WIDTH, 0);
        addtotal(&total, child, x, y, 0, 0);

    }

    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh, 0, 0);

    if (listbox->vscroll)
    {

        listbox->vscroll = util_clamp(listbox->vscroll, 0, (total.h > maxh) ? total.h - maxh + CONFIG_LISTBOX_PADDING_HEIGHT * 2 : 0);

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            child->position.y -= listbox->vscroll;

        }

    }

}

static void placeselect(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_select *select = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_BOLD);
    unsigned int extra = CONFIG_SELECT_EXTRA + CONFIG_SELECT_PADDING_WIDTH * 2;
    struct text_rowinfo rowinfo;
    struct list_item *current = 0;

    text_getrowinfo(&rowinfo, font, strpool_getstring(select->label), strpool_getcstringlength(select->label), ATTR_WRAP_NONE, maxw - CONFIG_SELECT_PADDING_WIDTH * 2, 0);
    placewidget2(widget, x, y, rowinfo.width + extra, rowinfo.lineheight, minw, minh, maxw, maxh, CONFIG_SELECT_PADDING_WIDTH, CONFIG_SELECT_PADDING_HEIGHT);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        placechild(child, widget->position.x, widget->position.y + widget->size.h, 0, widget->size.w, 0, INFINITY, INFINITY, 0, 0);

        if (widget->state != WIDGET_STATE_FOCUS)
            hideall(child);

    }

    cache_initrow(&select->cacherow, &rowinfo, font, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, widget->size.w, widget->size.h, extra, 0);

}

static void placetext(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    struct text_info info;

    text_gettextinfo(&info, font, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, maxw, offx);
    placewidget(widget, x, y, info.width, info.height, minw, minh, maxw, maxh, 0, 0);
    cache_inittext(&text->cachetext, info.rows, offx, info.lastrowx, info.lastrowy);

}

static void placetextbox(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_textbox *textbox = widget->data;
    struct list_item *current = 0;
    struct util_size total;
    unsigned int lastrowx = 0;
    unsigned int lastrowy = 0;

    util_initsize(&total, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;

            placechild(child, x, y + lastrowy, lastrowx, 0, 0, maxw, INFINITY, CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_TEXTBOX_PADDING_HEIGHT);
            addtotal(&total, child, x, y, CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_TEXTBOX_PADDING_HEIGHT);

            lastrowx = text->cachetext.lastrowx;
            lastrowy += text->cachetext.lastrowy;

        }

    }

    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh, 0, 0);

    if (textbox->vscroll)
    {

        textbox->vscroll = util_clamp(textbox->vscroll, 0, total.h - 56);

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            child->position.y -= textbox->vscroll;

        }

    }

}

static void placetextbutton(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_textbutton *textbutton = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, strpool_getstring(textbutton->label), strpool_getcstringlength(textbutton->label), ATTR_WRAP_NONE, maxw - CONFIG_TEXTBUTTON_PADDING_WIDTH * 2, 0);
    placewidget2(widget, x, y, rowinfo.width, rowinfo.lineheight, minw, minh, maxw, maxh, CONFIG_TEXTBUTTON_PADDING_WIDTH, CONFIG_TEXTBUTTON_PADDING_HEIGHT);
    cache_initrow(&textbutton->cacherow, &rowinfo, font, CONFIG_TEXTBUTTON_PADDING_WIDTH, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, widget->size.w, widget->size.h, 0, 0);

}

static void placewindow(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->fit)
            placechild(child, widget->position.x, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT, 0, widget->size.w, widget->size.h - CONFIG_WINDOW_BUTTON_HEIGHT, widget->size.w, widget->size.h - CONFIG_WINDOW_BUTTON_HEIGHT, CONFIG_WINDOW_BORDER_WIDTH, CONFIG_WINDOW_BORDER_HEIGHT);
        else
            placechild(child, widget->position.x, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT, 0, 0, 0, widget->size.w, widget->size.h - CONFIG_WINDOW_BUTTON_HEIGHT, CONFIG_WINDOW_BORDER_WIDTH, CONFIG_WINDOW_BORDER_HEIGHT);

    }

}

void place_widget(struct widget *widget, int x, int y, int offx, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        placebutton(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_CHOICE:
        placechoice(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_LAYOUT:
        placelayout(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_FILL:
        placefill(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_IMAGE:
        placeimage(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_LISTBOX:
        placelistbox(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_SELECT:
        placeselect(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_TEXT:
        placetext(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_TEXTBOX:
        placetextbox(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        placetextbutton(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_WINDOW:
        placewindow(widget, x, y, offx, minw, minh, maxw, maxh);

        break;

    }

}

