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

#define INC_NONE                0
#define INC_NORMAL              1
#define INC_STRETCHED           2
#define INFINITY                50000

struct
{

    unsigned int hasdamage;
    struct util_position position0;
    struct util_position position2;

} area;

struct calls
{

    struct util_size (*place)(struct widget *widget, struct util_region *region, struct util_size *min);
    void (*render)(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my);

};

static struct calls calls[32];
static struct util_size zerosize;

static struct util_position posshrink(struct util_position *pos, struct util_size *padding)
{

    struct util_position p;

    p.x = pos->x + padding->w;
    p.y = pos->y + padding->h;

    return p;

}

static struct util_size sizeshrink(struct util_size *size, struct util_size *padding)
{

    struct util_size n;

    n.w = util_clamp(size->w, 0, size->w - padding->w * 2);
    n.h = util_clamp(size->h, 0, size->h - padding->h * 2);

    return n;

}

static struct util_region regionshrink(struct util_region *region, struct util_size *padding)
{

    struct util_region r;

    r.position = posshrink(&region->position, padding);
    r.size = sizeshrink(&region->size, padding);

    return r;

}

static unsigned int getnumspans(struct widget *widget)
{

    struct list_item *current = 0;
    unsigned int spans = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->span)
            spans += child->span;

    }

    return spans;

}

static struct util_size placewidget(struct widget *widget, struct util_region *region, struct util_region *wregion, struct util_size *min)
{

    widget->region = util_region(wregion->position.x, wregion->position.y, util_clamp(wregion->size.w, min->w, region->size.w), util_clamp(wregion->size.h, min->h, region->size.h));
    widget->clip = widget->region;

    return util_size(widget->region.size.w, widget->region.size.h);

}

static struct util_size placechild(struct widget *widget, struct util_region *region, struct util_size *min, struct util_size *padding)
{

    struct util_region cregion = regionshrink(region, padding);
    struct util_size cmin = sizeshrink(min, padding);
    struct util_size wsize = calls[widget->type].place(widget, &cregion, &cmin);

    return util_size(wsize.w + padding->w * 2, wsize.h + padding->h * 2);

}

static struct util_size placeblockchildren1(struct widget *widget, struct util_region *region, struct util_size *padding, unsigned int incx, unsigned int incy, struct util_size *span)
{

    struct list_item *current = 0;
    struct util_size total = zerosize;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_region cregion = util_region(region->position.x, region->position.y, region->size.w, region->size.h);
        struct util_size cmin = zerosize;
        struct util_size csize;

        if (incx)
        {

            cregion.position.x += total.w;

            switch (incx)
            {

            case INC_STRETCHED:
                cmin.h = cregion.size.h;

                break;

            }

            if (child->span)
            {

                cmin.w = span->w * child->span;
                cregion.size.w = span->w * child->span;

            }

        }

        if (incy)
        {

            cregion.position.y += total.h;

            switch (incy)
            {

            case INC_STRETCHED:
                cmin.w = cregion.size.w;

                break;

            }

            if (child->span)
            {

                cmin.h = span->h * child->span;
                cregion.size.h = span->h * child->span;

            }

        }

        csize = placechild(child, &cregion, &cmin, padding);
        total.w = util_max(total.w, (cregion.position.x + csize.w) - region->position.x);
        total.h = util_max(total.h, (cregion.position.y + csize.h) - region->position.y);

    }

    return total;

}

static struct util_size placeblockchildren(struct widget *widget, struct util_region *region, struct util_size *padding, unsigned int incx, unsigned int incy)
{

    unsigned int spans = getnumspans(widget);
    struct util_size total = placeblockchildren1(widget, region, padding, incx, incy, &zerosize);

    if (spans)
    {

        struct util_size span = util_size((region->size.w - total.w) / spans, (region->size.h - total.h) / spans);

        return placeblockchildren1(widget, region, padding, incx, incy, &span);

    }

    return total;

}

static struct util_size placeinlinechildren(struct widget *widget, struct util_region *region, struct util_size *padding)
{

    struct util_size total = zerosize;
    struct list_item *current = 0;
    unsigned int offx = 0;
    unsigned int offy = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_region cregion = util_region(region->position.x, region->position.y + offy, region->size.w, region->size.h);
        struct util_size csize;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;

            text->offx = offx;
            csize = placechild(child, &cregion, &zerosize, padding);
            total.w = util_max(total.w, (cregion.position.x + csize.w) - region->position.x);
            total.h = util_max(total.h, (cregion.position.y + csize.h) - region->position.y);

            offx = text->lastrowx;
            offy += text->lastrowy;

        }

    }

    return total;

}

static void clipchildren1(struct widget *widget, struct util_region *clip)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        child->clip = util_region(clip->position.x, clip->position.y, clip->size.w, clip->size.h);

        clipchildren1(child, clip);

    }

}

static void clipchildren(struct widget *widget, struct util_size *padding)
{

    struct util_region clip = regionshrink(&widget->region, padding);

    clipchildren1(widget, &clip);

}

static void scrollchildren(struct widget *widget, int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        child->region.position.x += x;
        child->region.position.y += y;

        scrollchildren(child, x, y);

    }

}

static struct util_size gettext(struct widget *widget, unsigned int weight, unsigned int string, unsigned int wrap, unsigned int maxw, unsigned int offset)
{

    struct text_font *font = pool_getfont(weight);
    struct text_info info;

    text_gettextinfo(&info, font, strpool_getstring(string), strpool_getcstringlength(string), wrap, maxw, offset);

    if (widget->type == WIDGET_TYPE_TEXT)
    {

        struct widget_text *text = widget->data;

        text->lastrowx = info.lastrowx;
        text->lastrowy = info.lastrowy;

    }

    return util_size(info.width, info.height);

}

static struct util_size placebutton(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct widget_button *button = widget->data;
    struct util_size tsize = gettext(widget, ATTR_WEIGHT_BOLD, button->label, ATTR_WRAP_NONE, 0, 0);
    struct util_size padding = util_size(CONFIG_BUTTON_PADDING_WIDTH, CONFIG_BUTTON_PADDING_HEIGHT);
    struct util_region wregion = util_region(region->position.x, region->position.y, tsize.w + padding.w * 2, tsize.h + padding.h * 2);

    return placewidget(widget, region, &wregion, min);

}

static struct util_size placechoice(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct widget_choice *choice = widget->data;
    struct util_size tsize = gettext(widget, ATTR_WEIGHT_NORMAL, choice->label, ATTR_WRAP_NONE, 0, 0);
    struct util_size padding = util_size(CONFIG_CHOICE_PADDING_WIDTH, CONFIG_CHOICE_PADDING_HEIGHT);
    struct util_region wregion = util_region(region->position.x, region->position.y, tsize.w + padding.w * 2, tsize.h + padding.h * 2);

    return placewidget(widget, region, &wregion, min);

}

static struct util_size placefill(struct widget *widget, struct util_region *region, struct util_size *min)
{

    return placewidget(widget, region, region, min);

}

static struct util_size placeimage(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct widget_image *image = widget->data;
    struct util_region wregion = util_region(region->position.x, region->position.y, image->size.w, image->size.h);

    if (image->mimetype == ATTR_MIMETYPE_FUDGEMOUSE)
        return placewidget(widget, region, &widget->region, min);

    return placewidget(widget, region, &wregion, min);

}

static struct util_size placelayout(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct widget_layout *layout = widget->data;
    struct util_size cpadding = util_size(layout->padding * CONFIG_LAYOUT_PADDING_WIDTH, layout->padding * CONFIG_LAYOUT_PADDING_HEIGHT);
    struct util_region cregion = util_region(region->position.x, region->position.y, region->size.w, region->size.h);
    struct util_size csize;
    struct util_region wregion;

    switch (layout->flow)
    {

    case ATTR_FLOW_DEFAULT:
        csize = placeblockchildren(widget, &cregion, &cpadding, INC_NONE, INC_NONE);

        break;

    case ATTR_FLOW_HORIZONTAL:
        csize = placeblockchildren(widget, &cregion, &cpadding, INC_NORMAL, INC_NONE);

        break;

    case ATTR_FLOW_HORIZONTALSTRETCH:
        csize = placeblockchildren(widget, &cregion, &cpadding, INC_STRETCHED, INC_NONE);

        break;

    case ATTR_FLOW_VERTICAL:
        csize = placeblockchildren(widget, &cregion, &cpadding, INC_NONE, INC_NORMAL);

        break;

    case ATTR_FLOW_VERTICALSTRETCH:
        csize = placeblockchildren(widget, &cregion, &cpadding, INC_NONE, INC_STRETCHED);

        break;

    }

    wregion = util_region(region->position.x, region->position.y, csize.w, csize.h);

    return placewidget(widget, region, &wregion, min);

}

static struct util_size placelistbox(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct widget_listbox *listbox = widget->data;
    struct util_size padding = util_size(CONFIG_FRAME_WIDTH, CONFIG_FRAME_HEIGHT);
    struct util_region cregion = util_region(region->position.x + padding.w, region->position.y + padding.h, region->size.w - padding.w * 2, INFINITY);
    struct util_size csize = placeblockchildren(widget, &cregion, &zerosize, INC_NONE, INC_NORMAL);
    struct util_region wregion = util_region(region->position.x, region->position.y, csize.w + padding.w * 2, csize.h + padding.h * 2);
    struct util_size wsize = placewidget(widget, region, &wregion, min);

    clipchildren(widget, &padding);

    if (csize.h >= wsize.h - padding.h * 2)
    {

        if (listbox->vscroll < 0)
            listbox->vscroll = 0;

        if (listbox->vscroll > csize.h - wsize.h + padding.h * 2)
            listbox->vscroll = csize.h - wsize.h + padding.h * 2;

        scrollchildren(widget, 0, -listbox->vscroll);

    }

    return wsize;

}

static struct util_size placeselect(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct widget_select *select = widget->data;
    struct util_size tsize = gettext(widget, ATTR_WEIGHT_NORMAL, select->label, ATTR_WRAP_NONE, 0, 0);
    struct util_size padding = util_size(CONFIG_SELECT_PADDING_WIDTH, CONFIG_SELECT_PADDING_HEIGHT);
    struct util_region wregion = util_region(region->position.x, region->position.y, tsize.w + padding.w * 4, tsize.h + padding.h * 2);
    struct util_region cregion = util_region(region->position.x, region->position.y + wregion.size.h, wregion.size.w * 2, INFINITY);
    struct util_size wsize = placewidget(widget, region, &wregion, min);

    placeblockchildren(widget, &cregion, &zerosize, INC_NONE, INC_NORMAL);

    if (widget->state != WIDGET_STATE_FOCUS)
        clipchildren(widget, &zerosize);

    return wsize;

}

static struct util_size placetext(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct widget_text *text = widget->data;
    struct util_size tsize = gettext(widget, text->weight, text->content, text->wrap, region->size.w, text->offx);
    struct util_region wregion = util_region(region->position.x, region->position.y, tsize.w, tsize.h);

    return placewidget(widget, region, &wregion, min);

}

static struct util_size placetextbox(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct widget_textbox *textbox = widget->data;
    struct util_size padding = util_size(CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_TEXTBOX_PADDING_HEIGHT);
    struct util_region cregion = util_region(region->position.x + padding.w, region->position.y + padding.h, region->size.w - padding.w * 2, INFINITY);
    struct util_size csize = placeinlinechildren(widget, &cregion, &zerosize);
    struct util_region wregion = util_region(region->position.x, region->position.y, csize.w + padding.w * 2, csize.h + padding.h * 2);
    struct util_size wsize = placewidget(widget, region, &wregion, min);
    struct list_item *current = 0;

    clipchildren(widget, &padding);

    if (csize.h >= wsize.h - padding.h * 2)
    {

        if (textbox->vscroll < 0)
            textbox->vscroll = 0;

        if (textbox->vscroll > csize.h - wsize.h + padding.h * 2)
            textbox->vscroll = csize.h - wsize.h + padding.h * 2;

        scrollchildren(widget, 0, -textbox->vscroll);

    }

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;
            struct text_font *font = pool_getfont(text->weight);

            textbox->cursorheight = font->lineheight - font->lineheight / 4;
            textbox->cursorwidth = 2;
            textbox->cursorx = child->region.position.x + text->lastrowx;
            textbox->cursory = child->region.position.y + text->lastrowy + font->lineheight / 2 - textbox->cursorheight / 2;

        }

    }

    return wsize;

}

static struct util_size placetextbutton(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct widget_textbutton *textbutton = widget->data;
    struct util_size tsize = gettext(widget, ATTR_WEIGHT_NORMAL, textbutton->label, ATTR_WRAP_NONE, 0, 0);
    struct util_region wregion = util_region(region->position.x, region->position.y, tsize.w + CONFIG_TEXTBUTTON_PADDING_WIDTH * 2, tsize.h + CONFIG_TEXTBUTTON_PADDING_HEIGHT * 2);

    return placewidget(widget, region, &wregion, min);

}

static struct util_size placewindow(struct widget *widget, struct util_region *region, struct util_size *min)
{

    struct util_size cpadding = util_size(CONFIG_WINDOW_BORDER_WIDTH, CONFIG_WINDOW_BORDER_HEIGHT);
    struct util_region cregion = util_region(widget->region.position.x, widget->region.position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->region.size.w, widget->region.size.h - CONFIG_WINDOW_BUTTON_HEIGHT);

    placeblockchildren(widget, &cregion, &cpadding, INC_NONE, INC_NORMAL);

    return placewidget(widget, region, &widget->region, min);

}

static void _renderx(struct blit_display *display, struct util_region *region, unsigned int text, unsigned int *cmap, int x0, int x2, int offx, int offy, int markstart, int markend, unsigned int halign, unsigned int valign, unsigned int weight, unsigned int wrap, struct util_size *padding, int num, int line)
{

    struct text_font *font = pool_getfont(weight);
    unsigned int icurrent = text_getrowstart(font, strpool_getstring(text), strpool_getcstringlength(text), num, wrap, region->size.w, offx);

    if (icurrent < strpool_getcstringlength(text))
    {

        struct text_rowinfo rowinfo;
        struct util_position offset;

        text_getrowinfo(&rowinfo, font, strpool_getstring(text), strpool_getcstringlength(text), wrap, region->size.w, icurrent);

        offset.x = text_getrowx(&rowinfo, halign, region->size.w - padding->w * 2 - offx) + padding->w + offx;
        offset.y = text_getrowy(&rowinfo, valign, region->size.h - padding->h * 2 - offy) + padding->h + offy + font->lineheight * num;

        if (util_intersects(line, region->position.y + offset.y, region->position.y + offset.y + font->lineheight))
        {

            unsigned int mstart = util_max(0, util_min(markstart, markend) - rowinfo.istart);
            unsigned int mend = util_max(0, util_max(markstart, markend) - rowinfo.istart);

            blit_text(display, font, strpool_getstring(text) + rowinfo.istart, rowinfo.length, region->position.x + offset.x, region->position.y + offset.y, line, x0, x2, mstart, mend, cmap);

        }

    }

}

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_button *button = widget->data;
    struct util_size padding = util_size(CONFIG_BUTTON_PADDING_WIDTH, CONFIG_BUTTON_PADDING_HEIGHT);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 12, 0);

    blit_frame(display, &widget->region, line, x0, x2, cmapbody);
    _renderx(display, &widget->region, button->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_BOLD, ATTR_WRAP_NONE, &padding, 0, line);

}

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_choice *choice = widget->data;
    struct util_size padding = util_size(CONFIG_CHOICE_PADDING_WIDTH, CONFIG_CHOICE_PADDING_HEIGHT);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 12, 0);

    blit_frame(display, &widget->region, line, x0, x2, cmapbody);
    _renderx(display, &widget->region, choice->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_NORMAL, ATTR_WRAP_NONE, &padding, 0, line);

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
        blit_mouse(display, &widget->region, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));

        break;

    case ATTR_MIMETYPE_PCX:
        blit_pcx(display, image->resource, line, strpool_getstring(image->source), widget->region.position.x, widget->region.position.y, x0, x2);

        break;

    }

}

static void renderlayout(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

}

static void renderlistbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_listbox *listbox = widget->data;
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);

    if (listbox->mode == ATTR_MODE_READONLY)
        cmapbody = cmap_get(widget->state, widget->type, 0, 0);

    blit_frame(display, &widget->region, line, x0, x2, cmapbody);

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_select *select = widget->data;
    struct util_size padding = util_size(CONFIG_SELECT_PADDING_WIDTH, CONFIG_SELECT_PADDING_HEIGHT);
    struct util_region rarrow = util_region(widget->region.position.x + widget->region.size.w - padding.w - widget->region.size.h / 2, widget->region.position.y, widget->region.size.h, widget->region.size.h);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmapicon = cmap_get(widget->state, widget->type, 12, 0);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 13, 0);

    blit_frame(display, &widget->region, line, x0, x2, cmapbody);
    blit_iconarrowdown(display, &rarrow, line, x0, x2, cmapicon);
    _renderx(display, &widget->region, select->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_NORMAL, ATTR_WRAP_NONE, &padding, 0, line);

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    unsigned int *cmaptext = cmap_get(widget->state, widget->type, 0, 0);
    unsigned int rownum = (line - widget->region.position.y) / font->lineheight;

    _renderx(display, &widget->region, text->content, cmaptext, x0, x2, text->offx, 0, text->markstart, text->markend, text->halign, text->valign, text->weight, text->wrap, &zerosize, rownum, line);

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbox *textbox = widget->data;
    struct util_region rcursor = util_region(textbox->cursorx, textbox->cursory, textbox->cursorwidth, textbox->cursorheight);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmapicon = cmap_get(WIDGET_STATE_NORMAL, WIDGET_TYPE_TEXT, 0, 0);

    if (textbox->mode == ATTR_MODE_READONLY)
        cmapbody = cmap_get(widget->state, widget->type, 12, 0);

    blit_frame(display, &widget->region, line, x0, x2, cmapbody);
    blit_iconcursor(display, &rcursor, line, x0, x2, cmapicon);

}

static void rendertextbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbutton *textbutton = widget->data;
    struct util_size padding = util_size(CONFIG_TEXTBUTTON_PADDING_WIDTH, CONFIG_TEXTBUTTON_PADDING_HEIGHT);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 12, 0);

    blit_frame(display, &widget->region, line, x0, x2, cmapbody);
    _renderx(display, &widget->region, textbutton->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_NORMAL, ATTR_WRAP_NONE, &padding, 0, line);

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_window *window = widget->data;
    struct util_region rhamburger = util_region(widget->region.position.x, widget->region.position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rminimize = util_region(widget->region.position.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->region.position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rtitle = util_region(widget->region.position.x + CONFIG_WINDOW_BUTTON_WIDTH * 2, widget->region.position.y, widget->region.size.w - CONFIG_WINDOW_BUTTON_WIDTH * 3, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rclose = util_region(widget->region.position.x + widget->region.size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->region.position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rbody = util_region(widget->region.position.x, widget->region.position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->region.size.w, widget->region.size.h - CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onhamburger = util_intersects_region(&rhamburger, mx, my);
    unsigned int onminimize = util_intersects_region(&rminimize, mx, my);
    unsigned int onclose = util_intersects_region(&rclose, mx, my);
    unsigned int *cmaptop = cmap_get(widget->state, widget->type, 0, 0);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 4, 0);
    unsigned int *cmaptitle = cmap_get(widget->state, widget->type, 11, 0);
    unsigned int *cmapiconoff = cmap_get(widget->state, widget->type, 8, 0);
    unsigned int *cmapiconon = cmap_get(widget->state, widget->type, 8, 1);

    blit_frame(display, &rhamburger, line, x0, x2, cmaptop);
    blit_frame(display, &rminimize, line, x0, x2, cmaptop);
    blit_frame(display, &rtitle, line, x0, x2, cmaptop);
    blit_frame(display, &rclose, line, x0, x2, cmaptop);
    blit_frame(display, &rbody, line, x0, x2, cmapbody);
    _renderx(display, &rtitle, window->title, cmaptitle, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_BOLD, ATTR_WRAP_NONE, &zerosize, 0, line);
    blit_iconhamburger(display, &rhamburger, line, x0, x2, (onhamburger) ? cmapiconon : cmapiconoff);
    blit_iconminimize(display, &rminimize, line, x0, x2, (onminimize) ? cmapiconon : cmapiconoff);
    blit_iconx(display, &rclose, line, x0, x2, (onclose) ? cmapiconon : cmapiconoff);

}

void render_place(struct widget *widget, struct util_region *region)
{

    calls[widget->type].place(widget, region, &zerosize);

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

                int x0 = util_max(widget->region.position.x, area.position0.x);
                int x2 = util_min(widget->region.position.x + widget->region.size.w, area.position2.x);

                calls[widget->type].render(display, widget, line, x0, x2, mx, my);

            }

        }

        blit(display, line, area.position0.x, area.position2.x);

    }

}

static void setupcall(unsigned int type, struct util_size (*place)(struct widget *widget, struct util_region *region, struct util_size *min), void (*render)(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my))
{

    struct calls *call = &calls[type];

    call->place = place;
    call->render = render;

}

void render_init(void)
{

    setupcall(WIDGET_TYPE_BUTTON, placebutton, renderbutton);
    setupcall(WIDGET_TYPE_CHOICE, placechoice, renderchoice);
    setupcall(WIDGET_TYPE_FILL, placefill, renderfill);
    setupcall(WIDGET_TYPE_IMAGE, placeimage, renderimage);
    setupcall(WIDGET_TYPE_LAYOUT, placelayout, renderlayout);
    setupcall(WIDGET_TYPE_LISTBOX, placelistbox, renderlistbox);
    setupcall(WIDGET_TYPE_SELECT, placeselect, renderselect);
    setupcall(WIDGET_TYPE_TEXT, placetext, rendertext);
    setupcall(WIDGET_TYPE_TEXTBOX, placetextbox, rendertextbox);
    setupcall(WIDGET_TYPE_TEXTBUTTON, placetextbutton, rendertextbutton);
    setupcall(WIDGET_TYPE_WINDOW, placewindow, renderwindow);

}

