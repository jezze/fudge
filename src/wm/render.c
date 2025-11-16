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

struct calls
{

    void (*place)(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip);
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

static struct util_size sizegrow(struct util_size *size, struct util_size *padding)
{

    struct util_size n;

    n.w = size->w + padding->w * 2;
    n.h = size->h + padding->h * 2;

    return n;

}

static struct util_size sizeclamp(struct util_size *min, struct util_size *max)
{

    struct util_size n;

    n.w = util_clamp(min->w, 0, max->w);
    n.h = util_clamp(min->h, 0, max->h);

    return n;

}

static struct util_region regionshrink(struct util_region *region, struct util_size *padding)
{

    struct util_region r;

    r.x = region->x + padding->w;
    r.y = region->y + padding->h;
    r.w = util_clamp(region->w, 0, region->w - padding->w * 2);
    r.h = util_clamp(region->h, 0, region->h - padding->h * 2);

    return r;

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

static void placewidget(struct widget *widget, struct util_position *position, struct util_size *size, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    widget->bb = util_region(position->x, position->y, util_clamp(size->w, min->w, max->w), util_clamp(size->h, min->h, max->h));
    widget->clip = *clip;

}

static void placechild(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip, struct util_size *padding)
{

    struct util_position cpos = posshrink(pos, padding);
    struct util_size cmax = sizeshrink(max, padding);
    struct util_size cmin = sizeclamp(min, &cmax);

    calls[widget->type].place(widget, &cpos, &cmin, &cmax, clip);

}

static struct util_size placechildren1(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip, struct util_size *padding, unsigned int incx, unsigned int incy, struct util_size *span)
{

    struct list_item *current = 0;
    struct util_size total = zerosize;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_position cpos = util_position(pos->x, pos->y);
        struct util_size cmin = util_size(min->w, min->h);
        struct util_size cmax = util_size(max->w, max->h);

        if (incx)
        {

            cpos.x += total.w;
            cmax.w -= total.w;

            if (child->span)
            {

                cmin.w = span->w * child->span;
                cmax.w = span->w * child->span;

            }

        }

        if (incy)
        {

            cpos.y += total.h;
            cmax.h -= total.h;

            if (child->span)
            {

                cmin.h = span->h * child->span;
                cmax.h = span->h * child->span;

            }

        }

        placechild(child, &cpos, &cmin, &cmax, clip, padding);

        total.w = util_max(total.w, (child->bb.x + child->bb.w) - pos->x);
        total.h = util_max(total.h, (child->bb.y + child->bb.h) - pos->y);

    }

    return total;

}

static struct util_size placechildren(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip, struct util_size *padding, unsigned int incx, unsigned int incy)
{

    unsigned int spans = getnumspans(widget);
    struct util_size span = zerosize;
    struct util_size total = placechildren1(widget, pos, min, max, clip, padding, incx, incy, &span);

    if (spans)
    {

        span = util_size((max->w - total.w) / spans, (max->h - total.h) / spans);
        total = placechildren1(widget, pos, min, max, clip, padding, incx, incy, &span);

    }

    return total;

}

static struct util_size placetextflow(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip, struct util_size *padding)
{

    struct util_size total = zerosize;
    struct list_item *current = 0;
    unsigned int offx = 0;
    unsigned int offy = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_position cpos = util_position(pos->x, pos->y + offy);
        struct util_size cmax = *max;
        struct util_size cmin = *min;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            struct widget_text *text = child->data;

            text->offx = offx;

            placechild(child, &cpos, &cmin, &cmax, clip, padding);

            total.w = util_max(total.w, (child->bb.x + child->bb.w) - pos->x);
            total.h = util_max(total.h, (child->bb.y + child->bb.h) - pos->y);

            offx = text->lastrowx;
            offy += text->lastrowy;

        }

    }

    return total;

}

static void clipchildren(struct widget *widget, struct util_region *clip, struct util_size *padding)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        child->clip = regionshrink(clip, padding);

        clipchildren(child, clip, padding);

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

static void placebutton(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_button *button = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_BOLD);
    struct util_size padding = util_size(CONFIG_BUTTON_PADDING_WIDTH, CONFIG_BUTTON_PADDING_HEIGHT);
    struct text_rowinfo rowinfo;
    struct util_size wsize;

    text_getrowinfo(&rowinfo, font, strpool_getstring(button->label), strpool_getcstringlength(button->label), ATTR_WRAP_NONE, 0, 0);

    wsize = util_size(rowinfo.width + padding.w * 2, rowinfo.lineheight + padding.h * 2);

    placewidget(widget, pos, &wsize, min, max, clip);

}

static void placechoice(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_choice *choice = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct util_size padding = util_size(CONFIG_CHOICE_PADDING_WIDTH, CONFIG_CHOICE_PADDING_HEIGHT);
    struct text_rowinfo rowinfo;
    struct util_size wsize;

    text_getrowinfo(&rowinfo, font, strpool_getstring(choice->label), strpool_getcstringlength(choice->label), ATTR_WRAP_NONE, 0, 0);

    wsize = util_size(rowinfo.width + padding.w * 2, rowinfo.lineheight + padding.h * 2);

    placewidget(widget, pos, &wsize, min, max, clip);

}

static void placefill(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    placewidget(widget, pos, max, min, max, clip);

}

static void placeimagepcx(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_image *image = widget->data;
    struct util_size wsize;

    /* This should be done in some preload state after placement but before rendering. Left in placement for now. */
    if (!image->loaded)
    {

        unsigned int target = fs_auth(strpool_getstring(image->source));

        if (target)
        {

            unsigned int id = fs_walk(1, target, 0, strpool_getstring(image->source));

            if (id)
            {

                struct pcx_header header;

                fs_read_all(1, target, id, &header, sizeof (struct pcx_header), 0);

                image->size = util_size(header.xend - header.xstart + 1, header.yend - header.ystart + 1);
                image->loaded = 1;

            }

        }

    }

    wsize = util_size(image->size.w, image->size.h);

    placewidget(widget, pos, &wsize, min, max, clip);

}

static void placeimagefudgemouse(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct util_position wpos = util_position(widget->bb.x, widget->bb.y);
    struct util_size wsize = util_size(widget->bb.w, widget->bb.h);

    placewidget(widget, &wpos, &wsize, min, max, clip);

}

static void placeimage(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_image *image = widget->data;

    switch (image->mimetype)
    {

    case ATTR_MIMETYPE_FUDGEMOUSE:
        placeimagefudgemouse(widget, pos, min, max, clip);

        break;

    case ATTR_MIMETYPE_PCX:
        placeimagepcx(widget, pos, min, max, clip);

        break;

    }

}

static void placelayout(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_layout *layout = widget->data;
    struct util_size total;
    struct util_size cmin = zerosize;
    struct util_size cpadding = util_size(layout->padding * CONFIG_LAYOUT_PADDING_WIDTH, layout->padding * CONFIG_LAYOUT_PADDING_HEIGHT);
    struct util_size wsize;

    switch (layout->flow)
    {

    case ATTR_FLOW_DEFAULT:
        total = placechildren(widget, pos, &cmin, max, clip, &cpadding, 0, 0);

        break;

    case ATTR_FLOW_HORIZONTAL:
        total = placechildren(widget, pos, &cmin, max, clip, &cpadding, 1, 0);

        break;

    case ATTR_FLOW_HORIZONTALSTRETCH:
        cmin = util_size(0, max->h);
        total = placechildren(widget, pos, &cmin, max, clip, &cpadding, 1, 0);

        break;

    case ATTR_FLOW_VERTICAL:
        total = placechildren(widget, pos, &cmin, max, clip, &cpadding, 0, 1);

        break;

    case ATTR_FLOW_VERTICALSTRETCH:
        cmin = util_size(max->w, 0);
        total = placechildren(widget, pos, &cmin, max, clip, &cpadding, 0, 1);

        break;

    }

    wsize = util_size(total.w, total.h);

    placewidget(widget, pos, &wsize, min, max, clip);

}

static void placelistbox(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_listbox *listbox = widget->data;
    struct util_size padding = util_size(CONFIG_FRAME_WIDTH, CONFIG_FRAME_HEIGHT);
    struct util_position cpos = posshrink(pos, &padding);
    struct util_size cmax = util_size(max->w - padding.w * 2, INFINITY);
    struct util_size total = placechildren(widget, &cpos, &zerosize, &cmax, clip, &zerosize, 0, 1);
    struct util_size wsize = sizegrow(&total, &padding);

    placewidget(widget, pos, &wsize, min, max, clip);
    clipchildren(widget, &widget->bb, &padding);

    listbox->vscroll = util_clamp(listbox->vscroll, widget->bb.h - total.h, 0);

    scrollchildren(widget, 0, listbox->vscroll);

}

static void placeselect(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_select *select = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct util_size padding = util_size(CONFIG_SELECT_PADDING_WIDTH, CONFIG_SELECT_PADDING_HEIGHT);
    struct text_rowinfo rowinfo;
    struct util_position cpos;
    struct util_size cmax;
    struct util_size wsize;

    text_getrowinfo(&rowinfo, font, strpool_getstring(select->label), strpool_getcstringlength(select->label), ATTR_WRAP_NONE, 0, 0);

    wsize = util_size(rowinfo.width + padding.w * 4, rowinfo.lineheight + padding.h * 2);

    cpos = util_position(pos->x, pos->y + wsize.h);
    cmax = util_size(wsize.w * 2, INFINITY);

    placechildren(widget, &cpos, &zerosize, &cmax, clip, &zerosize, 0, 1);
    placewidget(widget, pos, &wsize, min, max, clip);

    if (widget->state != WIDGET_STATE_FOCUS)
    {

        struct util_region cclip = util_region(pos->x, pos->y, 0, 0);

        clipchildren(widget, &cclip, &zerosize);

    }

}

static void placetext(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    struct text_info info;
    struct util_size wsize;

    text_gettextinfo(&info, font, strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, max->w, text->offx);

    text->rows = info.rows;
    text->lastrowx = info.lastrowx;
    text->lastrowy = info.lastrowy;
    wsize = util_size(info.width, info.height);

    placewidget(widget, pos, &wsize, min, max, clip);

}

static void placetextbox(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_textbox *textbox = widget->data;
    struct util_size padding = util_size(CONFIG_TEXTBOX_PADDING_WIDTH, CONFIG_TEXTBOX_PADDING_HEIGHT);
    struct util_position cpos = posshrink(pos, &padding);
    struct util_size cmax = util_size(max->w - padding.w * 2, INFINITY);
    struct util_size total = placetextflow(widget, &cpos, &zerosize, &cmax, clip, &zerosize);
    struct util_size wsize = sizegrow(&total, &padding);
    struct list_item *current = 0;

    placewidget(widget, pos, &wsize, min, max, clip);
    clipchildren(widget, &widget->bb, &padding);

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

            text_gettextinfo(&info, font, strpool_getstring(text->content), textbox->cursor, text->wrap, max->w, text->offx);

            textbox->cursorheight = font->lineheight - font->lineheight / 4;
            textbox->cursorwidth = 2;
            textbox->cursorx = child->bb.x + info.lastrowx;
            textbox->cursory = child->bb.y + info.lastrowy + font->lineheight / 2 - textbox->cursorheight / 2;

        }

    }

}

static void placetextbutton(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct widget_textbutton *textbutton = widget->data;
    struct text_font *font = pool_getfont(ATTR_WEIGHT_NORMAL);
    struct text_rowinfo rowinfo;
    struct util_size wsize;

    text_getrowinfo(&rowinfo, font, strpool_getstring(textbutton->label), strpool_getcstringlength(textbutton->label), ATTR_WRAP_NONE, 0, 0);

    wsize = util_size(rowinfo.width + CONFIG_TEXTBUTTON_PADDING_WIDTH * 2, rowinfo.lineheight + CONFIG_TEXTBUTTON_PADDING_HEIGHT * 2);

    placewidget(widget, pos, &wsize, min, max, clip);

}

static void placewindow(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip)
{

    struct util_position cpos;
    struct util_size cmax;
    struct util_size cpadding;
    struct util_position wpos = util_position(widget->bb.x, widget->bb.y);
    struct util_size wsize = util_size(widget->bb.w, widget->bb.h);

    cpos = util_position(wpos.x, wpos.y + CONFIG_WINDOW_BUTTON_HEIGHT);
    cmax = util_size(wsize.w, wsize.h - CONFIG_WINDOW_BUTTON_HEIGHT);
    cpadding = util_size(CONFIG_WINDOW_BORDER_WIDTH, CONFIG_WINDOW_BORDER_HEIGHT);

    placechildren(widget, &cpos, &zerosize, &cmax, clip, &cpadding, 0, 1);
    placewidget(widget, &wpos, &wsize, min, max, clip);

}

static void _renderx(struct blit_display *display, struct util_region *region, unsigned int text, unsigned int *cmap, int x0, int x2, int offx, int offy, int markstart, int markend, unsigned int halign, unsigned int valign, unsigned int weight, unsigned int wrap, struct util_size *padding, int num, int line)
{

    struct text_font *font = pool_getfont(weight);
    unsigned int icurrent = text_getrowstart(font, strpool_getstring(text), strpool_getcstringlength(text), num, wrap, region->w, offx);
    struct text_rowinfo rowinfo;
    struct util_position offset;

    text_getrowinfo(&rowinfo, font, strpool_getstring(text), strpool_getcstringlength(text), wrap, region->w, icurrent);

    offset.x = text_getrowx(&rowinfo, halign, region->w - padding->w * 2 - offx) + padding->w + offx;
    offset.y = text_getrowy(&rowinfo, valign, region->h - padding->h * 2 - offy) + padding->h + offy + font->lineheight * num;

    if (util_intersects(line, region->y + offset.y, region->y + offset.y + font->lineheight))
    {

        unsigned int mstart = util_max(0, util_min(markstart, markend) - rowinfo.istart);
        unsigned int mend = util_max(0, util_max(markstart, markend) - rowinfo.istart);

        blit_text(display, font, strpool_getstring(text) + rowinfo.istart, rowinfo.length, region->x + offset.x, region->y + offset.y, line, x0, x2, mstart, mend, cmap);

    }

}

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_button *button = widget->data;
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 12, 0);
    struct util_size padding = util_size(CONFIG_BUTTON_PADDING_WIDTH, CONFIG_BUTTON_PADDING_HEIGHT);

    blit_frame(display, &widget->bb, line, x0, x2, cmapbody);
    _renderx(display, &widget->bb, button->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_BOLD, ATTR_WRAP_NONE, &padding, 0, line);

}

static void renderchoice(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_choice *choice = widget->data;
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 12, 0);
    struct util_size padding = util_size(CONFIG_CHOICE_PADDING_WIDTH, CONFIG_CHOICE_PADDING_HEIGHT);

    blit_frame(display, &widget->bb, line, x0, x2, cmapbody);
    _renderx(display, &widget->bb, choice->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_NORMAL, ATTR_WRAP_NONE, &padding, 0, line);

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
        blit_mouse(display, &widget->bb, line, x0, x2, cmap_get(widget->state, widget->type, 0, 0));

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
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);

    if (listbox->mode == ATTR_MODE_READONLY)
        cmapbody = cmap_get(widget->state, widget->type, 0, 0);

    blit_frame(display, &widget->bb, line, x0, x2, cmapbody);

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_select *select = widget->data;
    struct util_region body = util_region(widget->bb.x + widget->bb.w - CONFIG_SELECT_PADDING_WIDTH - widget->bb.h / 2, widget->bb.y, widget->bb.h, widget->bb.h);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmapicon = cmap_get(widget->state, widget->type, 12, 0);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 13, 0);
    struct util_size padding = util_size(CONFIG_SELECT_PADDING_WIDTH, CONFIG_SELECT_PADDING_HEIGHT);

    blit_frame(display, &widget->bb, line, x0, x2, cmapbody);
    blit_iconarrowdown(display, &body, line, x0, x2, cmapicon);
    _renderx(display, &widget->bb, select->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_LEFT, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_NORMAL, ATTR_WRAP_NONE, &padding, 0, line);

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_text *text = widget->data;
    struct text_font *font = pool_getfont(text->weight);
    unsigned int rownum = (line - widget->bb.y) / font->lineheight;
    unsigned int *cmaptext = cmap_get(widget->state, widget->type, 0, 0);

    if (rownum < text->rows)
        _renderx(display, &widget->bb, text->content, cmaptext, x0, x2, text->offx, 0, text->markstart, text->markend, text->halign, text->valign, text->weight, text->wrap, &zerosize, rownum, line);

}

static void rendertextbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbox *textbox = widget->data;
    struct util_region body = util_region(textbox->cursorx, textbox->cursory, textbox->cursorwidth, textbox->cursorheight);
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmapicon = cmap_get(WIDGET_STATE_NORMAL, WIDGET_TYPE_TEXT, 0, 0);

    if (textbox->mode == ATTR_MODE_READONLY)
        cmapbody = cmap_get(widget->state, widget->type, 12, 0);

    blit_frame(display, &widget->bb, line, x0, x2, cmapbody);
    blit_iconcursor(display, &body, line, x0, x2, cmapicon);

}

static void rendertextbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_textbutton *textbutton = widget->data;
    unsigned int *cmapbody = cmap_get(widget->state, widget->type, 0, 4);
    unsigned int *cmaplabel = cmap_get(widget->state, widget->type, 12, 0);
    struct util_size padding = util_size(CONFIG_TEXTBUTTON_PADDING_WIDTH, CONFIG_TEXTBUTTON_PADDING_HEIGHT);

    blit_frame(display, &widget->bb, line, x0, x2, cmapbody);
    _renderx(display, &widget->bb, textbutton->label, cmaplabel, x0, x2, 0, 0, 0, 0, ATTR_HALIGN_CENTER, ATTR_VALIGN_MIDDLE, ATTR_WEIGHT_NORMAL, ATTR_WRAP_NONE, &padding, 0, line);

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my)
{

    struct widget_window *window = widget->data;
    struct util_region rhamburger = util_region(widget->bb.x, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rminimize = util_region(widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rtitle = util_region(widget->bb.x + CONFIG_WINDOW_BUTTON_WIDTH * 2, widget->bb.y, widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH * 3, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rclose = util_region(widget->bb.x + widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_region rbody = util_region(widget->bb.x, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->bb.w, widget->bb.h - CONFIG_WINDOW_BUTTON_HEIGHT);
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

void render_place(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh, struct util_region *clip)
{

    struct util_position pos = util_position(x, y);
    struct util_size min = util_size(minw, minh);
    struct util_size max = util_size(maxw, maxh);

    calls[widget->type].place(widget, &pos, &min, &max, clip);

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

                int x0 = util_max(widget->bb.x, area.position0.x);
                int x2 = util_min(widget->bb.x + widget->bb.w, area.position2.x);

                calls[widget->type].render(display, widget, line, x0, x2, mx, my);

            }

        }

        blit(display, line, area.position0.x, area.position2.x);

    }

}

static void setupcall(unsigned int type, void (*place)(struct widget *widget, struct util_position *pos, struct util_size *min, struct util_size *max, struct util_region *clip), void (*render)(struct blit_display *display, struct widget *widget, int line, int x0, int x2, int mx, int my))
{

    struct calls *call = &calls[type];

    call->place = place;
    call->render = render;

}

void render_init(void)
{

    zerosize = util_size(0, 0);

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

