#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "widget.h"
#include "pool.h"
#include "blit.h"
#include "place.h"
#include "render.h"

static void resize(struct widget *widget, int x, int y, int w, int h, int minw, int minh, int maxw, int maxh)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(w, minw, maxw);
    widget->size.h = util_clamp(h, minh, maxh);

}

static void resize2(struct widget *widget, int x, int y, int w, int h, int minw, int minh, int maxw, int maxh)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(w, minw, maxw);
    widget->size.h = util_clamp(h, minh, maxh);

    if (maxw < w)
        widget->size.w = 0;

    if (maxh < h)
        widget->size.h = 0;

}

static void placebutton(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_button *button = widget->data;

    text_getrowinfo(pool_getfont(POOL_FONTBOLD), pool_getstring(button->label), pool_getcstringlength(button->label), &button->labelinfo, WIDGET_TEXT_WRAP_NONE, 0, 0);
    resize2(widget, x, y, button->labelinfo.width + CONFIG_BUTTON_PADDING_WIDTH * 2, button->labelinfo.lineheight + CONFIG_BUTTON_PADDING_HEIGHT * 2, minw, minh, maxw, maxh);

}

static void placechoice(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_choice *choice = widget->data;

    text_getrowinfo(pool_getfont(POOL_FONTNORMAL), pool_getstring(choice->label), pool_getcstringlength(choice->label), &choice->labelinfo, WIDGET_TEXT_WRAP_NONE, 0, 0);
    resize(widget, x, y, choice->labelinfo.width + CONFIG_CHOICE_PADDING_WIDTH * 2, choice->labelinfo.height + CONFIG_CHOICE_PADDING_HEIGHT * 2, minw, minh, maxw, maxh);

}

static void placecontainerfloat(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        place_widget(child, x, y, 0, 0, maxw, maxh);

    }

    resize(widget, x, y, 0, 0, minw, minh, maxw, maxh);

}

static void placecontainerhorizontal(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;
    int offx = container->padding;
    int offy = container->padding;
    int offw = container->padding * 2;
    int offh = container->padding * 2;
    int toth = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + offx; 
        int childy = y + offy; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = maxw - offw;
        int childmaxh = maxh - offh;

        if (container->placement == WIDGET_CONTAINER_PLACEMENT_STRETCHED)
            childminh = childmaxh;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        offx += child->size.w + container->padding;
        offw += child->size.w + container->padding;
        toth = util_max(toth, child->size.h);

    }

    resize(widget, x, y, offw, toth, minw, minh, maxw, maxh);

}

static void placecontainermaximize(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;
    int offx = container->padding;
    int offy = container->padding;
    int offw = container->padding * 2;
    int offh = container->padding * 2;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + offx; 
        int childy = y + offy; 
        int childmaxw = maxw - offw;
        int childmaxh = maxh - offh;

        place_widget(child, childx, childy, childmaxw, childmaxh, childmaxw, childmaxh);

    }

    resize(widget, x, y, maxw, maxh, minw, minh, maxw, maxh);

}

static void placecontainervertical(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;
    int offx = container->padding;
    int offy = container->padding;
    int offw = container->padding * 2;
    int offh = container->padding * 2;
    int totw = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + offx; 
        int childy = y + offy; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = maxw - offw;
        int childmaxh = maxh - offh;

        if (container->placement == WIDGET_CONTAINER_PLACEMENT_STRETCHED)
            childminw = childmaxw;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        offy += child->size.h + container->padding;
        offh += child->size.h + container->padding;
        totw = util_max(totw, child->size.w);

    }

    resize(widget, x, y, totw, offh, minw, minh, maxw, maxh);

}

static void placecontainer(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_container *container = widget->data;

    switch (container->layout)
    {

    case WIDGET_CONTAINER_LAYOUT_FLOAT:
        placecontainerfloat(widget, x, y, minw, minh, maxw, maxh);
        
        break;

    case WIDGET_CONTAINER_LAYOUT_HORIZONTAL:
        placecontainerhorizontal(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_CONTAINER_LAYOUT_MAXIMIZE:
        placecontainermaximize(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_CONTAINER_LAYOUT_VERTICAL:
        placecontainervertical(widget, x, y, minw, minh, maxw, maxh);

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
    int offx = grid->padding;
    int offy = grid->padding;
    int offw = grid->padding * 2;
    int offh = grid->padding * 2;
    int roww = 0;
    int rowh = 0;
    int totw = 0;
    int toth = 0;
    unsigned int numchildren = 0;
    unsigned int colw = (maxw - grid->padding * (grid->columns + 1)) / grid->columns;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + offx; 
        int childy = y + offy; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = maxw - offw;
        int childmaxh = maxh - offh;

        if (grid->placement == WIDGET_GRID_PLACEMENT_STRETCHED)
        {

            childminw = colw;
            childmaxw = colw;

        }

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        offx += colw + grid->padding;
        offw += colw + grid->padding;
        rowh = util_max(rowh, child->size.h);
        toth = util_max(toth, rowh);

        numchildren++;

        if (numchildren % grid->columns == 0)
        {

            /* roww is strange here */
            totw = util_max(totw, roww);
            offy += rowh + grid->padding;
            offh += rowh + grid->padding;
            offx = grid->padding;
            offw = grid->padding * 2;
            roww = 0;
            rowh = 0;

        }

    }

    resize(widget, x, y, totw, toth, minw, minh, maxw, maxh);

}

static void placeimagepcx(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_image *image = widget->data;
    unsigned int w = 0;
    unsigned int h = 0;

    if (file_walk2(FILE_L0, pool_getstring(image->source)))
    {

        struct pcx_header header;

        file_readall(FILE_L0, &header, sizeof (struct pcx_header));

        w = header.xend - header.xstart + 1;
        h = header.yend - header.ystart + 1;

    }

    resize(widget, x, y, w, h, minw, minh, maxw, maxh);

}

static void placeimage(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_image *image = widget->data;

    switch (image->type)
    {

    case WIDGET_IMAGE_TYPE_FUDGEMOUSE:
        break;

    case WIDGET_IMAGE_TYPE_PCX:
        placeimagepcx(widget, x, y, minw, minh, maxw, maxh);

        break;

    }

}

static void placeselect(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_select *select = widget->data;
    unsigned int extra = 16 + CONFIG_SELECT_PADDING_WIDTH * 2;

    text_getrowinfo(pool_getfont(POOL_FONTNORMAL), pool_getstring(select->label), pool_getcstringlength(select->label), &select->labelinfo, WIDGET_TEXT_WRAP_NONE, 0, 0);
    resize2(widget, x, y, select->labelinfo.width + CONFIG_SELECT_PADDING_WIDTH * 2 + extra, select->labelinfo.lineheight + CONFIG_SELECT_PADDING_HEIGHT * 2, minw, minh, maxw, maxh);

    if (widget->state == WIDGET_STATE_FOCUS)
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;
            int childx = widget->position.x; 
            int childy = widget->position.y + widget->size.h; 
            int childminw = widget->size.w;
            int childminh = 0;
            int childmaxw = widget->size.w;
            int childmaxh = 512;

            place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        }

    }

    else
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;
            int childx = widget->position.x; 
            int childy = widget->position.y + widget->size.h; 
            int childminw = 0;
            int childminh = 0;
            int childmaxw = 0;
            int childmaxh = 0;

            place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        }

    }

}

static void placetext(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_text *text = widget->data;
    unsigned int index = (text->weight == WIDGET_TEXT_WEIGHT_BOLD) ? POOL_FONTBOLD : POOL_FONTNORMAL;

    text->rownum = 0;
    text->rowstart = text_getrowstart(pool_getfont(index), pool_getstring(text->content), pool_getcstringlength(text->content), text->rownum, text->wrap, maxw);

    text_gettextinfo(pool_getfont(index), pool_getstring(text->content), pool_getcstringlength(text->content), &text->textinfo, text->wrap, text->firstrowoffset, maxw);
    resize(widget, x, y, text->textinfo.width + 1, text->textinfo.rows * text->textinfo.lineheight, minw, minh, maxw, maxh);

}

static void placetextbox(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_textbox *textbox = widget->data;
    struct list_item *current = 0;
    int offx = CONFIG_TEXTBOX_PADDING_WIDTH;
    int offy = CONFIG_TEXTBOX_PADDING_HEIGHT;
    int offw = CONFIG_TEXTBOX_PADDING_WIDTH * 2;
    int offh = CONFIG_TEXTBOX_PADDING_HEIGHT * 2;
    int soffy = offy;
    int soffh = offh;
    int totw = 0;
    unsigned int lasttype = 0;
    struct render_textinfo textinfo;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + offx; 
        int childy = y + offy; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = maxw - offw;
        int childmaxh = 5000;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;
            unsigned int index = (text->weight == WIDGET_TEXT_WEIGHT_BOLD) ? POOL_FONTBOLD : POOL_FONTNORMAL;

            text->firstrowoffset = 0;

            if (lasttype == WIDGET_TYPE_TEXT)
            {

                if (textinfo.last.newline)
                {

                    soffy += textinfo.rows * textinfo.lineheight;
                    soffh += textinfo.rows * textinfo.lineheight;

                }

                else
                {

                    text->firstrowoffset = textinfo.last.width;
                    soffy += (textinfo.rows - 1) * textinfo.lineheight;
                    soffh += (textinfo.rows - 1) * textinfo.lineheight;

                }

            }

            childy = y + soffy; 

            text_gettextinfo(pool_getfont(index), pool_getstring(text->content), pool_getcstringlength(text->content), &textinfo, text->wrap, text->firstrowoffset, childmaxw);

            place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        }

        else
        {

            place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        }

        lasttype = child->type;
        offy += child->size.h + CONFIG_TEXTBOX_PADDING_HEIGHT;
        offh += child->size.h + CONFIG_TEXTBOX_PADDING_HEIGHT;
        totw = util_max(totw, child->size.w);

    }

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        textbox->scroll = util_clamp(textbox->scroll, -100, 100);

        child->position.y -= textbox->scroll;

    }

    resize(widget, x, y, totw, offy, minw, minh, maxw, maxh);

}

static void placewindow(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct list_item *current = 0;
    int offx = CONFIG_WINDOW_BORDER_WIDTH;
    int offy = CONFIG_WINDOW_BORDER_HEIGHT + CONFIG_ICON_HEIGHT;
    int offw = CONFIG_WINDOW_BORDER_WIDTH * 2;
    int offh = CONFIG_WINDOW_BORDER_HEIGHT * 2 + CONFIG_ICON_HEIGHT;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = widget->position.x + offx;
        int childy = widget->position.y + offy;
        int childminw = 0;
        int childminh = 0;
        int childmaxw = widget->size.w - offw;
        int childmaxh = widget->size.h - offh;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

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

    case WIDGET_TYPE_CONTAINER:
        placecontainer(widget, x, y, minw, minh, maxw, maxh);

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

    case WIDGET_TYPE_WINDOW:
        placewindow(widget, x, y, minw, minh, maxw, maxh);

        break;

    }

}

