#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "widget.h"
#include "pool.h"
#include "place.h"

static void addtotal(struct widget_size *total, struct widget *widget, int x, int y, int pw, int ph)
{

    total->w = util_max(total->w, ((widget->position.x + widget->size.w) - x) + pw);
    total->h = util_max(total->h, ((widget->position.y + widget->size.h) - y) + ph);

}

static void resize(struct widget *widget, int x, int y, int w, int h, int minw, int minh, int maxw, int maxh)
{

    widget_initposition(&widget->position, x, y);
    widget_initsize(&widget->size, util_clamp(w, minw, maxw), util_clamp(h, minh, maxh));

}

static void resize2(struct widget *widget, int x, int y, int w, int h, int minw, int minh, int maxw, int maxh)
{

    resize(widget, x, y, w, h, minw, minh, maxw, maxh);

    if (maxw < w)
        widget->size.w = 0;

    if (maxh < h)
        widget->size.h = 0;

}

static void placebutton(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_button *button = widget->data;
    struct widget_cache *cache = &button->cache;
    struct text_font *font = pool_getfont(POOL_FONTBOLD);
    struct widget_size total;

    text_getrowinfo(&cache->textrow.info, font, pool_getstring(button->label), pool_getcstringlength(button->label), TEXT_WRAP_NONE, maxw - CONFIG_BUTTON_PADDING_WIDTH * 2, 0);
    widget_initsize(&total, cache->textrow.info.width + CONFIG_BUTTON_PADDING_WIDTH * 2, cache->textrow.info.lineheight + CONFIG_BUTTON_PADDING_HEIGHT * 2);
    resize2(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

}

static void placechoice(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_choice *choice = widget->data;
    struct widget_cache *cache = &choice->cache;
    struct text_font *font = pool_getfont(POOL_FONTNORMAL);
    struct widget_size total;

    text_getrowinfo(&cache->textrow.info, font, pool_getstring(choice->label), pool_getcstringlength(choice->label), TEXT_WRAP_NONE, maxw - CONFIG_CHOICE_PADDING_WIDTH * 2, 0);
    widget_initsize(&total, cache->textrow.info.width + CONFIG_CHOICE_PADDING_WIDTH * 2, cache->textrow.info.lineheight + CONFIG_CHOICE_PADDING_HEIGHT * 2);
    resize(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

}

static void placelayoutfloat(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        place_widget(child, x, y, 0, 0, maxw, maxh);

    }

    resize(widget, x, y, 0, 0, minw, minh, maxw, maxh);

}

static void placelayouthorizontal(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_layout *layout = widget->data;
    struct list_item *current = 0;
    struct widget_size total;

    widget_initsize(&total, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct widget_position cpos;
        struct widget_size cmax;
        struct widget_size cmin;

        widget_initposition(&cpos, x + layout->padding + total.w, y + layout->padding);
        widget_initsize(&cmax, util_max(0, maxw - total.w - layout->padding * 2), util_max(0, maxh - layout->padding * 2));
        widget_initsize(&cmin, 0, (layout->placement == LAYOUT_PLACEMENT_STRETCHED) ? cmax.h : 0);
        place_widget(child, cpos.x, cpos.y, cmin.w, cmin.h, cmax.w, cmax.h);
        addtotal(&total, child, x, y, layout->padding, layout->padding);

    }

    resize(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

}

static void placelayoutmaximize(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_layout *layout = widget->data;
    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct widget_position cpos;
        struct widget_size cmax;

        widget_initposition(&cpos, x + layout->padding, y + layout->padding);
        widget_initsize(&cmax, util_max(0, maxw - layout->padding * 2), util_max(0, maxh - layout->padding * 2));
        place_widget(child, cpos.x, cpos.y, cmax.w, cmax.h, cmax.w, cmax.h);

    }

    resize(widget, x, y, maxw, maxh, minw, minh, maxw, maxh);

}

static void placelayoutvertical(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_layout *layout = widget->data;
    struct list_item *current = 0;
    struct widget_size total;

    widget_initsize(&total, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct widget_position cpos;
        struct widget_size cmax;
        struct widget_size cmin;

        widget_initposition(&cpos, x + layout->padding, y + layout->padding + total.h);
        widget_initsize(&cmax, util_max(0, maxw - layout->padding * 2), util_max(0, maxh - total.h - layout->padding * 2));
        widget_initsize(&cmin, (layout->placement == LAYOUT_PLACEMENT_STRETCHED) ? cmax.w : 0, 0);
        place_widget(child, cpos.x, cpos.y, cmin.w, cmin.h, cmax.w, cmax.h);
        addtotal(&total, child, x, y, layout->padding, layout->padding);

    }

    resize(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

}

static void placelayout(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_layout *layout = widget->data;

    switch (layout->type)
    {

    case LAYOUT_TYPE_FLOAT:
        placelayoutfloat(widget, x, y, minw, minh, maxw, maxh);

        break;

    case LAYOUT_TYPE_HORIZONTAL:
        placelayouthorizontal(widget, x, y, minw, minh, maxw, maxh);

        break;

    case LAYOUT_TYPE_MAXIMIZE:
        placelayoutmaximize(widget, x, y, minw, minh, maxw, maxh);

        break;

    case LAYOUT_TYPE_VERTICAL:
        placelayoutvertical(widget, x, y, minw, minh, maxw, maxh);

        break;

    }

}

static void placefill(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    resize(widget, x, y, maxw, maxh, minw, minh, maxw, maxh);

}

static void placegrid(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_grid *grid = widget->data;
    struct list_item *current = 0;
    struct widget_size row;
    struct widget_size total;
    unsigned int column = 0;

    widget_initsize(&row, 0, 0);
    widget_initsize(&total, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct widget_position cpos;
        struct widget_size cmax;
        struct widget_size cmin;

        widget_initposition(&cpos, x + grid->padding + row.w, y + grid->padding + total.h);
        widget_initsize(&cmax, util_max(0, (maxw / grid->columns) - grid->padding * 2), util_max(0, maxh - total.h - grid->padding * 2));
        widget_initsize(&cmin, (grid->placement == GRID_PLACEMENT_STRETCHED) ? cmax.w : 0, 0);
        place_widget(child, cpos.x, cpos.y, cmin.w, cmin.h, cmax.w, cmax.h);

        row.w += (maxw / grid->columns);

        if (child->size.h)
            row.h = util_max(row.h, child->size.h + grid->padding * 2);

        if (++column % grid->columns == 0)
        {

            widget_initsize(&total, total.w + util_max(total.w, row.w), total.h + row.h);
            widget_initsize(&row, 0, 0);

        }

    }

    widget_initsize(&total, total.w + util_max(total.w, row.w), total.h + row.h);
    resize(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

}

static void placeimagepcx(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_image *image = widget->data;
    struct widget_size total;

    widget_initsize(&total, 0, 0);

    if (file_walk2(FILE_L0, pool_getstring(image->source)))
    {

        struct pcx_header header;

        file_readall(FILE_L0, &header, sizeof (struct pcx_header));
        widget_initsize(&total, header.xend - header.xstart + 1, header.yend - header.ystart + 1);

    }

    resize(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

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

static void placeselect(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_select *select = widget->data;
    struct widget_cache *cache = &select->cache;
    struct text_font *font = pool_getfont(POOL_FONTNORMAL);
    unsigned int extra = 16 + CONFIG_SELECT_PADDING_WIDTH * 2;
    struct widget_size total;

    text_getrowinfo(&cache->textrow.info, font, pool_getstring(select->label), pool_getcstringlength(select->label), TEXT_WRAP_NONE, maxw - CONFIG_SELECT_PADDING_WIDTH * 2, 0);
    widget_initsize(&total, cache->textrow.info.width + CONFIG_SELECT_PADDING_WIDTH * 2 + extra, cache->textrow.info.lineheight + CONFIG_SELECT_PADDING_HEIGHT * 2);
    resize2(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

    if (widget->state == WIDGET_STATE_FOCUS)
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;
            struct widget_position cpos;
            struct widget_size cmax;
            struct widget_size cmin;

            widget_initposition(&cpos, widget->position.x, widget->position.y + widget->size.h);
            widget_initsize(&cmax, widget->size.w, 512);
            widget_initsize(&cmin, widget->size.w, 0);
            place_widget(child, cpos.x, cpos.y, cmin.w, cmin.h, cmax.w, cmax.h);

        }

    }

    else
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;
            struct widget_position cpos;

            widget_initposition(&cpos, widget->position.x, widget->position.y + widget->size.h);
            place_widget(child, cpos.x, cpos.y, 0, 0, 0, 0);

        }

    }

}

static void placetext(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_text *text = widget->data;
    struct widget_cache *cache = &text->cache;
    struct text_font *font = pool_getfont((text->weight == TEXT_WEIGHT_BOLD) ? POOL_FONTBOLD : POOL_FONTNORMAL);

    cache->exist = 0;

    text_gettextinfo(&text->textinfo, font, pool_getstring(text->content), pool_getcstringlength(text->content), text->wrap, maxw, text->firstrowx);
    resize(widget, x, y, text->textinfo.width, text->textinfo.height, minw, minh, maxw, maxh);

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
        struct widget_position cpos;
        struct widget_size cmax;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;

            text->firstrowx = lastrowx;

            widget_initposition(&cpos, x + CONFIG_TEXTBOX_PADDING_WIDTH, y + CONFIG_TEXTBOX_PADDING_HEIGHT + lastrowy);
            widget_initsize(&cmax, util_max(0, maxw - CONFIG_TEXTBOX_PADDING_WIDTH * 2), 50000);
            place_widget(child, cpos.x, cpos.y, cmax.w, 0, cmax.w, cmax.h);
            addtotal(&total, child, x, y, CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_TEXTBOX_PADDING_HEIGHT);

            lastrowx = text->textinfo.lastrowx;
            lastrowy += text->textinfo.lastrowy;

        }

        else
        {

            widget_initposition(&cpos, x + 4, y + 4 + total.h);
            widget_initsize(&cmax, util_max(0, maxw - 4 * 2), 50000);
            place_widget(child, cpos.x, cpos.y, cmax.w, 0, cmax.w, cmax.h);
            addtotal(&total, child, x, y, 4, 0);

        }

    }

    resize(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

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
    struct widget_cache *cache = &textbutton->cache;
    struct text_font *font = pool_getfont(POOL_FONTNORMAL);
    struct widget_size total;

    text_getrowinfo(&cache->textrow.info, font, pool_getstring(textbutton->label), pool_getcstringlength(textbutton->label), TEXT_WRAP_NONE, maxw - 16 * 2, 0);
    widget_initsize(&total, cache->textrow.info.width + 16 * 2, cache->textrow.info.lineheight + 8 * 2);
    resize2(widget, x, y, total.w, total.h, minw, minh, maxw, maxh);

}

static void placewindow(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct widget_position cpos;
        struct widget_size cmax;

        widget_initposition(&cpos, widget->position.x + CONFIG_WINDOW_BORDER_WIDTH, widget->position.y + CONFIG_WINDOW_BORDER_HEIGHT + CONFIG_WINDOW_BUTTON_HEIGHT);
        widget_initsize(&cmax, util_max(0, widget->size.w - CONFIG_WINDOW_BORDER_WIDTH * 2), util_max(0, widget->size.h - CONFIG_WINDOW_BORDER_HEIGHT * 2 - CONFIG_WINDOW_BUTTON_HEIGHT));
        place_widget(child, cpos.x, cpos.y, 0, 0, cmax.w, cmax.h);

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

