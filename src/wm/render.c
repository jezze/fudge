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

#define INFINITY                    0xFFFF
#define DIRECTION_NONE              0
#define DIRECTION_HORIZONTAL        1
#define DIRECTION_VERTICAL          2
#define STRETCH_NONE                0
#define STRETCH_HORIZONTAL          1
#define STRETCH_VERTICAL            2
#define STRETCH_BOTH                3

static unsigned int hasdamage;

struct
{

    struct util_position position0;
    struct util_position position2;

} area;

struct calls
{

    struct util_size (*getsize)(struct widget *widget, struct util_size *limit, struct util_position *rowstart);
    void (*place)(struct widget *widget);
    void (*render)(struct blit_display *display, struct widget *widget, int line, int x0, int x2);

};

static struct util_position zeroposition;
static struct util_size zerosize;
static unsigned int measurespans;
static unsigned int measureminimum;
static struct util_position mouse;
static unsigned int mousetype;
static struct calls calls[32];

#define PALETTE_BUTTON                  0
#define PALETTE_CHECKBOX                1
#define PALETTE_BOX                     2
#define PALETTE_ITEM                    3
#define PALETTE_LINK                    4
#define PALETTE_MOUSE                   5
#define PALETTE_PANEL                   6
#define PALETTE_SELECT                  7
#define PALETTE_SWATCH                  8
#define PALETTE_TEXT                    9
#define PALETTE_WINDOW                  10
#define NUM_PALETTES                    11
#define SHADE_NORMAL                    0
#define SHADE_HOVER                     1
#define SHADE_FOCUS                     2
#define SHADE_DISABLED                  3
#define NUM_SHADES                      4

struct palette
{

    unsigned int *frame;
    unsigned int *text;
    unsigned int *icon;

};

static char *palettenames[NUM_PALETTES] = {"button", "checkbox", "box", "item", "link", "mouse", "panel", "select", "swatch", "text", "window"};
static char *shadenames[NUM_SHADES] = {"normal", "hover", "focus", "disabled"};
static struct palette palettes[NUM_PALETTES][NUM_SHADES];

static unsigned int getshade(struct widget *widget)
{

    if (widget->attributes.mode == ATTR_MODE_DISABLED)
        return SHADE_DISABLED;

    switch (widget->state)
    {

    case WIDGET_STATE_FOCUS:
        return SHADE_FOCUS;

    case WIDGET_STATE_HOVER:
        return SHADE_HOVER;

    }

    return SHADE_NORMAL;

}

static struct palette *getpalette(unsigned int palette, unsigned int shade)
{

    return &palettes[palette][shade];

}

static unsigned int getdirection(unsigned int flow)
{

    switch (flow)
    {

    case ATTR_FLOW_HORIZONTAL:
    case ATTR_FLOW_HORIZONTALSTRETCH:
        return DIRECTION_HORIZONTAL;

    case ATTR_FLOW_VERTICAL:
    case ATTR_FLOW_VERTICALSTRETCH:
        return DIRECTION_VERTICAL;

    }

    return DIRECTION_NONE;

}

static unsigned int getstretch(unsigned int flow)
{

    switch (flow)
    {

    case ATTR_FLOW_STRETCH:
        return STRETCH_BOTH;

    case ATTR_FLOW_HORIZONTALSTRETCH:
        return STRETCH_VERTICAL;

    case ATTR_FLOW_VERTICALSTRETCH:
        return STRETCH_HORIZONTAL;

    }

    return STRETCH_NONE;

}

static unsigned int isinlinetext(struct widget *widget)
{

    return widget->type == WIDGET_TYPE_TEXT && widget->attributes.display == ATTR_DISPLAY_INLINE;

}

static void advance(struct widget *widget, struct widget *child, unsigned int direction, struct util_size *size, struct util_position *offset, struct util_position *rowstart, int *pending)
{

    switch (direction)
    {

    case DIRECTION_HORIZONTAL:
        offset->x += size->w + widget->attributes.spacing;

        break;

    case DIRECTION_VERTICAL:
        if (isinlinetext(child))
        {

            offset->y += child->rowstop.y;
            *pending = size->h - child->rowstop.y;

        }

        else
        {

            offset->y += size->h + widget->attributes.spacing;

        }

        break;

    }

    *rowstart = (child->attributes.display == ATTR_DISPLAY_INLINE) ? child->rowstop : zeroposition;

}

static void startchild(struct widget *child, struct util_position *offset, int *pending)
{

    if (!isinlinetext(child))
    {

        offset->y += *pending;
        *pending = 0;

    }

}

static struct util_size childrengetsize(struct widget *widget, struct util_size *limit)
{

    unsigned int direction = getdirection(widget->attributes.flow);
    unsigned int padding = widget->attributes.padding;
    struct util_size inner = util_size_sub(limit, padding * 2, padding * 2);
    struct util_position rowstart = zeroposition;
    struct util_position offset = zeroposition;
    struct util_size total = zerosize;
    struct list_item *current = 0;
    int pending = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_size climit;
        struct util_size csize;

        startchild(child, &offset, &pending);

        climit = util_size_sub(&inner, offset.x, offset.y);
        csize = calls[child->type].getsize(child, &climit, &rowstart);

        /* scrolling is only vertical, so the content still needs its full width and room for the thumb */
        if (widget_isscrollable(child))
        {

            csize.w = util_min(csize.w + CONFIG_SCROLL_WIDTH, climit.w);

            if (measureminimum)
                csize.h = util_min(csize.h, (CONFIG_FRAME_HEIGHT + child->attributes.padding) * 2 + pool_getfont(child->attributes.weight)->lineheight);

        }

        if (child->attributes.span && !measurespans)
        {

            switch (direction)
            {

            case DIRECTION_HORIZONTAL:
                csize.w = 0;

                break;

            case DIRECTION_VERTICAL:
                csize.h = 0;

                break;

            }

        }

        total.w = util_clamp(util_max(total.w, csize.w + offset.x), 0, inner.w);
        total.h = util_clamp(util_max(total.h, csize.h + offset.y), 0, inner.h);

        advance(widget, child, direction, &csize, &offset, &rowstart, &pending);

    }

    return util_size(total.w + padding * 2, total.h + padding * 2);

}

static struct util_region getplacement(struct widget *widget, struct widget *child, struct util_position *offset, struct util_position *rowstart, struct util_size *span)
{

    unsigned int stretch = getstretch(widget->attributes.flow);
    struct util_size climit = util_size_sub(&widget->cplacement.size, offset->x, offset->y);
    struct util_region placement;

    placement.position = util_position_add(&widget->cplacement.position, offset->x, offset->y);
    placement.size = calls[child->type].getsize(child, &climit, rowstart);

    if (child->attributes.span)
    {

        unsigned int direction = getdirection(widget->attributes.flow);

        switch (direction)
        {

        case DIRECTION_HORIZONTAL:
            placement.size.w = child->attributes.span * span->w;

            break;

        case DIRECTION_VERTICAL:
            placement.size.h = child->attributes.span * span->h;

            break;

        }

    }

    switch (stretch)
    {

    case STRETCH_HORIZONTAL:
        placement.size.w = climit.w;

        break;

    case STRETCH_VERTICAL:
        placement.size.h = climit.h;

        break;

    case STRETCH_BOTH:
        placement.size.w = climit.w;
        placement.size.h = climit.h;

        break;

    }

    return placement;

}

static void place(struct widget *widget, struct util_region *placement, struct util_region *clip);

struct placeinfo
{

    unsigned int spans;
    struct util_size total;
    struct util_size freesize;
    struct util_size span;

};

static struct placeinfo placechildren(struct widget *widget, struct util_size *span, unsigned int measure)
{

    unsigned int direction = getdirection(widget->attributes.flow);
    struct util_position rowstart = zeroposition;
    struct util_position offset = zeroposition;
    struct list_item *current = 0;
    struct placeinfo info;
    int pending = 0;

    info.spans = 0;
    info.total = zerosize;
    info.freesize = zerosize;
    info.span = zerosize;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        struct util_region cplacement;

        startchild(child, &offset, &pending);

        cplacement = getplacement(widget, child, &offset, &rowstart, span);

        if (measure)
        {

            if (child->attributes.display == ATTR_DISPLAY_FIXED)
                cplacement.size = child->size;

        }

        else
        {

            place(child, &cplacement, &widget->cclip);

            cplacement.size = child->placement.size;

        }

        info.spans += child->attributes.span;
        info.total.w = util_clamp(util_max(info.total.w, cplacement.size.w + offset.x), 0, widget->cplacement.size.w);
        info.total.h = util_clamp(util_max(info.total.h, cplacement.size.h + offset.y), 0, widget->cplacement.size.h);

        advance(widget, child, direction, &cplacement.size, &offset, &rowstart, &pending);

    }

    info.freesize = util_size_sub(&widget->cplacement.size, info.total.w, info.total.h);

    if (info.spans)
        info.span = util_size(info.freesize.w / info.spans, info.freesize.h / info.spans);

    return info;

}

static void placecursor(struct widget *widget)
{

    struct list_item *current = 0;
    struct widget *text = 0;

    widget->cursorsize = zerosize;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
            text = child;

    }

    if (text)
    {

        struct text_font *font = pool_getfont(text->attributes.weight);
        char *label = strpool_getstring(text->attributes.label);
        unsigned int total = strpool_getcstringlength(text->attributes.label);
        unsigned int length = util_min(widget->attributes.cursor, total);
        unsigned int maxw = widget->cplacement.size.w - (text->placement.position.x - widget->cplacement.position.x);
        struct text_info info = text_info(font, label, length, text->attributes.wrap, maxw, text->rowstart.x);
        unsigned int num = info.lastrow.y / font->lineheight;
        int offx = (num) ? 0 : text->rowstart.x;
        struct text_rowinfo rowinfo;
        int x;

        /* place the cursor on its row the same way the row is aligned when the text is drawn */
        rowinfo.width = 0;
        rowinfo.lineheight = font->lineheight;

        text_getrowinfo(&rowinfo, font, label, total, text->attributes.wrap, util_max((int)text->placement.size.w - offx, 0), text_getrowstart(font, label, total, 0, 0, num, text->attributes.wrap, text->placement.size.w, text->rowstart.x));

        x = text_getrowx(&rowinfo, text->attributes.halign, text->placement.size.w - offx) + offx + (info.lastrow.x - offx);

        widget->cursorposition = util_position(text->placement.position.x + x, text->placement.position.y + info.lastrow.y);
        widget->cursorsize = util_size(2, font->lineheight);

    }

}

static unsigned int hasspans(struct widget *widget)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->attributes.span)
            return 1;

    }

    return 0;

}

static void clipcontent(struct widget *widget)
{

    struct util_region content;

    switch (widget->type)
    {

    case WIDGET_TYPE_LAYOUT:
    case WIDGET_TYPE_SELECT:
        return;

    case WIDGET_TYPE_WINDOW:
        content = widget->cplacement;

        break;

    default:
        {

            int left = widget->cplacement.position.x - widget->placement.position.x;
            int top = widget->cplacement.position.y + widget->scroll.y - widget->placement.position.y;

            content = util_region(widget->placement.position.x + left, widget->placement.position.y + top, util_max((int)widget->placement.size.w - left * 2, 0), util_max((int)widget->placement.size.h - top * 2, 0));

        }

        break;

    }

    widget->cclip = util_region_intersection(&content, &widget->cclip);

}

/* the thumb runs in a track along the right edge and is as tall, compared to the track, as the visible part is compared to all of the content */
static void placethumb(struct widget *widget, int top)
{

    int view = (int)widget->placement.size.h - top * 2;
    int track = util_max((int)widget->placement.size.h - CONFIG_FRAME_HEIGHT * 2, 0);
    int x = widget->placement.position.x + (int)widget->placement.size.w - CONFIG_FRAME_WIDTH - CONFIG_SCROLL_WIDTH;

    widget->thumb = util_region(x, widget->placement.position.y + CONFIG_FRAME_HEIGHT, CONFIG_SCROLL_WIDTH, track);

    if (widget->scrollmax > 0 && view > 0)
    {

        widget->thumb.size.h = util_clamp(track * view / (view + widget->scrollmax), util_min(CONFIG_SCROLL_MINIMUM, track), track);
        widget->thumb.position.y += (track - (int)widget->thumb.size.h) * widget->scroll.y / widget->scrollmax;

    }

}

static void place(struct widget *widget, struct util_region *placement, struct util_region *clip)
{

    struct placeinfo info;
    struct util_size span;

    if (widget->attributes.display == ATTR_DISPLAY_FIXED)
    {

        widget->placement.position = widget->position;
        widget->placement.size = widget->size;

    }

    else
    {

        widget->placement.position = placement->position;
        widget->placement.size = placement->size;

    }

    widget->clip = *clip;
    widget->cclip = *clip;
    widget->textrow = 0;
    widget->textoffset = 0;

    calls[widget->type].place(widget);

    /* the content of a scrolling widget is as tall as it needs, moved by the scroll and cut to the widget; the thumb sits against the border, so the padding goes between it and the content */
    if (widget_isscrollable(widget))
    {

        widget->cplacement.position.y -= widget->scroll.y;
        widget->cplacement.size.h = INFINITY;
        widget->cplacement.size.w = util_max((int)widget->cplacement.size.w - CONFIG_SCROLL_WIDTH, 0);
        widget->cclip = util_region_intersection(&widget->cclip, &widget->placement);

    }

    if (widget->attributes.padding)
    {

        unsigned int padding = widget->attributes.padding;

        widget->cplacement.position = util_position_add(&widget->cplacement.position, padding, padding);
        widget->cplacement.size = util_size_sub(&widget->cplacement.size, padding * 2, padding * 2);

    }

    clipcontent(widget);

    span = zerosize;

    if (hasspans(widget))
    {

        info = placechildren(widget, &zerosize, 1);
        span = info.span;

    }

    info = placechildren(widget, &span, 0);

    if (widget->type == WIDGET_TYPE_BOX)
        placecursor(widget);

    if (widget_isscrollable(widget))
    {

        int top = widget->cplacement.position.y + widget->scroll.y - widget->placement.position.y;
        int y = widget->scroll.y;

        if (widget->followcursor && widget->cursorsize.h)
        {

            int visibletop = widget->placement.position.y + top;
            int visiblebottom = widget->placement.position.y + (int)widget->placement.size.h - top;

            if (widget->cursorposition.y < visibletop)
                y -= visibletop - widget->cursorposition.y;

            if (widget->cursorposition.y + (int)widget->cursorsize.h > visiblebottom)
                y += widget->cursorposition.y + widget->cursorsize.h - visiblebottom;

        }

        widget->followcursor = 0;
        widget->scrollmax = util_max(0, top * 2 + info.total.h - (int)widget->placement.size.h);
        y = util_clamp(y, 0, widget->scrollmax);

        if (y != widget->scroll.y)
        {

            widget->scroll.y = y;

            place(widget, placement, clip);

            return;

        }

        placethumb(widget, top);

    }

}

static struct util_size textsize(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    struct text_font *font = pool_getfont(widget->attributes.weight);
    char *label = strpool_getstring(widget->attributes.label);
    unsigned int length = strpool_getcstringlength(widget->attributes.label);
    struct text_info info = text_info(font, label, length, widget->attributes.wrap, limit->w, rowstart->x);

    if (widget->attributes.display == ATTR_DISPLAY_INLINE)
    {

        widget->rowstart = *rowstart;
        widget->rowstop = info.lastrow;

    }

    /* text that wraps can be made as narrow as one character */
    if (measureminimum && widget->attributes.wrap != ATTR_WRAP_NONE)
    {

        struct text_info narrow = text_info(font, label, length, widget->attributes.wrap, 1, 0);

        return util_size(narrow.width, info.height);

    }

    return util_size(info.width, info.height);

}

static struct util_size getsizebutton(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    struct util_size padding = util_size(CONFIG_BUTTON_PADDING_WIDTH * 2, CONFIG_BUTTON_PADDING_HEIGHT * 2);
    struct util_size label = textsize(widget, limit, rowstart);

    return util_size_union(&label, &padding);

}

static struct util_size getsizecheckbox(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    return util_size(CONFIG_CHECKBOX_WIDTH, CONFIG_CHECKBOX_HEIGHT);

}

static struct util_size getsizefill(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    return widget->size;

}

static struct util_size getsizeimage(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    return widget->size;

}

static struct util_size getsizelayout(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    struct util_size csize = childrengetsize(widget, limit);

    return util_size(csize.w, csize.h);

}

static struct util_size getsizeswatch(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    unsigned int side = pool_getfont(widget->attributes.weight)->lineheight + CONFIG_SWATCH_PADDING * 2;

    return util_size(side, side);

}

static struct util_size getsizebox(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    struct util_size border = util_size(CONFIG_FRAME_WIDTH * 2, CONFIG_FRAME_HEIGHT * 2);
    struct util_size csize = childrengetsize(widget, limit);

    /* a box to type in needs room for a line of text, however short its text is */
    if (widget->attributes.mode == ATTR_MODE_EDIT)
    {

        csize.w = util_max(csize.w, CONFIG_BOX_EDITWIDTH + widget->attributes.padding * 2);
        csize.h = util_max(csize.h, pool_getfont(widget->attributes.weight)->lineheight + widget->attributes.padding * 2);

    }

    return util_size_union(&csize, &border);

}

static struct util_size getsizepanel(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    struct util_size csize = childrengetsize(widget, limit);

    return util_size(csize.w, csize.h);

}

static struct util_size getsizeselect(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    struct util_size padding = util_size(CONFIG_SELECT_PADDING_WIDTH * 4, CONFIG_SELECT_PADDING_HEIGHT * 2);
    struct util_size label = textsize(widget, limit, rowstart);

    return util_size_union(&label, &padding);

}

static struct util_size getsizetext(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    struct util_size label = textsize(widget, limit, rowstart);

    return util_size(label.w, label.h);

}

static struct util_size getsizeitem(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    return childrengetsize(widget, limit);

}

static struct util_size getsizewindow(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    return widget->size;

}

static void placebutton(struct widget *widget)
{

}

static void placecheckbox(struct widget *widget)
{

}

static void placefill(struct widget *widget)
{

}

static void placeimage(struct widget *widget)
{

}

static void placelayout(struct widget *widget)
{

    widget->cplacement = widget->placement;

}

static void placeswatch(struct widget *widget)
{

}

static void placebox(struct widget *widget)
{

    struct util_region inner = util_region(widget->placement.position.x + CONFIG_FRAME_WIDTH, widget->placement.position.y + CONFIG_FRAME_HEIGHT, widget->placement.size.w - CONFIG_FRAME_WIDTH * 2, widget->placement.size.h - CONFIG_FRAME_HEIGHT * 2);

    widget->cclip = util_region_intersection(&inner, &widget->clip);
    widget->clip = util_region_intersection(&widget->placement, &widget->clip);
    widget->cplacement = inner;

}

static void placepanel(struct widget *widget)
{

    widget->cplacement = widget->placement;

}

static void placeselect(struct widget *widget)
{

    widget->cplacement = util_region(widget->placement.position.x, widget->placement.position.y + widget->placement.size.h, INFINITY, INFINITY);

    if (widget->state != WIDGET_STATE_FOCUS)
        widget->clip = util_region_intersection(&widget->placement, &widget->clip);

    widget->cclip = widget->clip;

}

static void placetext(struct widget *widget)
{

}

static void placeitem(struct widget *widget)
{

    widget->cplacement = widget->placement;

}

static void placewindow(struct widget *widget)
{

    widget->clip = util_region_intersection(&widget->placement, &widget->clip);
    widget->cclip = widget->clip;

    widget->cplacement = util_region(widget->placement.position.x, widget->placement.position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->placement.size.w, widget->placement.size.h - CONFIG_WINDOW_BUTTON_HEIGHT);

}

static void textrender(struct blit_display *display, struct widget *widget, struct util_region *placement, unsigned int *cmap, int x0, int x2, int offx, int offy, struct util_size *padding, int num, int line)
{

    struct text_font *font = pool_getfont(widget->attributes.weight);
    unsigned int icurrent;

    /* rows are drawn top to bottom, so each row is found from the one drawn before it */
    if ((unsigned int)num < widget->textrow)
    {

        widget->textrow = 0;
        widget->textoffset = 0;

    }

    icurrent = text_getrowstart(font, strpool_getstring(widget->attributes.label), strpool_getcstringlength(widget->attributes.label), widget->textrow, widget->textoffset, num, widget->attributes.wrap, placement->size.w, offx);
    widget->textrow = num;
    widget->textoffset = icurrent;

    if (icurrent < strpool_getcstringlength(widget->attributes.label))
    {

        struct text_rowinfo rowinfo;
        struct util_position offset;

        text_getrowinfo(&rowinfo, font, strpool_getstring(widget->attributes.label), strpool_getcstringlength(widget->attributes.label), widget->attributes.wrap, util_max((int)placement->size.w - ((num) ? 0 : offx), 0), icurrent);

        offset.x = text_getrowx(&rowinfo, widget->attributes.halign, placement->size.w - padding->w * 2 - ((num) ? 0 : offx)) + padding->w + ((num) ? 0 : offx);
        offset.y = text_getrowy(&rowinfo, widget->attributes.valign, placement->size.h - padding->h * 2 - offy) + padding->h + offy + font->lineheight * num;

        if (util_intersects(line, placement->position.y + offset.y, placement->position.y + offset.y + font->lineheight))
        {

            unsigned int mstart = util_max(0, util_min(widget->markstart, widget->markend) - rowinfo.istart);
            unsigned int mend = util_max(0, util_max(widget->markstart, widget->markend) - rowinfo.istart);

            blit_text(display, font, strpool_getstring(widget->attributes.label) + rowinfo.istart, rowinfo.length, placement->position.x + offset.x, placement->position.y + offset.y, line, x0, x2, mstart, mend, cmap);

        }

    }

}

static void renderbutton(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct util_size padding = util_size(CONFIG_BUTTON_PADDING_WIDTH, CONFIG_BUTTON_PADDING_HEIGHT);
    struct palette *palette = getpalette(PALETTE_BUTTON, getshade(widget));

    blit_frame(display, &widget->placement, line, x0, x2, palette->frame);
    textrender(display, widget, &widget->placement, palette->text, x0, x2, 0, 0, &padding, 0, line);

}

static void rendercheckbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct util_region box = util_region(widget->placement.position.x + ((int)widget->placement.size.w - CONFIG_CHECKBOX_WIDTH) / 2, widget->placement.position.y + ((int)widget->placement.size.h - CONFIG_CHECKBOX_HEIGHT) / 2, CONFIG_CHECKBOX_WIDTH, CONFIG_CHECKBOX_HEIGHT);
    struct palette *palette = getpalette(PALETTE_CHECKBOX, getshade(widget));

    blit_frame(display, &box, line, x0, x2, palette->frame);

    if (widget->attributes.checked)
        blit_iconx(display, &box, line, x0, x2, palette->icon);

}

static void renderfill(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    blit_line(display, widget->attributes.color, x0, x2);

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

    switch (widget->attributes.mimetype)
    {

    case ATTR_MIMETYPE_PCX:
        if (widget->resource)
        {

            struct pool_pcxresource *resource = widget->resource;

            /* the image covers its placement, which its parent's flow stretches or leaves at the image's own size */
            blit_pcx(display, resource, line, widget->placement.position.x, widget->placement.position.y, widget->placement.size.w, widget->placement.size.h, x0, x2);

        }

        break;

    }

}

static void renderlayout(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

}

static void renderswatch(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    int left = widget->placement.position.x + CONFIG_FRAME_WIDTH;
    int right = widget->placement.position.x + widget->placement.size.w - CONFIG_FRAME_WIDTH;
    int top = widget->placement.position.y + CONFIG_FRAME_HEIGHT;
    int bottom = widget->placement.position.y + widget->placement.size.h - CONFIG_FRAME_HEIGHT;

    blit_frame(display, &widget->placement, line, x0, x2, getpalette(PALETTE_SWATCH, SHADE_NORMAL)->frame);

    if (line >= top && line < bottom)
        blit_alphaline(display, widget->attributes.color, util_max(x0, left), util_min(x2, right));

}

static void renderthumb(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    blit_frame(display, &widget->thumb, line, x0, x2, getpalette(PALETTE_BUTTON, SHADE_NORMAL)->frame);

}

static void renderbox(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct util_region rcursor = util_region(widget->cursorposition.x, widget->cursorposition.y, widget->cursorsize.w, widget->cursorsize.h);
    unsigned int shade = getshade(widget);

    /* only a box to type in shows hover and focus */
    if (widget->attributes.mode != ATTR_MODE_EDIT && shade != SHADE_DISABLED)
        shade = SHADE_NORMAL;

    blit_frame(display, &widget->placement, line, x0, x2, getpalette(PALETTE_BOX, shade)->frame);

    if (widget->state == WIDGET_STATE_FOCUS && (widget->attributes.mode == ATTR_MODE_EDIT || widget->appcursor))
        blit_iconcursor(display, &rcursor, line, x0, x2, getpalette(PALETTE_TEXT, SHADE_NORMAL)->text);

}

static void renderpanel(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    blit_frame(display, &widget->placement, line, x0, x2, getpalette(PALETTE_PANEL, SHADE_NORMAL)->frame);

}

static void renderselect(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct util_size padding = util_size(CONFIG_SELECT_PADDING_WIDTH, CONFIG_SELECT_PADDING_HEIGHT);
    struct util_region rarrow = util_region(widget->placement.position.x + widget->placement.size.w - padding.w - widget->placement.size.h / 2, widget->placement.position.y, widget->placement.size.h, widget->placement.size.h);
    struct palette *palette = getpalette(PALETTE_SELECT, getshade(widget));

    blit_frame(display, &widget->placement, line, x0, x2, palette->frame);
    blit_iconarrowdown(display, &rarrow, line, x0, x2, palette->icon);
    textrender(display, widget, &widget->placement, palette->text, x0, x2, 0, 0, &padding, 0, line);

}

static void rendertext(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct text_font *font = pool_getfont(widget->attributes.weight);
    unsigned int *cmaptext = getpalette((widget_islink(widget)) ? PALETTE_LINK : PALETTE_TEXT, SHADE_NORMAL)->text;
    int top = 0;
    unsigned int rownum;

    if (widget->attributes.valign != ATTR_VALIGN_TOP)
    {

        struct text_rowinfo rowinfo;

        if (text_getrowinfo(&rowinfo, font, strpool_getstring(widget->attributes.label), strpool_getcstringlength(widget->attributes.label), widget->attributes.wrap, widget->placement.size.w - widget->rowstart.x, 0))
            top = text_getrowy(&rowinfo, widget->attributes.valign, widget->placement.size.h);

    }

    if (line < widget->placement.position.y + top)
        return;

    rownum = (line - widget->placement.position.y - top) / font->lineheight;

    textrender(display, widget, &widget->placement, cmaptext, x0, x2, widget->rowstart.x, 0, &zerosize, rownum, line);

}

static void renderitem(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    unsigned int *cmapbody = getpalette(PALETTE_ITEM, (widget->selected) ? SHADE_FOCUS : getshade(widget))->frame;

    blit_frame(display, &widget->placement, line, x0, x2, cmapbody);

}

static struct util_size measurewindow(struct widget *widget, struct util_size *limit, unsigned int minimum)
{

    struct util_size inner = util_size_sub(limit, 0, CONFIG_WINDOW_BUTTON_HEIGHT);
    struct util_size csize;

    /* a spanning widget only takes leftover space when placed, but it must fit when the window is measured */
    measurespans = 1;
    measureminimum = minimum;
    csize = childrengetsize(widget, &inner);
    measurespans = 0;
    measureminimum = 0;

    return util_size(csize.w, csize.h + CONFIG_WINDOW_BUTTON_HEIGHT);

}

static unsigned int titlewidth(struct widget *widget, struct util_size *limit)
{

    struct text_info title = text_info(pool_getfont(ATTR_WEIGHT_NORMAL), strpool_getstring(widget->attributes.label), strpool_getcstringlength(widget->attributes.label), ATTR_WRAP_NONE, limit->w, 0);

    return title.width + CONFIG_WINDOW_BUTTON_WIDTH * 4;

}

struct util_size render_getwindowsize(struct widget *widget, struct util_size *limit)
{

    struct util_size size = measurewindow(widget, limit, 0);

    return util_size(util_clamp(util_max(size.w, titlewidth(widget, limit)), 0, limit->w), util_clamp(size.h, 0, limit->h));

}

struct util_size render_getwindowminsize(struct widget *widget, struct util_size *limit)
{

    struct util_size size = measurewindow(widget, limit, 1);

    return util_size(util_clamp(util_max(size.w, titlewidth(widget, limit)), 0, limit->w), util_clamp(size.h, 0, limit->h));

}

struct util_region render_getwindowbutton(struct widget *widget, unsigned int button)
{

    switch (button)
    {

    case RENDER_WINDOWBUTTON_MENU:
        return util_region(widget->placement.position.x, widget->placement.position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);

    case RENDER_WINDOWBUTTON_MINIMIZE:
        return util_region(widget->placement.position.x + CONFIG_WINDOW_BUTTON_WIDTH, widget->placement.position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);

    case RENDER_WINDOWBUTTON_CLOSE:
        return util_region(widget->placement.position.x + widget->placement.size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->placement.position.y, CONFIG_WINDOW_BUTTON_WIDTH, CONFIG_WINDOW_BUTTON_HEIGHT);

    }

    return util_region(widget->placement.position.x + CONFIG_WINDOW_BUTTON_WIDTH * 2, widget->placement.position.y, widget->placement.size.w - CONFIG_WINDOW_BUTTON_WIDTH * 3, CONFIG_WINDOW_BUTTON_HEIGHT);

}

static void renderwindow(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    struct util_region rhamburger = render_getwindowbutton(widget, RENDER_WINDOWBUTTON_MENU);
    struct util_region rminimize = render_getwindowbutton(widget, RENDER_WINDOWBUTTON_MINIMIZE);
    struct util_region rtitle = render_getwindowbutton(widget, RENDER_WINDOWBUTTON_TITLE);
    struct util_region rclose = render_getwindowbutton(widget, RENDER_WINDOWBUTTON_CLOSE);
    unsigned int onhamburger = util_region_intersects(&rhamburger, mouse.x, mouse.y);
    unsigned int onminimize = util_region_intersects(&rminimize, mouse.x, mouse.y);
    unsigned int onclose = util_region_intersects(&rclose, mouse.x, mouse.y);

    struct palette *normal = getpalette(PALETTE_WINDOW, SHADE_NORMAL);
    struct palette *hover = getpalette(PALETTE_WINDOW, SHADE_HOVER);

    blit_frame(display, &rhamburger, line, x0, x2, normal->frame);
    blit_frame(display, &rminimize, line, x0, x2, normal->frame);
    blit_frame(display, &rtitle, line, x0, x2, normal->frame);
    blit_frame(display, &rclose, line, x0, x2, normal->frame);
    textrender(display, widget, &rtitle, normal->text, x0, x2, 0, 0, &zerosize, 0, line);
    blit_iconhamburger(display, &rhamburger, line, x0, x2, (onhamburger) ? hover->icon : normal->icon);
    blit_iconminimize(display, &rminimize, line, x0, x2, (onminimize) ? hover->icon : normal->icon);
    blit_iconx(display, &rclose, line, x0, x2, (onclose) ? hover->icon : normal->icon);

}

static void damagemouse(void)
{

    struct util_region region = blit_getmouseregion(mousetype, mouse.x, mouse.y);

    render_damage(region.position.x, region.position.y, region.position.x + region.size.w, region.position.y + region.size.h);

}

void render_setmouse(int x, int y, unsigned int type)
{

    struct list_item *current = 0;

    if (x == mouse.x && y == mouse.y && type == mousetype)
        return;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;
        unsigned int button;

        if (widget->type != WIDGET_TYPE_WINDOW)
            continue;

        for (button = RENDER_WINDOWBUTTON_MENU; button <= RENDER_WINDOWBUTTON_CLOSE; button++)
        {

            struct util_region region = render_getwindowbutton(widget, button);

            if (util_region_intersects(&region, mouse.x, mouse.y) != util_region_intersects(&region, x, y))
                render_damage(region.position.x, region.position.y, region.position.x + region.size.w, region.position.y + region.size.h);

        }

    }

    damagemouse();

    mouse.x = x;
    mouse.y = y;
    mousetype = type;

    damagemouse();

}

void render_place(struct widget *widget, struct util_region *placement)
{

    place(widget, placement, placement);

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

    static struct widget *damaged[POOL_WIDGETS];
    struct list_item *current = 0;
    unsigned int ndamaged = 0;
    int line;

    if (!hasdamage)
        return;

    area.position0.x = util_clamp(area.position0.x, 0, display->region.size.w);
    area.position0.y = util_clamp(area.position0.y, 0, display->region.size.h);
    area.position2.x = util_clamp(area.position2.x, 0, display->region.size.w);
    area.position2.y = util_clamp(area.position2.y, 0, display->region.size.h);

    while ((current = pool_next(current)) && ndamaged < POOL_WIDGETS)
    {

        struct widget *widget = current->data;
        struct util_region visible = util_region_intersection(&widget->placement, &widget->clip);

        if (visible.position.x < area.position2.x && visible.position.x + (int)visible.size.w > area.position0.x && visible.position.y < area.position2.y && visible.position.y + (int)visible.size.h > area.position0.y)
            damaged[ndamaged++] = widget;

    }

    for (line = area.position0.y; line < area.position2.y; line++)
    {

        unsigned int i;

        for (i = 0; i < ndamaged; i++)
        {

            struct widget *widget = damaged[i];

            if (widget_intersectsy(widget, line))
            {

                int x0 = util_max(util_max(widget->placement.position.x, widget->clip.position.x), area.position0.x);
                int x2 = util_min(util_min(widget->placement.position.x + widget->placement.size.w, widget->clip.position.x + widget->clip.size.w), area.position2.x);

                calls[widget->type].render(display, widget, line, x0, x2);

                if (widget_isscrollable(widget))
                    renderthumb(display, widget, line, x0, x2);

            }

        }

        blit_mouse(display, mousetype, mouse.x, mouse.y, line, area.position0.x, area.position2.x, getpalette(PALETTE_MOUSE, SHADE_NORMAL)->frame);
        blit(display, line, area.position0.x, area.position2.x);

    }

}

static void setupcall(unsigned int type, struct util_size (*getsize)(struct widget *widget, struct util_size *limit, struct util_position *rowstart), void (*place)(struct widget *widget), void (*render)(struct blit_display *display, struct widget *widget, int line, int x0, int x2))
{

    struct calls *call = &calls[type];

    call->getsize = getsize;
    call->place = place;
    call->render = render;

}

void render_init(void)
{

    unsigned int palette;
    unsigned int shade;

    for (palette = 0; palette < NUM_PALETTES; palette++)
    {

        for (shade = 0; shade < NUM_SHADES; shade++)
        {

            char name[64];

            cstring_write_fmt(name, 64, 0, "%s-%s-shadow\\0", palettenames[palette], shadenames[shade]);
            palettes[palette][shade].frame = cmap_get(name);
            cstring_write_fmt(name, 64, 0, "%s-%s-text\\0", palettenames[palette], shadenames[shade]);
            palettes[palette][shade].text = cmap_get(name);
            cstring_write_fmt(name, 64, 0, "%s-%s-icon\\0", palettenames[palette], shadenames[shade]);
            palettes[palette][shade].icon = cmap_get(name);

        }

    }

    setupcall(WIDGET_TYPE_BOX, getsizebox, placebox, renderbox);
    setupcall(WIDGET_TYPE_BUTTON, getsizebutton, placebutton, renderbutton);
    setupcall(WIDGET_TYPE_CHECKBOX, getsizecheckbox, placecheckbox, rendercheckbox);
    setupcall(WIDGET_TYPE_FILL, getsizefill, placefill, renderfill);
    setupcall(WIDGET_TYPE_IMAGE, getsizeimage, placeimage, renderimage);
    setupcall(WIDGET_TYPE_ITEM, getsizeitem, placeitem, renderitem);
    setupcall(WIDGET_TYPE_SWATCH, getsizeswatch, placeswatch, renderswatch);
    setupcall(WIDGET_TYPE_LAYOUT, getsizelayout, placelayout, renderlayout);
    setupcall(WIDGET_TYPE_PANEL, getsizepanel, placepanel, renderpanel);
    setupcall(WIDGET_TYPE_SELECT, getsizeselect, placeselect, renderselect);
    setupcall(WIDGET_TYPE_TEXT, getsizetext, placetext, rendertext);
    setupcall(WIDGET_TYPE_WINDOW, getsizewindow, placewindow, renderwindow);

}

