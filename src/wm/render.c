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

#define INFINITY                50000

static unsigned int hasdamage;

struct
{

    struct util_position position0;
    struct util_position position2;

} area;

struct calls
{

    struct util_size (*getsize)(struct widget *widget, unsigned int maxw, unsigned int maxh);
    void (*place)(struct widget *widget, struct util_region *region);
    void (*clip)(struct widget *widget, struct util_region *clip);
    void (*render)(struct blit_display *display, struct widget *widget, int line, int x0, int x2);

};

static struct util_position zeroposition;
static struct util_size zerosize;
static struct util_position mouse;
static struct calls calls[32];

static struct util_size childrengetsize(struct widget *widget, unsigned int maxw, unsigned int maxh, unsigned int flow)
{

    struct util_position offset = zeroposition;
    struct util_size total = zerosize;
    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_size csize = calls[child->type].getsize(child, maxw - total.w, maxh - total.h);

        switch (flow)
        {

        case ATTR_FLOW_HORIZONTAL:
        case ATTR_FLOW_HORIZONTALSTRETCH:
            if (child->span)
                csize.w = 0;

            break;

        case ATTR_FLOW_VERTICAL:
        case ATTR_FLOW_VERTICALSTRETCH:
            if (child->span)
                csize.h = 0;

            break;

        }

        total.w = util_max(total.w, csize.w + offset.x);
        total.h = util_max(total.h, csize.h + offset.y);

        if (total.w > maxw)
            total.w = maxw;

        if (total.h > maxh)
            total.h = maxh;

        switch (flow)
        {

        case ATTR_FLOW_HORIZONTAL:
        case ATTR_FLOW_HORIZONTALSTRETCH:
            offset.x += csize.w;

            break;

        case ATTR_FLOW_VERTICAL:
        case ATTR_FLOW_VERTICALSTRETCH:
            offset.y += csize.h;

            break;

        }

    }

    return total;

}

static void childrenplace(struct widget *widget, struct util_region *region, unsigned int flow)
{

    struct util_position offset = zeroposition;
    struct util_size total = childrengetsize(widget, region->size.w, region->size.h, flow);
    struct util_size span = zerosize;
    struct list_item *current = 0;
    unsigned int spans = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        spans += child->span;

    }

    if (spans)
    {

        span.w = (region->size.w - total.w) / spans;
        span.h = (region->size.h - total.h) / spans;

    }

    current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_size csize = calls[child->type].getsize(child, region->size.w - offset.x, region->size.h - offset.y);
        struct util_region cregion;

        cregion.position.x = region->position.x;
        cregion.position.y = region->position.y;
        cregion.size.w = csize.w;
        cregion.size.h = csize.h;

        switch (flow)
        {

        case ATTR_FLOW_HORIZONTAL:
        case ATTR_FLOW_HORIZONTALSTRETCH:
            cregion.position.x += offset.x;

            if (child->span)
            {

                cregion.size.w = child->span * span.w;
                csize = calls[child->type].getsize(child, cregion.size.w, region->size.h - offset.y);
                cregion.size.h = csize.h;

            }

            offset.x += cregion.size.w;

            break;

        case ATTR_FLOW_VERTICAL:
        case ATTR_FLOW_VERTICALSTRETCH:
            cregion.position.y += offset.y;

            if (child->span)
            {

                cregion.size.h = child->span * span.h;
                csize = calls[child->type].getsize(child, region->size.w - offset.x, cregion.size.h);
                cregion.size.w = csize.w;

            }

            offset.y += cregion.size.h;

            break;

        }

        switch (flow)
        {

        case ATTR_FLOW_DEFAULT:
            cregion.size.w = region->size.w;
            cregion.size.h = region->size.h;

            break;

        case ATTR_FLOW_HORIZONTALSTRETCH:
            cregion.size.h = region->size.h;

            break;

        case ATTR_FLOW_VERTICALSTRETCH:
            cregion.size.w = region->size.w;

            break;

        }

        calls[child->type].place(child, &cregion);

    }

}

static void childrenclip(struct widget *widget, struct util_region *clip)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        calls[child->type].clip(child, clip);

    }

}

static struct util_size getsizebutton(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    struct widget_button *button = widget->data;
    struct text_info info;

    text_gettextinfo(&info, pool_getfont(ATTR_WEIGHT_BOLD), strpool_getstring(button->label), strpool_getcstringlength(button->label), ATTR_WRAP_NONE, maxw, 0);

    return util_size(info.width + CONFIG_BUTTON_PADDING_WIDTH * 2, info.height + CONFIG_BUTTON_PADDING_HEIGHT * 2);

}

static struct util_size getsizechoice(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    struct widget_choice *choice = widget->data;
    struct text_info info;

    text_gettextinfo(&info, pool_getfont(ATTR_WEIGHT_NORMAL), strpool_getstring(choice->label), strpool_getcstringlength(choice->label), ATTR_WRAP_NONE, maxw, 0);

    return util_size(info.width + CONFIG_CHOICE_PADDING_WIDTH * 2, info.height + CONFIG_CHOICE_PADDING_HEIGHT * 2);

}

static struct util_size getsizefill(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    return zerosize;

}

static struct util_size getsizeimage(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    struct widget_image *image = widget->data;

    if (image->mimetype == ATTR_MIMETYPE_FUDGEMOUSE)
        return widget->region.size;

    return util_size(image->size.w, image->size.h);

}

static struct util_size getsizelayout(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    struct widget_layout *layout = widget->data;
    struct util_size total = childrengetsize(widget, maxw, maxh, layout->flow);

    return util_size(total.w, total.h);

}

static struct util_size getsizelistbox(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    struct util_size total = childrengetsize(widget, maxw, maxh, ATTR_FLOW_VERTICAL);

    return util_size(total.w + CONFIG_FRAME_WIDTH * 2, total.h + CONFIG_FRAME_HEIGHT * 2);

}

static struct util_size getsizeselect(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    struct widget_select *select = widget->data;
    struct text_info info;

    text_gettextinfo(&info, pool_getfont(ATTR_WEIGHT_NORMAL), strpool_getstring(select->label), strpool_getcstringlength(select->label), ATTR_WRAP_NONE, maxw, 0);

    return util_size(info.width + CONFIG_SELECT_PADDING_WIDTH * 4, info.height + CONFIG_SELECT_PADDING_HEIGHT * 2);

}

static struct util_size getsizetext(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    struct widget_text *text = widget->data;
    struct text_info info;

    text_gettextinfo(&info, pool_getfont(text->weight), strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, maxw, 0);

    return util_size(info.width, info.height);

}

static struct util_size getsizetextbox(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    struct util_size total = childrengetsize(widget, maxw, maxh, ATTR_FLOW_VERTICAL);

    return util_size(total.w + CONFIG_TEXTBOX_PADDING_WIDTH * 2, total.h + CONFIG_TEXTBOX_PADDING_HEIGHT * 2);

}

static struct util_size getsizetextbutton(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    struct widget_textbutton *textbutton = widget->data;
    struct text_info info;

    text_gettextinfo(&info, pool_getfont(ATTR_WEIGHT_NORMAL), strpool_getstring(textbutton->label), strpool_getcstringlength(textbutton->label), ATTR_WRAP_NONE, maxw, 0);

    return util_size(info.width + CONFIG_TEXTBUTTON_PADDING_WIDTH * 2, info.height + CONFIG_TEXTBUTTON_PADDING_HEIGHT * 2);

}

static struct util_size getsizewindow(struct widget *widget, unsigned int maxw, unsigned int maxh)
{

    return widget->region.size;

}

static void placebutton(struct widget *widget, struct util_region *region)
{

    widget->region = *region;

}

static void placechoice(struct widget *widget, struct util_region *region)
{

    widget->region = *region;

}

static void placefill(struct widget *widget, struct util_region *region)
{

    widget->region = *region;

}

static void placeimage(struct widget *widget, struct util_region *region)
{

    struct widget_image *image = widget->data;

    if (image->mimetype == ATTR_MIMETYPE_FUDGEMOUSE)
        widget->region.size = calls[widget->type].getsize(widget, INFINITY, INFINITY);
    else
        widget->region = *region;

}

static void placelayout(struct widget *widget, struct util_region *region)
{

    struct widget_layout *layout = widget->data;

    widget->region = *region;

    childrenplace(widget, region, layout->flow);

}

static void placelistbox(struct widget *widget, struct util_region *region)
{

    struct util_region cregion = util_region(region->position.x + CONFIG_FRAME_WIDTH, region->position.y + CONFIG_FRAME_HEIGHT, region->size.w - CONFIG_FRAME_WIDTH * 2, INFINITY);

    widget->region = *region;

    childrenplace(widget, &cregion, ATTR_FLOW_VERTICALSTRETCH);

}

static void placeselect(struct widget *widget, struct util_region *region)
{

    struct util_size wsize = calls[widget->type].getsize(widget, INFINITY, INFINITY);
    struct util_region cregion = util_region(region->position.x, region->position.y + wsize.h, INFINITY, INFINITY);

    widget->region = *region;

    childrenplace(widget, &cregion, ATTR_FLOW_VERTICAL);

}

static void placetext(struct widget *widget, struct util_region *region)
{

    widget->region = *region;

}

static void placetextbox(struct widget *widget, struct util_region *region)
{

    struct util_region cregion = util_region(region->position.x + CONFIG_TEXTBOX_PADDING_WIDTH, region->position.y + CONFIG_TEXTBOX_PADDING_HEIGHT, region->size.w - CONFIG_TEXTBOX_PADDING_WIDTH * 2, INFINITY);

    widget->region = *region;

    childrenplace(widget, &cregion, ATTR_FLOW_VERTICAL);

}

static void placetextbutton(struct widget *widget, struct util_region *region)
{

    widget->region = *region;

}

static void placewindow(struct widget *widget, struct util_region *region)
{

    struct util_region cregion = util_region(widget->region.position.x, widget->region.position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->region.size.w, widget->region.size.h - CONFIG_WINDOW_BUTTON_HEIGHT);

    childrenplace(widget, &cregion, ATTR_FLOW_VERTICALSTRETCH);

}

static void clipbutton(struct widget *widget, struct util_region *clip)
{

    widget->clip = *clip;

}

static void clipchoice(struct widget *widget, struct util_region *clip)
{

    widget->clip = *clip;

}

static void clipfill(struct widget *widget, struct util_region *clip)
{

    widget->clip = *clip;

}

static void clipimage(struct widget *widget, struct util_region *clip)
{

    widget->clip = *clip;

}

static void cliplayout(struct widget *widget, struct util_region *clip)
{

    widget->clip = *clip;

    childrenclip(widget, &widget->clip);

}

static void cliplistbox(struct widget *widget, struct util_region *clip)
{

    widget->clip = util_region_intersection(&widget->region, clip);

    childrenclip(widget, &widget->clip);

}

static void clipselect(struct widget *widget, struct util_region *clip)
{

    widget->clip = *clip;

    if (widget->state != WIDGET_STATE_FOCUS)
        widget->clip = util_region_intersection(&widget->region, clip);

    childrenclip(widget, &widget->clip);

}

static void cliptext(struct widget *widget, struct util_region *clip)
{

    widget->clip = *clip;

}

static void cliptextbox(struct widget *widget, struct util_region *clip)
{

    widget->clip = util_region_intersection(&widget->region, clip);

    childrenclip(widget, &widget->clip);

}

static void cliptextbutton(struct widget *widget, struct util_region *clip)
{

    widget->clip = *clip;

}

static void clipwindow(struct widget *widget, struct util_region *clip)
{

    widget->clip = *clip;

    childrenclip(widget, &widget->clip);

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

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_button *button = widget->data;
    struct util_size padding = util_size(CONFIG_BUTTON_PADDING_WIDTH, CONFIG_BUTTON_PADDING_HEIGHT);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 12, 0);

    blit_frame(display, &widget->region, line, x0, x2, cmapbody);
    _renderx(display, &widget->region, button->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_BOLD, ATTR_WRAP_NONE, &padding, 0, line);

}

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_choice *choice = widget->data;
    struct util_size padding = util_size(CONFIG_CHOICE_PADDING_WIDTH, CONFIG_CHOICE_PADDING_HEIGHT);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 12, 0);

    blit_frame(display, &widget->region, line, x0, x2, cmapbody);
    _renderx(display, &widget->region, choice->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_NORMAL, ATTR_WRAP_NONE, &padding, 0, line);

}

static void renderfill(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_fill *fill = widget->data;

    blit_line(display, fill->color, x0, x2);

}

/*
static void rendergradient(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
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

static void renderimage(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
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

static void renderlayout(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

}

static void renderlistbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_listbox *listbox = widget->data;
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmapbodyro = cmap_get(widget->state, widget->type, 0, 0);

    blit_frame(display, &widget->region, line, x0, x2, (listbox->mode == ATTR_MODE_READONLY) ? cmapbodyro : cmapbody);

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
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

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    unsigned int *cmaptext = cmap_get(widget->state, widget->type, 0, 0);
    unsigned int rownum = (line - widget->region.position.y) / font->lineheight;

    _renderx(display, &widget->region, text->content, cmaptext, x0, x2, text->inlinex, 0, text->markstart, text->markend, text->halign, text->valign, text->weight, text->wrap, &zerosize, rownum, line);

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_textbox *textbox = widget->data;
    struct util_region rcursor = util_region(textbox->cursorx, textbox->cursory, textbox->cursorwidth, textbox->cursorheight);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmapbodyro = cmap_get(widget->state, widget->type, 12, 0);
    unsigned int *cmapicon = cmap_get(WIDGET_STATE_NORMAL, WIDGET_TYPE_TEXT, 0, 0);

    blit_frame(display, &widget->region, line, x0, x2, (textbox->mode == ATTR_MODE_READONLY) ? cmapbodyro : cmapbody);
    blit_iconcursor(display, &rcursor, line, x0, x2, cmapicon);

}

static void rendertextbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_textbutton *textbutton = widget->data;
    struct util_size padding = util_size(CONFIG_TEXTBUTTON_PADDING_WIDTH, CONFIG_TEXTBUTTON_PADDING_HEIGHT);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 12, 0);

    blit_frame(display, &widget->region, line, x0, x2, cmapbody);
    _renderx(display, &widget->region, textbutton->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_NORMAL, ATTR_WRAP_NONE, &padding, 0, line);

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct widget_window *window = widget->data;
    struct util_region rhamburger = util_region(widget->region.position.x, widget->region.position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rminimize = util_region(widget->region.position.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->region.position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rtitle = util_region(widget->region.position.x + CONFIG_WINDOW_BUTTON_WIDTH * 2, widget->region.position.y, widget->region.size.w - CONFIG_WINDOW_BUTTON_WIDTH * 3, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rclose = util_region(widget->region.position.x + widget->region.size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->region.position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rbody = util_region(widget->region.position.x, widget->region.position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->region.size.w, widget->region.size.h - CONFIG_WINDOW_BUTTON_HEIGHT);
    unsigned int onhamburger = util_region_intersects(&rhamburger, mouse.x, mouse.y);
    unsigned int onminimize = util_region_intersects(&rminimize, mouse.x, mouse.y);
    unsigned int onclose = util_region_intersects(&rclose, mouse.x, mouse.y);
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

void render_setmouse(int x, int y)
{

    mouse.x = x;
    mouse.y = y;

}

void render_place(struct widget *widget, struct util_region *region)
{

    calls[widget->type].place(widget, region);
    calls[widget->type].clip(widget, region);

}

void render_damage(int x0, int y0, int x2, int y2)
{

    if (hasdamage)
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

    hasdamage = 1;

}

void render_undamage(void)
{

    hasdamage = 0;

}

void render_update(struct blit_display *display)
{

    int line;

    if (!hasdamage)
        return;

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

                calls[widget->type].render(display, widget, line, x0, x2);

            }

        }

        blit(display, line, area.position0.x, area.position2.x);

    }

}

static void setupcall(unsigned int type, struct util_size (*getsize)(struct widget *widget, unsigned int maxw, unsigned int maxh), void (*place)(struct widget *widget, struct util_region *region), void (*clip)(struct widget *widget, struct util_region *clip), void (*render)(struct blit_display *display, struct widget *widget, int line, int x0, int x2))
{

    struct calls *call = &calls[type];

    call->getsize = getsize;
    call->place = place;
    call->clip = clip;
    call->render = render;

}

void render_init(void)
{

    setupcall(WIDGET_TYPE_BUTTON, getsizebutton, placebutton, clipbutton, renderbutton);
    setupcall(WIDGET_TYPE_CHOICE, getsizechoice, placechoice, clipchoice, renderchoice);
    setupcall(WIDGET_TYPE_FILL, getsizefill, placefill, clipfill, renderfill);
    setupcall(WIDGET_TYPE_IMAGE, getsizeimage, placeimage, clipimage, renderimage);
    setupcall(WIDGET_TYPE_LAYOUT, getsizelayout, placelayout, cliplayout, renderlayout);
    setupcall(WIDGET_TYPE_LISTBOX, getsizelistbox, placelistbox, cliplistbox, renderlistbox);
    setupcall(WIDGET_TYPE_SELECT, getsizeselect, placeselect, clipselect, renderselect);
    setupcall(WIDGET_TYPE_TEXT, getsizetext, placetext, cliptext, rendertext);
    setupcall(WIDGET_TYPE_TEXTBOX, getsizetextbox, placetextbox, cliptextbox, rendertextbox);
    setupcall(WIDGET_TYPE_TEXTBUTTON, getsizetextbutton, placetextbutton, cliptextbutton, rendertextbutton);
    setupcall(WIDGET_TYPE_WINDOW, getsizewindow, placewindow, clipwindow, renderwindow);

}

