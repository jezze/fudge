#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "widget.h"
#include "pool.h"
#include "place.h"

static void addtotal(struct widget_size *total, struct widget *widget, int x, int y, int paddingw, int paddingh)
{

    total->w = util_max(total->w, ((widget->position.x + widget->size.w) - x) + paddingw);
    total->h = util_max(total->h, ((widget->position.y + widget->size.h) - y) + paddingh);

}

static void placewidget(struct widget *widget, int x, int y, int w, int h, int minw, int minh, int maxw, int maxh)
{

    widget_initposition(&widget->position, x, y);
    widget_initsize(&widget->size, util_clamp(w, minw, maxw), util_clamp(h, minh, maxh));

}

static void placewidget2(struct widget *widget, int x, int y, int w, int h, int minw, int minh, int maxw, int maxh)
{

    placewidget(widget, x, y, w, h, minw, minh, maxw, maxh);

    if (maxw < w)
        widget->size.w = 0;

    if (maxh < h)
        widget->size.h = 0;

}

static void placechild(struct widget *widget, int x, int y, int maxw, int maxh, int paddingw, int paddingh, int stretchw, int stretchh)
{

    struct widget_position cpos;
    struct widget_size cmax;
    struct widget_size cmin;

    widget_initposition(&cpos, x + paddingw, y + paddingh);
    widget_initsize(&cmax, util_max(0, maxw - paddingw * 2), util_max(0, maxh - paddingh * 2));
    widget_initsize(&cmin, (stretchw == LAYOUT_PLACEMENT_STRETCHED) ? cmax.w : 0, (stretchh == LAYOUT_PLACEMENT_STRETCHED) ? cmax.h : 0);
    place_widget(widget, cpos.x, cpos.y, cmin.w, cmin.h, cmax.w, cmax.h);

}

static void updatecache(struct widget *widget, struct widget_cache_placement *placement, struct text_rowinfo *rowinfo, struct text_font *font, unsigned int paddingx, unsigned int paddingy, unsigned int halign, unsigned int valign)
{

    placement->rx = text_getrowx(rowinfo, halign, paddingx, widget->size.w);
    placement->ry = text_getrowy(rowinfo, valign, paddingy, widget->size.h);
    placement->chars = rowinfo->chars;
    placement->font = font;

}

static void placebutton(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_button *button = widget->data;
    struct text_font *font = pool_getfont(POOL_FONTBOLD);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, pool_getstring(button->label), pool_getcstringlength(button->label), TEXT_WRAP_NONE, maxw - CONFIG_BUTTON_PADDING_WIDTH * 2, 0);
    placewidget2(widget, x, y, rowinfo.width + CONFIG_BUTTON_PADDING_WIDTH * 2, rowinfo.lineheight + CONFIG_BUTTON_PADDING_HEIGHT * 2, minw, minh, maxw, maxh);
    updatecache(widget, &button->placement, &rowinfo, font, 0, 0, TEXT_HALIGN_CENTER, TEXT_VALIGN_MIDDLE);

}

static void placechoice(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_choice *choice = widget->data;
    struct text_font *font = pool_getfont(POOL_FONTNORMAL);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, pool_getstring(choice->label), pool_getcstringlength(choice->label), TEXT_WRAP_NONE, maxw - CONFIG_CHOICE_PADDING_WIDTH * 2, 0);
    placewidget(widget, x, y, rowinfo.width + CONFIG_CHOICE_PADDING_WIDTH * 2, rowinfo.lineheight + CONFIG_CHOICE_PADDING_HEIGHT * 2, minw, minh, maxw, maxh);
    updatecache(widget, &choice->placement, &rowinfo, font, CONFIG_CHOICE_PADDING_WIDTH, 0, TEXT_HALIGN_LEFT, TEXT_VALIGN_MIDDLE);

}

static void placelayout(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_layout *layout = widget->data;
    struct list_item *current = 0;
    struct widget_size total;

    widget_initsize(&total, 0, 0);

    switch (layout->type)
    {

    case LAYOUT_TYPE_FLOAT:
        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            placechild(child, x, y, maxw, maxh, layout->padding, layout->padding, LAYOUT_PLACEMENT_NORMAL, LAYOUT_PLACEMENT_NORMAL);
            addtotal(&total, child, x, y, layout->padding, layout->padding);

        }

        break;

    case LAYOUT_TYPE_HORIZONTAL:
        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            placechild(child, x + total.w, y, maxw - total.w, maxh, layout->padding, layout->padding, LAYOUT_PLACEMENT_NORMAL, layout->placement);
            addtotal(&total, child, x, y, layout->padding, layout->padding);

        }

        break;

    case LAYOUT_TYPE_MAXIMIZE:
        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            placechild(child, x, y, maxw, maxh, layout->padding, layout->padding, LAYOUT_PLACEMENT_STRETCHED, LAYOUT_PLACEMENT_STRETCHED);
            addtotal(&total, child, x, y, layout->padding, layout->padding);

        }

        break;

    case LAYOUT_TYPE_VERTICAL:
        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            placechild(child, x, y + total.h, maxw, maxh - total.h, layout->padding, layout->padding, layout->placement, LAYOUT_PLACEMENT_NORMAL);
            addtotal(&total, child, x, y, layout->padding, layout->padding);

        }

        break;

    }

    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

}

static void placefill(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    placewidget(widget, x, y, maxw, maxh, minw, minh, maxw, maxh);

}

static void placegrid(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_grid *grid = widget->data;
    struct list_item *current = 0;
    struct widget_size total;
    struct widget_size row;
    struct widget_size rowtotal;
    unsigned int column = 0;

    widget_initsize(&row, 0, 0);
    widget_initsize(&rowtotal, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        placechild(child, x + row.w, y + rowtotal.h, maxw / grid->columns, maxh - rowtotal.h, grid->padding, grid->padding, grid->placement, GRID_PLACEMENT_NORMAL);
        addtotal(&total, child, x, y, grid->padding, grid->padding);

        row.w += (maxw / grid->columns);

        if (child->size.h)
            row.h = util_max(row.h, child->size.h + grid->padding * 2);

        if (++column % grid->columns == 0)
        {

            widget_initsize(&rowtotal, 0, rowtotal.h + row.h);
            widget_initsize(&row, 0, 0);

        }

    }

    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

}

static void placeimagepcx(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_image *image = widget->data;

    /* This should be done in some preload state after placement but before rendering. Left in placement for now. */
    if (!image->placement.loaded)
    {

        if (file_walk2(FILE_L0, pool_getstring(image->source)))
        {

            struct pcx_header header;

            file_readall(FILE_L0, &header, sizeof (struct pcx_header));
            widget_initsize(&image->placement.size, header.xend - header.xstart + 1, header.yend - header.ystart + 1);

            image->placement.loaded = 1;

        }

    }

    placewidget(widget, x, y, image->placement.size.w, image->placement.size.h, minw, minh, maxw, maxh);

}

static void placeimage(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_image *image = widget->data;

    switch (image->type)
    {

    case IMAGE_TYPE_FUDGEMOUSE:
        break;

    case IMAGE_TYPE_PCX:
        placeimagepcx(widget, x, y, minw, minh, maxw, maxh);

        break;

    }

}

static void placelistbox(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_listbox *listbox = widget->data;
    struct list_item *current = 0;
    struct widget_size total;

    widget_initsize(&total, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        placechild(child, x, y + CONFIG_LISTBOX_PADDING_HEIGHT + total.h, maxw, 50000, CONFIG_LISTBOX_PADDING_WIDTH, 0, LAYOUT_PLACEMENT_STRETCHED, LAYOUT_PLACEMENT_NORMAL);
        addtotal(&total, child, x, y, 0, 0);

    }

    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

    if (listbox->scroll)
    {

        listbox->scroll = util_clamp(listbox->scroll, 0, (total.h > maxh) ? total.h - maxh + CONFIG_LISTBOX_PADDING_HEIGHT * 2 : 0);

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            child->position.y -= listbox->scroll;

        }

    }

}

static void placeselect(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_select *select = widget->data;
    struct text_font *font = pool_getfont(POOL_FONTBOLD);
    unsigned int extra = CONFIG_SELECT_EXTRA + CONFIG_SELECT_PADDING_WIDTH * 2;
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, pool_getstring(select->label), pool_getcstringlength(select->label), TEXT_WRAP_NONE, maxw - CONFIG_SELECT_PADDING_WIDTH * 2, 0);
    placewidget2(widget, x, y, rowinfo.width + CONFIG_SELECT_PADDING_WIDTH * 2 + extra, rowinfo.lineheight + CONFIG_SELECT_PADDING_HEIGHT * 2, minw, minh, maxw, maxh);

    if (widget->state == WIDGET_STATE_FOCUS)
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            placechild(child, widget->position.x, widget->position.y + widget->size.h, widget->size.w, 512, 0, 0, LAYOUT_PLACEMENT_NORMAL, LAYOUT_PLACEMENT_NORMAL);

        }

    }

    else
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            placechild(child, widget->position.x, widget->position.y + widget->size.h, 0, 0, 0, 0, LAYOUT_PLACEMENT_NORMAL, LAYOUT_PLACEMENT_NORMAL);

        }

    }

    updatecache(widget, &select->placement, &rowinfo, font, extra, 0, TEXT_HALIGN_CENTER, TEXT_VALIGN_MIDDLE);

}

static void placetext(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont((text->weight == TEXT_WEIGHT_BOLD) ? POOL_FONTBOLD : POOL_FONTNORMAL);
    struct text_info info;

    text_gettextinfo(&info, font, pool_getstring(text->content), pool_getcstringlength(text->content), text->wrap, maxw, text->placement.firstrowx);
    placewidget(widget, x, y, info.width, info.height, minw, minh, maxw, maxh);

    text->placement.rows = info.rows;
    text->placement.lastrowx = info.lastrowx;
    text->placement.lastrowy = info.lastrowy;
    text->placement.font = font;
    text->rendering.exist = 0;

}

static void placetextbox(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_textbox *textbox = widget->data;
    struct list_item *current = 0;
    struct widget_size total;
    unsigned int lastrowx = 0;
    unsigned int lastrowy = 0;

    widget_initsize(&total, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;

            text->placement.firstrowx = lastrowx;

            placechild(child, x, y + lastrowy, maxw, 50000, CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_TEXTBOX_PADDING_HEIGHT, LAYOUT_PLACEMENT_STRETCHED, LAYOUT_PLACEMENT_NORMAL);
            addtotal(&total, child, x, y, CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_TEXTBOX_PADDING_HEIGHT);

            lastrowx = text->placement.lastrowx;
            lastrowy += text->placement.lastrowy;

        }

    }

    placewidget(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

    if (textbox->scroll)
    {

        textbox->scroll = util_clamp(textbox->scroll, 0, total.h - 56);

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            child->position.y -= textbox->scroll;

        }

    }

}

static void placetextbutton(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_textbutton *textbutton = widget->data;
    struct text_font *font = pool_getfont(POOL_FONTNORMAL);
    struct text_rowinfo rowinfo;

    text_getrowinfo(&rowinfo, font, pool_getstring(textbutton->label), pool_getcstringlength(textbutton->label), TEXT_WRAP_NONE, maxw - CONFIG_TEXTBUTTON_PADDING_WIDTH * 2, 0);
    placewidget2(widget, x, y, rowinfo.width + CONFIG_TEXTBUTTON_PADDING_WIDTH * 2, rowinfo.lineheight + CONFIG_TEXTBUTTON_PADDING_HEIGHT * 2, minw, minh, maxw, maxh);
    updatecache(widget, &textbutton->placement, &rowinfo, font, CONFIG_TEXTBUTTON_PADDING_WIDTH, 0, TEXT_HALIGN_LEFT, TEXT_VALIGN_MIDDLE);

}

static void placewindow(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        placechild(child, widget->position.x, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->size.w, widget->size.h - CONFIG_WINDOW_BUTTON_HEIGHT, CONFIG_WINDOW_BORDER_WIDTH, CONFIG_WINDOW_BORDER_HEIGHT, LAYOUT_PLACEMENT_NORMAL, LAYOUT_PLACEMENT_NORMAL);

    }

}

void place_widget(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        placebutton(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_CHOICE:
        placechoice(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_LAYOUT:
        placelayout(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_FILL:
        placefill(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_GRID:
        placegrid(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_IMAGE:
        placeimage(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_LISTBOX:
        placelistbox(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_SELECT:
        placeselect(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_TEXT:
        placetext(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_TEXTBOX:
        placetextbox(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        placetextbutton(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_WINDOW:
        placewindow(widget, x, y, minw, minh, maxw, maxh);

        break;

    }

}

