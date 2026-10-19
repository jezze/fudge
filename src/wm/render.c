#include <fudge.h>
#include <abi.h>
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

#define INFINITY                        0xFFFF
#define DIRECTION_NONE                  0
#define DIRECTION_HORIZONTAL            1
#define DIRECTION_VERTICAL              2
#define STRETCH_NONE                    0
#define STRETCH_HORIZONTAL              1
#define STRETCH_VERTICAL                2
#define STRETCH_BOTH                    3

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

struct palette
{

    unsigned int *frame;
    unsigned int *text;
    unsigned int *icon;

};

static struct util_position zeroposition;
static struct util_size zerosize;
static unsigned int measurespans;
static unsigned int measureminimum;
static unsigned int measureclipped;
static struct util_position mouse;
static unsigned int mousetype;
static struct calls calls[32];
static struct palette palettes[NUM_PALETTES][NUM_SHADES];
static struct widget *hoverwidget;
static struct widget *focusedwidget;
static struct widget *focusedwindow;

static unsigned int isfocused(struct widget *widget)
{

    return widget == focusedwidget || widget == focusedwindow;

}

static unsigned int getshade(struct widget *widget)
{

    if (widget->attributes.mode == ATTR_MODE_DISABLED)
        return SHADE_DISABLED;

    if (isfocused(widget))
        return SHADE_FOCUS;

    if (widget == hoverwidget)
        return SHADE_HOVER;

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

static unsigned int isscrollablex(struct widget *widget)
{

    return widget_isscrollable(widget) || widget_iseditable(widget);

}

static struct util_region getplacement(struct widget *widget, struct widget *child, struct util_position *offset, struct util_position *rowstart, struct util_size *span)
{

    unsigned int stretch = getstretch(widget->attributes.flow);
    struct util_size climit = util_size_sub(&widget->cplacement.size, offset->x, offset->y);
    struct util_region placement;
    struct util_size size;

    placement.position = util_position_add(&widget->cplacement.position, offset->x, offset->y);
    placement.size = size = calls[child->type].getsize(child, &climit, rowstart);

    if (widget_isscrollable(child))
        placement.size.w = size.w = util_min(size.w + CONFIG_SCROLL_WIDTH, climit.w);

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

    if (isscrollablex(widget))
        placement.size.w = util_max(placement.size.w, size.w);

    return placement;

}

static void place(struct widget *widget, struct util_region *placement, struct util_region *clip);

struct placeinfo
{

    unsigned int spans;
    unsigned int width;
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
    info.width = 0;
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
        info.width = util_max(info.width, cplacement.size.w + offset.x);
        info.total.w = util_clamp(util_max(info.total.w, cplacement.size.w + offset.x), 0, widget->cplacement.size.w);
        info.total.h = util_clamp(util_max(info.total.h, cplacement.size.h + offset.y), 0, widget->cplacement.size.h);

        advance(widget, child, direction, &cplacement.size, &offset, &rowstart, &pending);

    }

    info.freesize = util_size_sub(&widget->cplacement.size, info.total.w, info.total.h);

    if (info.spans)
        info.span = util_size(info.freesize.w / info.spans, info.freesize.h / info.spans);

    return info;

}

struct util_region render_getcursor(struct widget *widget, unsigned int index)
{

    unsigned int length;
    struct widget *text = pool_gettextat(widget, index, &length);

    if (text)
    {

        struct text_font *font = pool_getfont(text->attributes.weight);
        char *label = strpool_getstring(text->attributes.label);
        unsigned int total = strpool_getcstringlength(text->attributes.label);
        unsigned int maxw = widget->cplacement.size.w - (text->placement.position.x - widget->cplacement.position.x);
        struct text_info info = text_info(font, label, length, text->attributes.wrap, maxw, text->rowstart.x);
        unsigned int num = info.lastrow.y / font->lineheight;
        int offx = (num) ? 0 : text->rowstart.x;
        struct text_rowinfo rowinfo;
        int x;

        rowinfo.width = 0;
        rowinfo.lineheight = font->lineheight;

        text_getrowinfo(&rowinfo, font, label, total, text->attributes.wrap, util_max((int)text->placement.size.w - offx, 0), text_getrowstart(font, label, total, 0, 0, num, text->attributes.wrap, text->placement.size.w, text->rowstart.x));

        x = text_getrowx(&rowinfo, text->attributes.halign, text->placement.size.w - offx) + offx + (info.lastrow.x - offx);

        return util_region(text->placement.position.x + x, text->placement.position.y + info.lastrow.y, 2, font->lineheight);

    }

    return util_region(0, 0, 0, 0);

}

static void placecursor(struct widget *widget)
{

    struct util_region cursor = render_getcursor(widget, widget->attributes.cursor);

    widget->cursorposition = cursor.position;
    widget->cursorsize = cursor.size;

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

            int left = widget->cplacement.position.x + widget->scroll.x - widget->placement.position.x;
            int top = widget->cplacement.position.y + widget->scroll.y - widget->placement.position.y;

            content = util_region(widget->placement.position.x + left, widget->placement.position.y + top, widget->cplacement.size.w, util_max((int)widget->placement.size.h - top * 2, 0));

        }

        break;

    }

    widget->cclip = util_region_intersection(&content, &widget->cclip);

}

static void placethumb(struct widget *widget, int view)
{

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

    widget->cplacement = widget->placement;

    if (calls[widget->type].place)
        calls[widget->type].place(widget);

    if (isscrollablex(widget))
        widget->cplacement.position.x -= widget->scroll.x;

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

    if (isscrollablex(widget))
    {

        struct util_region view = widget_getview(widget);
        struct util_position scroll = widget->scroll;
        int scrollmaxx = (info.width > view.size.w) ? (int)(info.width + widget->cursorsize.w - view.size.w) : 0;

        if (widget->followcursor && widget->cursorsize.h)
        {

            struct util_region cursor = util_region(widget->cursorposition.x, widget->cursorposition.y, widget->cursorsize.w, widget->cursorsize.h);

            widget_scrollto(widget, &cursor);

        }

        widget->followcursor = 0;
        widget->scrollmax = (widget_isscrollable(widget)) ? util_max(0, (int)info.total.h - (int)view.size.h) : 0;
        widget->scroll.x = util_clamp(widget->scroll.x, 0, scrollmaxx);
        widget->scroll.y = util_clamp(widget->scroll.y, 0, widget->scrollmax);

        if (widget->scroll.x != scroll.x || widget->scroll.y != scroll.y)
        {

            place(widget, placement, clip);

            return;

        }

        if (widget_isscrollable(widget))
            placethumb(widget, view.size.h);

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

    if ((measureminimum && widget->attributes.wrap != ATTR_WRAP_NONE) || (measureclipped && widget->attributes.wrap == ATTR_WRAP_NONE))
    {

        struct text_info narrow = text_info(font, label, length, (widget->attributes.wrap == ATTR_WRAP_NONE) ? ATTR_WRAP_CHAR : widget->attributes.wrap, 1, 0);

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

static struct util_size getsizechildren(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    return childrengetsize(widget, limit);

}

static struct util_size getsizefixed(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    return widget->size;

}

static struct util_size getsizeswatch(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    unsigned int side = pool_getfont(widget->attributes.weight)->lineheight + CONFIG_SWATCH_PADDING * 2;

    return util_size(side, side);

}

static struct util_size getsizebox(struct widget *widget, struct util_size *limit, struct util_position *rowstart)
{

    struct util_size border = util_size(CONFIG_FRAME_WIDTH * 2, CONFIG_FRAME_HEIGHT * 2);
    unsigned int clipped = measureclipped;
    struct util_size csize;

    measureclipped = clipped || widget_isscrollable(widget);
    csize = childrengetsize(widget, limit);
    measureclipped = clipped;

    if (widget->attributes.mode == ATTR_MODE_EDIT)
        csize.h = util_max(csize.h, pool_getfont(widget->attributes.weight)->lineheight + widget->attributes.padding * 2);

    return util_size_union(&csize, &border);

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

static void placebox(struct widget *widget)
{

    struct util_region inner = util_region(widget->placement.position.x + CONFIG_FRAME_WIDTH, widget->placement.position.y + CONFIG_FRAME_HEIGHT, widget->placement.size.w - CONFIG_FRAME_WIDTH * 2, widget->placement.size.h - CONFIG_FRAME_HEIGHT * 2);

    widget->cclip = util_region_intersection(&inner, &widget->clip);
    widget->clip = util_region_intersection(&widget->placement, &widget->clip);
    widget->cplacement = inner;

}

static void placeselect(struct widget *widget)
{

    int top = widget->placement.position.y + widget->placement.size.h;
    int bottom = widget->clip.position.y + widget->clip.size.h;

    widget->cplacement = util_region(widget->placement.position.x, top, INFINITY, util_max(bottom - top, 0));

    if (!isfocused(widget))
        widget->clip = util_region_intersection(&widget->placement, &widget->clip);

    widget->cclip = widget->clip;

}

static void placewindow(struct widget *widget)
{

    widget->clip = widget->placement;
    widget->cclip = widget->clip;
    widget->cplacement = util_region(widget->placement.position.x, widget->placement.position.y + CONFIG_WINDOW_BUTTON_HEIGHT, widget->placement.size.w, widget->placement.size.h - CONFIG_WINDOW_BUTTON_HEIGHT);

}

static void textrender(struct blit_display *display, struct widget *widget, struct util_region *placement, unsigned int *cmap, int x0, int x2, int offx, int offy, struct util_size *padding, int num, int line)
{

    struct text_font *font = pool_getfont(widget->attributes.weight);
    unsigned int icurrent;

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

static void renderimage(struct blit_display *display, struct widget *widget, int line, int x0, int x2)
{

    switch (widget->attributes.mimetype)
    {

    case ATTR_MIMETYPE_RAW:
        if (widget->resource)
        {

            struct pool_rawresource *resource = widget->resource;

            blit_raw(display, resource->data, widget->attributes.width, widget->attributes.height, line, widget->placement.position.x, widget->placement.position.y, widget->placement.size.w, widget->placement.size.h, x0, x2);

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

    struct util_region cursor = util_region(widget->cursorposition.x, widget->cursorposition.y, widget->cursorsize.w, widget->cursorsize.h);
    struct util_region inside = util_region(widget->placement.position.x + CONFIG_FRAME_WIDTH, widget->cclip.position.y, util_max((int)widget->placement.size.w - CONFIG_FRAME_WIDTH * 2, 0), widget->cclip.size.h);
    struct util_region rcursor = util_region_intersection(&cursor, &inside);
    unsigned int shade = getshade(widget);

    if (widget->attributes.mode != ATTR_MODE_EDIT && shade != SHADE_DISABLED)
        shade = SHADE_NORMAL;

    blit_frame(display, &widget->placement, line, x0, x2, getpalette(PALETTE_BOX, shade)->frame);

    if (isfocused(widget) && (widget->attributes.mode == ATTR_MODE_EDIT || widget->appcursor))
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
    int textx2 = util_min(x2, widget->placement.position.x + (int)widget->placement.size.w - (int)padding.w * 2);

    blit_frame(display, &widget->placement, line, x0, x2, palette->frame);
    blit_iconarrowdown(display, &rarrow, line, x0, x2, palette->icon);

    if (textx2 > x0)
        textrender(display, widget, &widget->placement, palette->text, x0, textx2, 0, 0, &padding, 0, line);

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

void render_setstate(struct widget *hover, struct widget *focus, struct widget *window)
{

    hoverwidget = hover;
    focusedwidget = focus;
    focusedwindow = window;

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

unsigned int render_update(struct blit_display *display, struct util_region *updated)
{

    static struct widget *damaged[POOL_WIDGETS];
    struct list_item *current = 0;
    unsigned int ndamaged = 0;
    int line;

    if (!hasdamage)
        return 0;

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

    *updated = util_region(area.position0.x, area.position0.y, area.position2.x - area.position0.x, area.position2.y - area.position0.y);

    return 1;

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

            unsigned int *colors = cmap_get(palette, shade);

            palettes[palette][shade].frame = colors + PART_SHADOW;
            palettes[palette][shade].text = colors + PART_TEXT;
            palettes[palette][shade].icon = colors + PART_ICON;

        }

    }

    setupcall(WIDGET_TYPE_BOX, getsizebox, placebox, renderbox);
    setupcall(WIDGET_TYPE_BUTTON, getsizebutton, 0, renderbutton);
    setupcall(WIDGET_TYPE_CHECKBOX, getsizecheckbox, 0, rendercheckbox);
    setupcall(WIDGET_TYPE_FILL, getsizefixed, 0, renderfill);
    setupcall(WIDGET_TYPE_IMAGE, getsizefixed, 0, renderimage);
    setupcall(WIDGET_TYPE_ITEM, getsizechildren, 0, renderitem);
    setupcall(WIDGET_TYPE_SWATCH, getsizeswatch, 0, renderswatch);
    setupcall(WIDGET_TYPE_LAYOUT, getsizechildren, 0, renderlayout);
    setupcall(WIDGET_TYPE_PANEL, getsizechildren, 0, renderpanel);
    setupcall(WIDGET_TYPE_SELECT, getsizeselect, placeselect, renderselect);
    setupcall(WIDGET_TYPE_TEXT, getsizetext, 0, rendertext);
    setupcall(WIDGET_TYPE_WINDOW, getsizefixed, placewindow, renderwindow);
}

