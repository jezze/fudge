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
#include "render.h"
#include "parser.h"
#include "cmap.h"
#include "bind.h"

#define STATE_NORMAL        0
#define STATE_GRABBED       1
#define TEXTBOX_SIZE        4096
#define LAYOUT_DEFAULT      0
#define LAYOUT_TILING       1
#define NUM_FOCUSABLE       256
#define ROW_FAR             0x40000000

struct state
{

    unsigned int state;
    struct util_position mouseposition;
    struct util_position mousemovement;
    struct util_position mousepressed;
    struct util_position mousereleased;
    unsigned int mousebuttonleft;
    unsigned int mousebuttonright;
    struct widget *rootwidget;
    struct widget *hoverwidget;
    struct widget *focusedwindow;
    struct widget *focusedwidget;
    struct keys keys;
    unsigned int layout;
    unsigned int windows;
    struct event_videoconf videoconf;
    struct widget *draggedthumb;
    unsigned int video;
    struct util_position markpoint;
    struct util_position markscroll;
    int scrollgrab;

};

static struct blit_display display;
static struct state state;
static unsigned int linebuffer[3840];
static char *fontn[] = {
    "initrd:data/font/ter-112n.pcf",
    "initrd:data/font/ter-114n.pcf",
    "initrd:data/font/ter-116n.pcf",
    "initrd:data/font/ter-118n.pcf",
};
static char *fontb[] = {
    "initrd:data/font/ter-112b.pcf",
    "initrd:data/font/ter-114b.pcf",
    "initrd:data/font/ter-116b.pcf",
    "initrd:data/font/ter-118b.pcf",
};

static void setupvideo(unsigned int video)
{

    unsigned char black[768];

    buffer_clear(black, 768);
    channel_send(0, video, EVENT_VIDEOCMAP, 768, &black);
    channel_send(0, video, EVENT_VIDEOCONF, sizeof (struct event_videoconf), &state.videoconf);
    channel_send(1, video, EVENT_INFO, 0, 0);
    channel_wait(1, video, EVENT_VIDEOINFO, 0, 0);

}

static unsigned int getword(char *data, unsigned int count, unsigned int index, char **word)
{

    unsigned int start = 0;
    unsigned int i;

    for (i = 0; i <= count; i++)
    {

        if (i == count || data[i] == ' ' || data[i] == '\n')
        {

            if (i > start && !index--)
            {

                *word = data + start;

                return i - start;

            }

            start = i + 1;

        }

    }

    return 0;

}

static unsigned int getnumber(char *data, unsigned int count, unsigned int index, unsigned int base, unsigned int fallback)
{

    char *word;
    unsigned int length = getword(data, count, index, &word);

    return (length) ? cstring_read_value(word, length, base) : fallback;

}

static void loadconfig(char *config)
{

    static char data[0x4000];
    unsigned int count;
    unsigned int length;
    char *line;

    count = system_feed(0, 0, data, 0x4000, "mq -query .screen.resolution -format \"{width} {height} {bpp}\" %s", config);
    state.videoconf.width = getnumber(data, count, 0, 10, state.videoconf.width);
    state.videoconf.height = getnumber(data, count, 1, 10, state.videoconf.height);
    state.videoconf.bpp = getnumber(data, count, 2, 10, state.videoconf.bpp);

    count = system_feed(0, 0, data, 0x4000, "mq -query .general.layout %s", config);
    state.layout = (count >= 6 && buffer_match(data, "tiling", 6)) ? LAYOUT_TILING : LAYOUT_DEFAULT;

    count = system_feed(0, 0, data, 0x4000, "mq -query .keyboard.layout %s", config);
    keys_init(&state.keys, KEYS_LAYOUT_QWERTY_US, (count >= 9 && buffer_match(data, "qwerty_SE", 9)) ? KEYS_MAP_SE : KEYS_MAP_US);

    count = system_feed(0, 0, data, 0x4000, "mq -query .colors -format \"{name} {value}\" %s", config);

    for (line = data; line < data + count; line += length + 1)
    {

        unsigned int space;
        char name[64];

        length = buffer_findbyte(line, data + count - line, '\n');
        space = buffer_findbyte(line, length, ' ');

        if (space < length && space < 64)
        {

            buffer_write(name, 64, line, space, 0);
            name[space] = '\0';
            cmap_set(name, cstring_read_value(line + space + 1, length - space - 1, 16));

        }

    }

    count = system_feed(0, 0, data, 0x4000, "mq -query .bindings -format \"{key} {mod} {action} {command}\" %s", config);

    for (line = data; line < data + count; line += length + 1)
    {

        length = buffer_findbyte(line, data + count - line, '\n');

        bind_add(line, length);

    }

}

static unsigned int intersectsrows(struct widget *widget, int x, int y)
{

    struct text_font *font = pool_getfont(widget->attributes.weight);
    int rx = x - widget->placement.position.x;
    int ry = y - widget->placement.position.y;

    if (widget->attributes.display != ATTR_DISPLAY_INLINE)
        return 1;

    if (ry < (int)font->lineheight && rx < widget->rowstart.x)
        return 0;

    if (ry >= widget->rowstop.y && rx >= widget->rowstop.x)
        return 0;

    return 1;

}

static struct widget *getinteractivewidgetat(int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_prev(current)))
    {

        struct widget *child = current->data;

        if (widget_isinteractive(child) && widget_intersects(child, x, y) && (!widget_islink(child) || intersectsrows(child, x, y)))
            return child;

    }

    return 0;

}

static struct widget *getparent(struct widget *widget)
{

    return (widget->attributes.in) ? pool_getwidgetbyid(widget->source, strpool_getstring(widget->attributes.in)) : 0;

}

static struct widget *getitem(struct widget *widget)
{

    struct widget *parent;

    if (widget->type == WIDGET_TYPE_ITEM)
        return widget;

    parent = getparent(widget);

    return (parent && parent->type == WIDGET_TYPE_ITEM) ? parent : 0;

}

static struct widget *getselecteditem(struct widget *widget)
{

    struct list_item *current = 0;
    struct widget *item = getitem(widget);

    if (item)
        return item;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_ITEM && child->anchor)
            return child;

    }

    return 0;

}

static struct widget *gethoverwidgetat(int x, int y)
{

    struct widget *widget = getinteractivewidgetat(x, y);
    struct widget *item = (widget) ? getitem(widget) : 0;

    return (item) ? item : widget;

}

static unsigned int getmousetype(int x, int y)
{

    struct widget *widget = getinteractivewidgetat(x, y);
    unsigned int button;

    if (!widget)
        return BLIT_MOUSE_ARROW;

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
    case WIDGET_TYPE_CHECKBOX:
    case WIDGET_TYPE_ITEM:
    case WIDGET_TYPE_SELECT:
        return BLIT_MOUSE_HAND;

    case WIDGET_TYPE_TEXT:
        if (widget_islink(widget))
            return BLIT_MOUSE_HAND;

        break;

    case WIDGET_TYPE_BOX:
        if (widget->attributes.mode == ATTR_MODE_EDIT)
            return BLIT_MOUSE_TEXT;

        break;

    case WIDGET_TYPE_WINDOW:
        for (button = RENDER_WINDOWBUTTON_MENU; button <= RENDER_WINDOWBUTTON_CLOSE; button++)
        {

            struct util_region region = render_getwindowbutton(widget, button);

            if (util_region_intersects(&region, x, y))
                return BLIT_MOUSE_HAND;

        }

        break;

    }

    return BLIT_MOUSE_ARROW;

}

static struct widget *getscrollablewidgetat(int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_prev(current)))
    {

        struct widget *child = current->data;

        if (widget_isscrollable(child) && widget_intersects(child, x, y))
            return child;

    }

    return 0;

}

static struct widget *getwidgetoftypeat(unsigned int type, int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_prev(current)))
    {

        struct widget *child = current->data;

        if (child->type == type && widget_intersects(child, x, y))
            return child;

    }

    return 0;

}

static void damage(struct widget *widget)
{

    int x0 = util_clamp(widget->position.x, 0, display.region.size.w);
    int y0 = util_clamp(widget->position.y, 0, display.region.size.h);
    int x2 = util_clamp(widget->position.x + widget->size.w, 0, display.region.size.w);
    int y2 = util_clamp(widget->position.y + widget->size.h, 0, display.region.size.h);

    render_damage(x0, y0, x2, y2);

    x0 = util_clamp(widget->placement.position.x, 0, display.region.size.w);
    y0 = util_clamp(widget->placement.position.y, 0, display.region.size.h);
    x2 = util_clamp(widget->placement.position.x + widget->placement.size.w, 0, display.region.size.w);
    y2 = util_clamp(widget->placement.position.y + widget->placement.size.h, 0, display.region.size.h);

    render_damage(x0, y0, x2, y2);

}

static void damageall(struct widget *widget)
{

    struct list_item *current = 0;

    damage(widget);

    /* everything in a window is drawn inside it, so there is nothing more to damage */
    if (widget->type == WIDGET_TYPE_WINDOW)
        return;

    while ((current = pool_nextin(current, widget)))
        damageall(current->data);

}

static void shiftregion(struct util_region *region, int x, int y)
{

    region->position.x += x;
    region->position.y += y;

}

/* moves a window and everything placed in it without a new layout, which is only possible when it is the only window of its program; says whether it did */
static unsigned int shiftwindow(struct widget *window, int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, window->source)))
    {

        struct widget *widget = current->data;

        if (widget->type == WIDGET_TYPE_WINDOW && widget != window)
            return 0;

    }

    while ((current = pool_nextsource(current, window->source)))
    {

        struct widget *widget = current->data;

        shiftregion(&widget->placement, x, y);
        shiftregion(&widget->cplacement, x, y);
        shiftregion(&widget->clip, x, y);
        shiftregion(&widget->cclip, x, y);
        shiftregion(&widget->thumb, x, y);

        widget->cursorposition.x += x;
        widget->cursorposition.y += y;

    }

    return 1;

}

/* a window covers everything in it, so damaging its own area is enough */
static unsigned int translatewindow(struct widget *window, int x, int y)
{

    unsigned int shifted;

    damage(window);

    window->position.x += x;
    window->position.y += y;
    shifted = window->source && shiftwindow(window, x, y);

    damage(window);

    return shifted;

}

static void scalewidget(struct widget *widget, unsigned int w, unsigned int h)
{

    damageall(widget);

    widget->size.w = w;
    widget->size.h = h;

    damageall(widget);

}

static void scrollwidget(struct widget *widget, int y)
{

    if (widget_isscrollable(widget))
        widget->scroll.y += y;

    damage(widget);

}

static void bump(struct widget *widget)
{

    pool_bump(widget);
    damageall(widget);

}

static void bumpchildren(struct widget *widget)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
        bump(current->data);

}

static void launch(char *command)
{

    system_run(0, "%s &", command);

}

static void sendevent(unsigned int source, unsigned int type, unsigned int action, char *text)
{

    if (source)
    {

        struct {struct event_wmevent wmevent; char data[512];} message;

        message.wmevent.type = type;
        message.wmevent.length = cstring_write_fmt(message.data, 512, 0, "%s%s\\0", strpool_getstring(action), text);

        channel_send(0, source, EVENT_WMEVENT, sizeof (struct event_wmevent) + message.wmevent.length, &message);

    }

    else
    {

        char *cmd = strpool_getstring(action);

        if (buffer_match(cmd, "run=", 4))
            launch(cmd + 4);

    }

}

static void setselected(struct widget *item, unsigned int selected)
{

    if (item->selected != selected)
    {

        item->selected = selected;

        damage(item);

        if (item->attributes.onselect)
            sendevent(item->source, 1, item->attributes.onselect, (selected) ? "1" : "0");

    }

}

static void notifyselection(struct widget *parent)
{

    struct list_item *current = 0;
    unsigned int count = 0;
    char text[16];

    if (!parent || !parent->attributes.onselect)
        return;

    while ((current = pool_nextin(current, parent)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_ITEM && child->selected)
            count++;

    }

    cstring_write_fmt(text, 16, 0, "%u\\0", &count);
    sendevent(parent->source, 1, parent->attributes.onselect, text);

}

static void setanchor(struct widget *item)
{

    struct widget *parent = getparent(item);
    struct list_item *current = 0;

    while (parent && (current = pool_nextin(current, parent)))
    {

        struct widget *child = current->data;

        child->anchor = 0;

    }

    item->anchor = 1;

}

static void selectitem(struct widget *item)
{

    struct widget *parent = getparent(item);
    struct list_item *current = 0;

    while (parent && (current = pool_nextin(current, parent)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_ITEM)
            setselected(child, child == item);

    }

    setanchor(item);
    notifyselection(parent);

}

static void toggleitem(struct widget *item)
{

    setselected(item, !item->selected);
    setanchor(item);
    notifyselection(getparent(item));

}

static void selectrange(struct widget *item)
{

    struct widget *parent = getparent(item);
    struct list_item *current = 0;
    unsigned int inside = 0;

    while (parent && (current = pool_nextin(current, parent)))
    {

        struct widget *child = current->data;

        if (child->anchor)
            break;

    }

    if (!current)
    {

        selectitem(item);

        return;

    }

    current = 0;

    while ((current = pool_nextin(current, parent)))
    {

        struct widget *child = current->data;
        unsigned int edge;

        if (child->type != WIDGET_TYPE_ITEM)
            continue;

        edge = (child == item) + child->anchor;

        if (edge)
        {

            setselected(child, 1);

            inside = !inside && edge == 1;

        }

        else
        {

            setselected(child, inside);

        }

    }

    notifyselection(parent);

}

static void scrollto(struct widget *widget, struct util_region *region)
{

    widget_scrollto(widget, region);
    damage(widget);

}

static void showwidget(struct widget *widget)
{

    struct widget *parent = getparent(widget);

    while (parent && !widget_isscrollable(parent))
        parent = getparent(parent);

    if (parent)
        scrollto(parent, &widget->placement);

}

static unsigned int gettextlength(struct widget *text)
{

    return strpool_getcstringlength(text->attributes.label);

}

static unsigned int gettextoffsetat(struct widget *text, int x, int y)
{

    struct text_font *font = pool_getfont(text->attributes.weight);

    return text_getoffsetat(font, strpool_getstring(text->attributes.label), gettextlength(text), text->attributes.wrap, text->attributes.halign, text->placement.size.w, text->rowstart.x, x - text->placement.position.x, y - text->placement.position.y);

}

static void marktext(struct widget *text, struct util_position *scrolled, unsigned int clear)
{

    text->markstart = (clear) ? 0 : gettextoffsetat(text, state.mousepressed.x - scrolled->x, state.mousepressed.y - scrolled->y);
    text->markend = (clear) ? 0 : gettextoffsetat(text, state.markpoint.x, state.markpoint.y);

}

static void markwidget(struct widget *widget, unsigned int clear)
{

    struct util_position scrolled = util_position(widget->scroll.x - state.markscroll.x, widget->scroll.y - state.markscroll.y);
    struct list_item *current = 0;

    if (widget->type != WIDGET_TYPE_BOX)
        return;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
            marktext(child, &scrolled, clear);

    }

}

/* the texts of a textbox count as one run of characters, so a position is an index into all of them */
static unsigned int getmarkindex(struct widget *textbox, int x, int y)
{

    struct list_item *current = 0;
    unsigned int base = 0;

    while ((current = pool_nextin(current, textbox)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            unsigned int offset = gettextoffsetat(child, x, y);

            if (offset < gettextlength(child))
                return base + offset;

            base += gettextlength(child);

        }

    }

    return base;

}

static void setmarkpoint(struct widget *textbox, unsigned int index)
{

    unsigned int offset;
    struct widget *text = pool_gettextat(textbox, index, &offset);

    if (text)
    {

        struct text_font *font = pool_getfont(text->attributes.weight);
        struct text_info info = text_info(font, strpool_getstring(text->attributes.label), offset, text->attributes.wrap, text->placement.size.w, text->rowstart.x);

        state.markpoint.x = text->placement.position.x + info.lastrow.x;
        state.markpoint.y = text->placement.position.y + info.lastrow.y + font->lineheight / 2;

    }

}

static unsigned int ismarkkey(unsigned int id)
{

    return id == KEYS_KEY_CURSORLEFT || id == KEYS_KEY_CURSORRIGHT || id == KEYS_KEY_CURSORUP || id == KEYS_KEY_CURSORDOWN || id == KEYS_KEY_HOME || id == KEYS_KEY_END;

}

static void extendmark(struct widget *textbox, unsigned int id)
{

    struct text_font *font = pool_getfont(textbox->attributes.weight);
    unsigned int index = getmarkindex(textbox, state.markpoint.x, state.markpoint.y);

    switch (id)
    {

    case KEYS_KEY_CURSORLEFT:
        setmarkpoint(textbox, (index) ? index - 1 : 0);

        break;

    case KEYS_KEY_CURSORRIGHT:
        setmarkpoint(textbox, index + 1);

        break;

    case KEYS_KEY_CURSORUP:
        state.markpoint.y -= font->lineheight;

        break;

    case KEYS_KEY_CURSORDOWN:
        state.markpoint.y += font->lineheight;

        break;

    case KEYS_KEY_HOME:
        setmarkpoint(textbox, getmarkindex(textbox, -ROW_FAR, state.markpoint.y));

        break;

    case KEYS_KEY_END:
        setmarkpoint(textbox, getmarkindex(textbox, ROW_FAR, state.markpoint.y));

        break;

    }

    markwidget(textbox, 0);
    damageall(textbox);

}

static void setfocus(struct widget *widget)
{

    if (state.focusedwidget)
    {

        markwidget(state.focusedwidget, 1);
        widget_setstate(state.focusedwidget, WIDGET_STATE_FOCUSOFF);
        widget_setstate(state.focusedwidget, WIDGET_STATE_NORMAL);
        damageall(state.focusedwidget);

        state.focusedwidget = 0;

    }

    if (widget && widget_setstate(widget, WIDGET_STATE_FOCUS))
    {

        state.focusedwidget = widget;

        damageall(state.focusedwidget);

        if (state.focusedwidget->type == WIDGET_TYPE_SELECT)
        {

            bumpchildren(state.focusedwidget);

        }

    }

}

static void setfocuswindow(struct widget *widget)
{

    if (state.focusedwindow)
    {

        widget_setstate(state.focusedwindow, WIDGET_STATE_FOCUSOFF);
        widget_setstate(state.focusedwindow, WIDGET_STATE_NORMAL);
        damageall(state.focusedwindow);

        state.focusedwindow = 0;

    }

    if (widget && widget_setstate(widget, WIDGET_STATE_FOCUS))
    {

        state.focusedwindow = widget;

        bump(state.focusedwindow);

    }

}

static void sethover(struct widget *widget)
{

    if (state.hoverwidget)
    {

        widget_setstate(state.hoverwidget, WIDGET_STATE_HOVEROFF);
        widget_setstate(state.hoverwidget, WIDGET_STATE_NORMAL);
        damageall(state.hoverwidget);

        state.hoverwidget = 0;

    }

    if (widget && widget_setstate(widget, WIDGET_STATE_HOVER))
    {

        state.hoverwidget = widget;

        damageall(state.hoverwidget);

    }

}

static void placewindows(unsigned int source)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, source)))
    {

        struct widget *widget = current->data;

        if (widget->type == WIDGET_TYPE_WINDOW)
        {

            widget->attributes.display = ATTR_DISPLAY_FIXED;

            if (widget->size.w == 0 && widget->size.h == 0)
            {

                struct util_size limit;

                widget->position.x = display.region.size.w / 8;
                widget->position.y = display.region.size.h / 8;
                limit = util_size(util_max((int)display.region.size.w - widget->position.x * 2, 0), util_max((int)display.region.size.h - widget->position.y * 2, 0));
                widget->size = render_getwindowsize(widget, &limit);
                widget->size.w = util_min(util_max(widget->size.w, display.region.size.w / 2), limit.w);
                widget->size.h = util_min(util_max(widget->size.h, display.region.size.h / 2), limit.h);

                widget->serial = ++state.windows;

                setfocuswindow(widget);
                setfocus(0);

            }

            /* new content can need more room than the window was given, so it grows to fit it */
            else if (state.layout == LAYOUT_DEFAULT)
            {

                struct util_size minimum = render_getwindowminsize(widget, &display.region.size);

                if (widget->size.w < minimum.w || widget->size.h < minimum.h)
                    scalewidget(widget, util_max(widget->size.w, minimum.w), util_max(widget->size.h, minimum.h));

            }

            damageall(widget);

        }

    }

}

static struct widget *getolderwindow(unsigned int serial)
{

    struct list_item *current = 0;
    struct widget *older = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (widget->type == WIDGET_TYPE_WINDOW && widget->state != WIDGET_STATE_DESTROYED && widget->serial < serial && (!older || widget->serial > older->serial))
            older = widget;

    }

    return older;

}

static struct widget *getnewerwindow(unsigned int serial)
{

    struct list_item *current = 0;
    struct widget *newer = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (widget->type == WIDGET_TYPE_WINDOW && widget->state != WIDGET_STATE_DESTROYED && widget->serial > serial && (!newer || widget->serial < newer->serial))
            newer = widget;

    }

    return newer;

}

static void cyclewindow(unsigned int older)
{

    struct widget *window = 0;

    if (state.focusedwindow)
        window = (older) ? getolderwindow(state.focusedwindow->serial) : getnewerwindow(state.focusedwindow->serial);

    if (!window)
        window = (older) ? getolderwindow(state.windows + 1) : getnewerwindow(0);

    if (window)
    {

        setfocuswindow(window);
        setfocus(0);

    }

}

static void swapwindow(void)
{

    struct widget *first = getolderwindow(state.windows + 1);

    if (state.focusedwindow && first && first != state.focusedwindow)
    {

        unsigned int serial = first->serial;

        first->serial = state.focusedwindow->serial;
        state.focusedwindow->serial = serial;

    }

}

static void placewindow(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    if (widget->position.x != x || widget->position.y != y || widget->size.w != w || widget->size.h != h)
    {

        damageall(widget);

        widget->position.x = x;
        widget->position.y = y;
        widget->size.w = w;
        widget->size.h = h;

        damageall(widget);

    }

}

/* the newest window takes the left half below the menu, the others share the right half from top to bottom, newest first */
static void tilewindows(void)
{

    struct widget *menu = pool_getwidgetbyid(0, "menu");
    int top = (menu) ? menu->placement.position.y + (int)menu->placement.size.h : 0;
    unsigned int w = display.region.size.w / 2;
    unsigned int h = util_max((int)display.region.size.h - top, 0);
    struct widget *first = getolderwindow(state.windows + 1);
    struct widget *widget;
    unsigned int count = 0;
    unsigned int i = 0;

    if (!first)
        return;

    placewindow(first, 0, top, w, h);

    for (widget = getolderwindow(first->serial); widget; widget = getolderwindow(widget->serial))
        count++;

    for (widget = getolderwindow(first->serial); widget; widget = getolderwindow(widget->serial), i++)
        placewindow(widget, w, top + h * i / count, display.region.size.w - w, h * (i + 1) / count - h * i / count);

}

static struct widget *gettextchild(struct widget *widget)
{

    struct list_item *current = 0;
    struct widget *text = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
            text = child;

    }

    return text;

}

static char textbuffer[TEXTBOX_SIZE];

static unsigned int copytext(struct widget *text)
{

    return cstring_write_fmt(textbuffer, TEXTBOX_SIZE, 0, "%s\\0", strpool_getstring(text->attributes.label)) - 1;

}

/* all texts of a box one after another, as far as they fit */
static void copytexts(struct widget *widget)
{

    struct list_item *current = 0;
    unsigned int offset = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
            offset += cstring_write_fmt(textbuffer, TEXTBOX_SIZE - 1, offset, "%s", strpool_getstring(child->attributes.label));

    }

    textbuffer[offset] = '\0';

}

/* the marked part of all texts, as indices into all of them; says whether anything is marked */
static unsigned int getmarked(struct widget *widget, unsigned int *start, unsigned int *end)
{

    struct list_item *current = 0;
    unsigned int base = 0;
    unsigned int marked = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            if (child->markstart != child->markend)
            {

                if (!marked)
                    *start = base + util_min(child->markstart, child->markend);

                *end = base + util_max(child->markstart, child->markend);
                marked = 1;

            }

            base += gettextlength(child);

        }

    }

    return marked;

}

/* marks from one index to another across all texts */
static void setmarked(struct widget *widget, unsigned int start, unsigned int end)
{

    struct list_item *current = 0;
    unsigned int base = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            unsigned int length = gettextlength(child);

            child->markstart = util_clamp((int)start - (int)base, 0, length);
            child->markend = util_clamp((int)end - (int)base, 0, length);
            base += length;

        }

    }

}

/* removes the marked part of every text and gives the index where it started */
static unsigned int removemarked(struct widget *widget, unsigned int cursor)
{

    struct list_item *current = 0;
    unsigned int base = 0;
    unsigned int first = 1;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            if (child->markstart != child->markend)
            {

                unsigned int length = copytext(child);
                unsigned int start = util_min(util_min(child->markstart, child->markend), length);
                unsigned int end = util_min(util_max(child->markstart, child->markend), length);

                buffer_copy(textbuffer + start, textbuffer + end, length - end + 1);

                child->attributes.label = strpool_updatestring(child->attributes.label, textbuffer);
                child->markstart = 0;
                child->markend = 0;

                if (first)
                    cursor = base + start;

                first = 0;

            }

            base += gettextlength(child);

        }

    }

    return cursor;

}

/* inserts as much as fits into the text the index falls in and says how much that was */
static unsigned int inserttext(struct widget *widget, unsigned int index, char *data, unsigned int count)
{

    unsigned int offset;
    struct widget *text = pool_gettextat(widget, index, &offset);
    unsigned int length = copytext(text);
    unsigned int i;

    count = util_min(count, TEXTBOX_SIZE - 1 - length);

    for (i = length + 1; i > offset; i--)
        textbuffer[i - 1 + count] = textbuffer[i - 1];

    buffer_copy(textbuffer + offset, data, count);

    text->attributes.label = strpool_updatestring(text->attributes.label, textbuffer);

    return count;

}

static void removetext(struct widget *widget, unsigned int index)
{

    unsigned int offset;
    struct widget *text = pool_gettextat(widget, index, &offset);
    unsigned int length = copytext(text);

    buffer_copy(textbuffer + offset, textbuffer + offset + 1, length - offset);

    text->attributes.label = strpool_updatestring(text->attributes.label, textbuffer);

}

static void notifychange(struct widget *widget)
{

    if (widget->attributes.onchange)
    {

        copytexts(widget);
        sendevent(widget->source, 1, widget->attributes.onchange, textbuffer);

    }

}

/* the cursor counts through all texts of the box, and each edit changes the text the cursor is in */
static void edittextbox(struct widget *widget, unsigned int id)
{

    unsigned int offset;
    struct widget *text = pool_gettextat(widget, widget->attributes.cursor, &offset);
    unsigned int printable = state.keys.code.length == 1 && state.keys.code.value[0] >= 0x20 && state.keys.code.value[0] < 0x7F;
    char c = (printable) ? state.keys.code.value[0] : 0;
    unsigned int previous = util_min(widget->attributes.cursor, pool_gettextlength(widget));
    unsigned int cursor = previous;
    unsigned int changed = 0;
    unsigned int moved = 0;
    unsigned int removed = 0;
    unsigned int start;
    unsigned int end;

    if (!text)
        return;

    /* enter is sent to the program when it asks for it, otherwise it starts a new line */
    if (id == KEYS_KEY_ENTER)
    {

        if (widget->attributes.onenter)
        {

            copytexts(widget);
            sendevent(widget->source, 1, widget->attributes.onenter, textbuffer);

            return;

        }

        c = '\n';

    }

    /* typing or erasing while text is marked first removes the marked text */
    if (getmarked(widget, &start, &end) && (c || id == KEYS_KEY_BACKSPACE || id == KEYS_KEY_DELETE))
    {

        cursor = removemarked(widget, cursor);
        changed = 1;
        removed = 1;

    }

    switch (id)
    {

    case KEYS_KEY_BACKSPACE:
        if (!removed && cursor > 0)
        {

            cursor--;

            removetext(widget, cursor);

            changed = 1;

        }

        break;

    case KEYS_KEY_DELETE:
        if (!removed && cursor < pool_gettextlength(widget))
        {

            removetext(widget, cursor);

            changed = 1;

        }

        break;

    case KEYS_KEY_CURSORLEFT:
        if (cursor > 0)
            cursor--;

        moved = 1;

        break;

    case KEYS_KEY_CURSORRIGHT:
        if (cursor < pool_gettextlength(widget))
            cursor++;

        moved = 1;

        break;

    case KEYS_KEY_CURSORUP:
    case KEYS_KEY_CURSORDOWN:
        {

            int lineheight = pool_getfont(text->attributes.weight)->lineheight;

            cursor = getmarkindex(widget, widget->cursorposition.x, widget->cursorposition.y + lineheight / 2 + ((id == KEYS_KEY_CURSORUP) ? -lineheight : lineheight));
            moved = 1;

        }

        break;

    case KEYS_KEY_HOME:
    case KEYS_KEY_END:
        {

            int lineheight = pool_getfont(text->attributes.weight)->lineheight;

            cursor = getmarkindex(widget, (id == KEYS_KEY_HOME) ? -ROW_FAR : ROW_FAR, widget->cursorposition.y + lineheight / 2);
            moved = 1;

        }

        break;

    default:
        if (c && inserttext(widget, cursor, &c, 1))
        {

            cursor++;
            changed = 1;

        }

        break;

    }

    /* with shift held a move marks from where the marking started to the cursor */
    if (moved && (state.keys.mod & KEYS_MOD_SHIFT))
    {

        unsigned int anchor = previous;

        if (getmarked(widget, &start, &end))
            anchor = (previous == start) ? end : start;

        setmarked(widget, util_min(anchor, cursor), util_max(anchor, cursor));

    }

    else if (moved || changed)
    {

        markwidget(widget, 1);

    }

    widget->attributes.cursor = cursor;
    widget->followcursor = 1;

    if (changed)
        notifychange(widget);

    damageall(widget);

}

/* sends the marked part of every text to the clipboard service, replacing what it held */
static unsigned int copymarked(struct widget *widget)
{

    unsigned int service = channel_lookup(option_getstring("clipboard-service"));
    struct list_item *current = 0;
    unsigned int start;
    unsigned int end;

    if (!service || !getmarked(widget, &start, &end))
        return 0;

    channel_send(1, service, EVENT_CLIPBOARDWRITE, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT && child->markstart != child->markend)
        {

            char *label = strpool_getstring(child->attributes.label);
            unsigned int offset = util_min(util_min(child->markstart, child->markend), gettextlength(child));
            unsigned int stop = util_min(util_max(child->markstart, child->markend), gettextlength(child));

            for (; offset < stop; offset += MESSAGE_SIZE)
                channel_send(1, service, EVENT_CLIPBOARDWRITE, util_min(stop - offset, MESSAGE_SIZE), label + offset);

        }

    }

    return 1;

}

/* replaces the marked text, or inserts at the cursor, with what the clipboard service holds */
static void pastetext(struct widget *widget)
{

    unsigned int service = channel_lookup(option_getstring("clipboard-service"));
    unsigned int cursor = util_min(widget->attributes.cursor, pool_gettextlength(widget));
    struct message message;
    unsigned int offset;

    if (!service || !pool_gettextat(widget, cursor, &offset))
        return;

    cursor = removemarked(widget, cursor);

    channel_send(1, service, EVENT_CLIPBOARDREAD, 0, 0);

    while (channel_poll(1, service, EVENT_CLIPBOARDDATA, &message) && message.length)
        cursor += inserttext(widget, cursor, message.data, message.length);

    widget->attributes.cursor = cursor;
    widget->followcursor = 1;

    notifychange(widget);
    damageall(widget);

}

static void movecursor(struct widget *widget)
{

    unsigned int offset;
    struct widget *first = pool_gettextat(widget, 0, &offset);
    struct widget *last = pool_gettextat(widget, pool_gettextlength(widget), &offset);

    if (first && last)
    {

        int y = util_clamp(state.mouseposition.y, first->placement.position.y, last->placement.position.y + util_max((int)last->placement.size.h - 1, 0));

        widget->attributes.cursor = getmarkindex(widget, state.mouseposition.x, y);
        widget->followcursor = 1;

        damageall(widget);

    }

}

static struct widget *getselect(struct widget *widget)
{

    while (widget && widget->type != WIDGET_TYPE_WINDOW)
    {

        if (widget->type == WIDGET_TYPE_SELECT)
            return widget;

        widget = (widget->attributes.in) ? pool_getwidgetbyid(widget->source, strpool_getstring(widget->attributes.in)) : 0;

    }

    return 0;

}

static void clickwidget(struct widget *widget)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_CHECKBOX:
        if (state.mousebuttonleft)
        {

            widget->attributes.checked = !widget->attributes.checked;

            damage(widget);

            if (widget->attributes.onclick)
                sendevent(widget->source, 1, widget->attributes.onclick, (widget->attributes.checked) ? "1" : "0");

        }

        return;

    case WIDGET_TYPE_ITEM:
        if (state.mousebuttonleft)
        {

            struct widget *select = getselect(widget);
            struct widget *text = gettextchild(widget);

            if (select && select->attributes.onselect)
                sendevent(select->source, 1, select->attributes.onselect, (text) ? strpool_getstring(text->attributes.label) : "");

        }

        break;

    case WIDGET_TYPE_BOX:
        if (state.mousebuttonleft && widget->attributes.mode == ATTR_MODE_EDIT)
            movecursor(widget);

        break;

    case WIDGET_TYPE_WINDOW:
        if (state.mousebuttonleft)
        {

            struct util_region rclose = render_getwindowbutton(widget, RENDER_WINDOWBUTTON_CLOSE);

            if (util_region_intersects(&rclose, state.mouseposition.x, state.mouseposition.y))
                channel_send(0, widget->source, EVENT_INTERRUPT, 0, 0);

        }

        break;

    }

    if (state.mousebuttonleft)
    {

        if (widget->attributes.onclick)
            sendevent(widget->source, 1, widget->attributes.onclick, "");

    }

}

static void purge(unsigned int source)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, source)))
    {

        struct widget *widget = current->data;

        if (widget->state == WIDGET_STATE_DESTROYED)
        {

            if (state.hoverwidget == widget)
                sethover(0);

            if (state.focusedwidget == widget)
                setfocus(0);

            if (state.focusedwindow == widget)
                setfocuswindow(0);

            if (state.draggedthumb == widget)
                state.draggedthumb = 0;

            damageall(widget);
            pool_destroy(widget);

            current = 0;

        }

    }

    if (!state.focusedwindow && state.layout == LAYOUT_TILING)
        setfocuswindow(getolderwindow(state.windows + 1));

}

static void redraw(unsigned int relayout)
{

    if (state.state == STATE_NORMAL && display.framebuffer)
    {

        if (relayout)
        {

            if (state.layout == LAYOUT_TILING)
                tilewindows();

            render_place(state.rootwidget, &display.region);
            render_setmouse(state.mouseposition.x, state.mouseposition.y, getmousetype(state.mouseposition.x, state.mouseposition.y));

        }

        render_update(&display);
        render_undamage();

    }

}

static void scrollpage(int direction)
{

    struct widget *scrollablewidget = getscrollablewidgetat(state.mouseposition.x, state.mouseposition.y);

    if (scrollablewidget)
        scrollwidget(scrollablewidget, direction * util_max((int)widget_getview(scrollablewidget).size.h, 16));

    sethover(gethoverwidgetat(state.mouseposition.x, state.mouseposition.y));

}

static void moveselection(struct widget *item, unsigned int forward)
{

    struct widget *parent = getparent(item);
    struct widget *previous = 0;
    struct widget *next = 0;
    struct widget *target;
    struct list_item *current = 0;
    unsigned int found = 0;

    while (parent && (current = pool_nextin(current, parent)))
    {

        struct widget *child = current->data;

        if (child->type != WIDGET_TYPE_ITEM)
            continue;

        if (child == item)
            found = 1;
        else if (!found)
            previous = child;
        else if (!next)
            next = child;

    }

    target = (forward) ? next : previous;

    if (target)
    {

        if ((state.keys.mod & KEYS_MOD_SHIFT))
            selectrange(target);
        else
            selectitem(target);

        setfocus(target);
        showwidget(target);

    }

}

static struct widget *getgroupitem(struct widget *parent)
{

    struct list_item *current = 0;
    struct widget *first = 0;

    while ((current = pool_nextin(current, parent)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_ITEM && child->anchor)
            return child;

        if (child->type == WIDGET_TYPE_ITEM && !first)
            first = child;

    }

    return first;

}

static unsigned int isfocusable(struct widget *widget)
{

    struct widget *parent = getparent(widget);

    if (widget->attributes.mode == ATTR_MODE_DISABLED)
        return 0;

    if (getselect(widget) && widget->type != WIDGET_TYPE_SELECT)
        return 0;

    if (parent && parent->type == WIDGET_TYPE_ITEM)
        return 0;

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
    case WIDGET_TYPE_CHECKBOX:
    case WIDGET_TYPE_SELECT:
        return 1;

    case WIDGET_TYPE_TEXT:
        return widget_islink(widget);

    case WIDGET_TYPE_ITEM:
        return parent && widget == getgroupitem(parent);

    case WIDGET_TYPE_BOX:
        return widget->attributes.mode == ATTR_MODE_EDIT;

    }

    return 0;

}

static unsigned int collectfocusable(struct widget *widget, struct widget **list, unsigned int count)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (count < NUM_FOCUSABLE && isfocusable(child))
            list[count++] = child;

        count = collectfocusable(child, list, count);

    }

    return count;

}

/* moves the focus to the next or previous widget in the focused window that takes keys, and says whether there was one */
static unsigned int cyclefocus(unsigned int forward)
{

    static struct widget *list[NUM_FOCUSABLE];
    struct widget *item = (state.focusedwidget) ? getitem(state.focusedwidget) : 0;
    unsigned int count;
    unsigned int current;
    unsigned int i;

    if (!state.focusedwindow)
        return 0;

    count = collectfocusable(state.focusedwindow, list, 0);

    if (!count)
        return 0;

    for (current = 0; current < count; current++)
    {

        if (list[current] == state.focusedwidget || (item && list[current]->type == WIDGET_TYPE_ITEM && getparent(list[current]) == getparent(item)))
            break;

    }

    if (current == count)
        i = (forward) ? 0 : count - 1;
    else
        i = (current + ((forward) ? 1 : count - 1)) % count;

    setfocus(list[i]);
    showwidget(list[i]);

    return 1;

}

/* marks every text in a box; an edit box's cursor goes to the end; anything else gets the key */
static unsigned int markall(struct widget *widget)
{

    unsigned int total;

    if (!widget || widget->type != WIDGET_TYPE_BOX)
        return 0;

    total = pool_gettextlength(widget);

    setmarked(widget, 0, total);

    if (widget_iseditable(widget))
    {

        widget->attributes.cursor = total;
        widget->followcursor = 1;

    }

    damageall(widget);

    return 1;

}

static unsigned int dobinding(struct bind_binding *binding)
{

    switch (binding->action)
    {

    case BIND_ACTION_CLOSE:
        if (state.focusedwindow)
            channel_send(0, state.focusedwindow->source, EVENT_INTERRUPT, 0, 0);

        break;

    case BIND_ACTION_NEXT:
        cyclewindow(1);

        break;

    case BIND_ACTION_PREVIOUS:
        cyclewindow(0);

        break;

    case BIND_ACTION_RUN:
        launch(binding->command);

        break;

    case BIND_ACTION_SCROLLDOWN:
        scrollpage(1);

        break;

    case BIND_ACTION_SCROLLUP:
        scrollpage(-1);

        break;

    case BIND_ACTION_SWAP:
        if (state.layout == LAYOUT_TILING)
            swapwindow();

        break;

    case BIND_ACTION_FOCUSNEXT:
        return cyclefocus(1);

    case BIND_ACTION_FOCUSPREVIOUS:
        return cyclefocus(0);

    case BIND_ACTION_MARKALL:
        return markall(state.focusedwidget);

    case BIND_ACTION_COPY:
        return state.focusedwidget && state.focusedwidget->type == WIDGET_TYPE_BOX && copymarked(state.focusedwidget);

    case BIND_ACTION_CUT:
        if (!state.focusedwidget || !widget_iseditable(state.focusedwidget) || !copymarked(state.focusedwidget))
            return 0;

        edittextbox(state.focusedwidget, KEYS_KEY_DELETE);

        break;

    case BIND_ACTION_PASTE:
        if (!state.focusedwidget || !widget_iseditable(state.focusedwidget))
            return 0;

        pastetext(state.focusedwidget);

        break;

    }

    return 1;

}

static void onkeypress(struct message *message)
{

    struct event_keypress *keypress = message->data;
    unsigned int id = keys_getcode(&state.keys, keypress->scancode);
    struct widget *selecteditem = (state.focusedwidget) ? getselecteditem(state.focusedwidget) : 0;
    struct bind_binding *binding = bind_find(id, state.keys.mod);

    if (id && (!binding || !dobinding(binding)))
    {

        if (selecteditem && id == KEYS_KEY_ENTER && selecteditem->attributes.onenter)
        {

            sendevent(selecteditem->source, 1, selecteditem->attributes.onenter, "");

        }

        else if (selecteditem && id == KEYS_KEY_CURSORUP)
        {

            moveselection(selecteditem, 0);

        }

        else if (selecteditem && id == KEYS_KEY_CURSORDOWN)
        {

            moveselection(selecteditem, 1);

        }

        else if (state.focusedwidget && widget_iseditable(state.focusedwidget))
        {

            edittextbox(state.focusedwidget, id);

        }

        else if (state.focusedwidget && state.focusedwidget->type == WIDGET_TYPE_BOX && (state.keys.mod & KEYS_MOD_SHIFT) && ismarkkey(id))
        {

            extendmark(state.focusedwidget, id);

        }

        else
        {

            if (state.focusedwindow)
            {

                struct event_wmkeypress wmkeypress;

                wmkeypress.id = state.keys.id;
                wmkeypress.scancode = keypress->scancode;
                wmkeypress.unicode = state.keys.code.value[0];
                wmkeypress.length = state.keys.code.length;
                wmkeypress.keymod = state.keys.mod;

                channel_send(0, state.focusedwindow->source, EVENT_WMKEYPRESS, sizeof (struct event_wmkeypress), &wmkeypress);

            }

        }

    }

    redraw(1);

}

static void onkeyrelease(struct message *message)
{

    struct event_keyrelease *keyrelease = message->data;

    keys_getcode(&state.keys, keyrelease->scancode);

}

static void reloadconfig(void)
{

    struct event_videoconf videoconf = state.videoconf;

    bind_reset();
    loadconfig(option_getstring("config"));

    if (videoconf.width != state.videoconf.width || videoconf.height != state.videoconf.height || videoconf.bpp != state.videoconf.bpp)
    {

        setupvideo(state.video);

    }

    else
    {

        render_damage(0, 0, display.region.size.w, display.region.size.h);
        redraw(1);

    }

}

/* sets the options like every program does, and reloads the configuration when it is pointed out once the wm runs */
static void onoption(struct message *message)
{

    char *data = message->data;
    unsigned int offset = 0;

    if (!message->length || data[message->length - 1] != '\0')
        return;

    while (offset < message->length)
    {

        char *key = data + offset;
        char *value = key + cstring_length_zero(key);

        offset += cstring_length_zero(key);

        if (offset >= message->length)
            break;

        offset += cstring_length_zero(value);

        if (!option_getstring(key))
            channel_send_fmt(0, message->source, EVENT_ERROR, "Unrecognized option: %s\n", key);
        else if (!option_setstring(key, value))
            channel_send_fmt(0, message->source, EVENT_ERROR, "Option too long: %s\n", key);
        else if (state.video && cstring_match(key, "config"))
            reloadconfig();

    }

}

static void onmain(struct message *message)
{

    unsigned int keyboard = channel_lookup(option_getstring("keyboard-service"));
    unsigned int mouse = channel_lookup(option_getstring("mouse-service"));
    unsigned int video = channel_lookup(option_getstring("video-service"));

    state.video = video;

    loadconfig(option_getstring("config"));

    call_announce(0, cstring_length(option_getstring("service")), option_getstring("service"));
    channel_send(0, keyboard, EVENT_LINK, 0, 0);
    channel_send(0, mouse, EVENT_LINK, 0, 0);
    setupvideo(video);
    channel_hold(0);
    channel_send(0, mouse, EVENT_UNLINK, 0, 0);
    channel_send(0, keyboard, EVENT_UNLINK, 0, 0);

}

/* moves the content so that the thumb stays under the point where it was grabbed */
static void dragthumb(struct widget *widget)
{

    int track = util_max((int)widget->placement.size.h - CONFIG_FRAME_HEIGHT * 2, 0);
    int free = track - (int)widget->thumb.size.h;

    if (widget->scrollmax > 0 && free > 0)
    {

        int thumb = state.mouseposition.y - state.scrollgrab - (widget->placement.position.y + CONFIG_FRAME_HEIGHT);

        widget->scroll.y = util_clamp(thumb, 0, free) * widget->scrollmax / free;

        damage(widget);

    }

}

static void onmousemove(struct message *message)
{

    struct event_mousemove *mousemove = message->data;
    int x = util_clamp(state.mouseposition.x + mousemove->relx, 0, display.region.size.w);
    int y = util_clamp(state.mouseposition.y + mousemove->rely, 0, display.region.size.h);
    unsigned int relayout = state.mousebuttonleft || state.mousebuttonright;

    state.mousemovement.x = x - state.mouseposition.x;
    state.mousemovement.y = y - state.mouseposition.y;
    state.mouseposition.x = x;
    state.mouseposition.y = y;

    sethover(gethoverwidgetat(state.mouseposition.x, state.mouseposition.y));
    render_setmouse(state.mouseposition.x, state.mouseposition.y, getmousetype(state.mouseposition.x, state.mouseposition.y));

    if (state.mousebuttonleft)
    {

        state.markpoint = state.mouseposition;

        if (state.draggedthumb)
            dragthumb(state.draggedthumb);
        else if (state.focusedwidget)
        {

            markwidget(state.focusedwidget, 0);

            /* the cursor follows the end of the marking, so the box scrolls along */
            if (widget_iseditable(state.focusedwidget))
                movecursor(state.focusedwidget);
            else if (widget_isscrollable(state.focusedwidget))
            {

                struct util_region point = util_region(state.mouseposition.x, state.mouseposition.y, 0, 0);

                scrollto(state.focusedwidget, &point);

            }

        }

        if (!state.focusedwidget && state.focusedwindow)
        {

            if (state.layout == LAYOUT_DEFAULT && widget_isdragable(state.focusedwindow) && translatewindow(state.focusedwindow, state.mousemovement.x, state.mousemovement.y))
                relayout = 0;

        }

    }

    if (state.mousebuttonright)
    {

        if (state.focusedwindow)
        {

            if (state.layout == LAYOUT_DEFAULT && widget_isresizable(state.focusedwindow))
            {

                struct util_size minimum = render_getwindowminsize(state.focusedwindow, &display.region.size);

                scalewidget(state.focusedwindow, util_max((int)(state.focusedwindow->placement.size.w) + state.mousemovement.x, minimum.w), util_max((int)(state.focusedwindow->placement.size.h) + state.mousemovement.y, minimum.h));

            }

        }

    }

    redraw(relayout);

}

static void grabthumb(void)
{

    struct widget *widget = getscrollablewidgetat(state.mouseposition.x, state.mouseposition.y);

    state.draggedthumb = (widget && util_region_intersects(&widget->thumb, state.mouseposition.x, state.mouseposition.y)) ? widget : 0;

    if (state.draggedthumb)
        state.scrollgrab = state.mouseposition.y - widget->thumb.position.y;

}

static void onmousepress(struct message *message)
{

    struct event_mousepress *mousepress = message->data;
    struct widget *window = getwidgetoftypeat(WIDGET_TYPE_WINDOW, state.mouseposition.x, state.mouseposition.y);
    struct widget *interactivewidget = getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y);

    state.mousepressed.x = state.mouseposition.x;
    state.mousepressed.y = state.mouseposition.y;
    state.markpoint = state.mouseposition;
    state.markscroll = (interactivewidget) ? interactivewidget->scroll : util_position(0, 0);

    switch (mousepress->button)
    {

    case 1:
        state.mousebuttonleft = 1;

        setfocuswindow(window);
        setfocus(interactivewidget);
        grabthumb();

        if (interactivewidget)
        {

            struct widget *item = getitem(interactivewidget);

            if (item && getselect(item))
                item = 0;

            if (item && (state.keys.mod & KEYS_MOD_SHIFT))
                selectrange(item);
            else if (item && (state.keys.mod & KEYS_MOD_CTRL))
                toggleitem(item);
            else if (item)
                selectitem(item);

            if (!item || !(state.keys.mod & (KEYS_MOD_SHIFT | KEYS_MOD_CTRL)))
                clickwidget(interactivewidget);
            markwidget(interactivewidget, 0);

        }

        break;

    case 2:
        state.mousebuttonright = 1;

        break;

    }

    redraw(1);

}

static void onmousescroll(struct message *message)
{

    struct widget *scrollablewidget = getscrollablewidgetat(state.mouseposition.x, state.mouseposition.y);

    if (scrollablewidget)
    {

        struct event_mousescroll *mousescroll = message->data;

        scrollwidget(scrollablewidget, mousescroll->relz * 16);

    }

    sethover(gethoverwidgetat(state.mouseposition.x, state.mouseposition.y));

    redraw(1);

}

static void onmouserelease(struct message *message)
{

    struct event_mouserelease *mouserelease = message->data;

    state.mousereleased.x = state.mouseposition.x;
    state.mousereleased.y = state.mouseposition.y;

    switch (mouserelease->button)
    {

    case 1:
        state.mousebuttonleft = 0;
        state.draggedthumb = 0;

        break;

    case 2:
        state.mousebuttonright = 0;

        break;

    }

    redraw(1);

}

static void onvideoinfo(struct message *message)
{

    struct event_videoinfo videoinfo = *(struct event_videoinfo *)message->data;
    unsigned int factor = videoinfo.height / 320;
    unsigned int lineheight = 12 + factor * 4;
    unsigned int padding = 4 + factor * 2;

    blit_initdisplay(&display, (void *)(unsigned long)videoinfo.framebuffer, videoinfo.width, videoinfo.height, videoinfo.bpp, linebuffer);
    pool_loadfont(0, fontn[factor]);
    pool_setfont(0, lineheight, padding);
    pool_loadfont(1, fontb[factor]);
    pool_setfont(1, lineheight, padding);

    state.mouseposition.x = videoinfo.width / 4;
    state.mouseposition.y = videoinfo.height / 4;

    render_setmouse(state.mouseposition.x, state.mouseposition.y, BLIT_MOUSE_ARROW);

    render_damage(0, 0, videoinfo.width, videoinfo.height);
    render_place(state.rootwidget, &display.region);
    render_update(&display);
    render_undamage();

}

static void onwmgrab(struct message *message)
{

    state.state = STATE_GRABBED;

    channel_bind(EVENT_KEYPRESS, 0);
    channel_bind(EVENT_KEYRELEASE, 0);
    channel_bind(EVENT_MOUSEMOVE, 0);
    channel_bind(EVENT_MOUSEPRESS, 0);
    channel_bind(EVENT_MOUSESCROLL, 0);
    channel_bind(EVENT_MOUSERELEASE, 0);
    channel_bind(EVENT_VIDEOINFO, 0);
    channel_send(0, message->source, EVENT_WMACK, 0, 0);

}

static void onwmmap(struct message *message)
{

    channel_send(0, message->source, EVENT_WMINIT, 0, 0);

}

static void onwmrenderdata(struct message *message)
{

    parser_parse(message->source, "root", message->length, message->data);
    pool_loadresources();
    placewindows(message->source);
    purge(message->source);
    redraw(1);

}

/* sends the texts in a program's widget as pieces, and an empty piece at the end */
static void onwmread(struct message *message)
{

    unsigned int source = message->source;
    char id[64];
    struct widget *widget;

    cstring_write_fmt(id, 64, 0, "%s\\0", message->data);

    widget = pool_getwidgetbyid(source, id);

    if (widget)
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            if (child->type == WIDGET_TYPE_TEXT)
            {

                char *label = strpool_getstring(child->attributes.label);
                unsigned int length = strpool_getcstringlength(child->attributes.label);
                unsigned int offset;

                for (offset = 0; offset < length; offset += MESSAGE_SIZE)
                    channel_send(0, source, EVENT_WMTEXT, util_min(length - offset, MESSAGE_SIZE), label + offset);

            }

        }

    }

    channel_send(0, source, EVENT_WMTEXT, 0, 0);

}

static void onwmrenderfile(struct message *message)
{

    char data[4096];
    unsigned int count = fs_read_path(1, message->data, data, 4096);

    if (count)
    {

        parser_parse(message->source, "root", count, data);
        pool_loadresources();
        placewindows(message->source);
        purge(message->source);

    }

    redraw(1);

}

static void onwmungrab(struct message *message)
{

    state.state = STATE_NORMAL;

    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_MOUSEMOVE, onmousemove);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_MOUSESCROLL, onmousescroll);
    channel_bind(EVENT_MOUSERELEASE, onmouserelease);
    channel_bind(EVENT_VIDEOINFO, onvideoinfo);
    channel_send(0, message->source, EVENT_WMACK, 0, 0);
    setupvideo(channel_lookup(option_getstring("video-service")));

}

static void onwmunmap(struct message *message)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, message->source)))
    {

        struct widget *widget = current->data;

        widget_setstate(widget, WIDGET_STATE_DESTROYED);

    }

    purge(message->source);
    redraw(1);

}

static void setupwidgets(void)
{

    char *data0 = "+ layout id \"root\" flow \"stretch\"\n";
    char data1[4096];

    parser_parse(0, "", cstring_length(data0), data0);
    parser_parse(0, "root", fs_read_path(1, "initrd:data/alfi/wm.alfi", data1, 4096), data1);
    pool_loadresources();

    state.rootwidget = pool_getwidgetbyid(0, "root");

}

void init(void)
{

    keys_init(&state.keys, KEYS_LAYOUT_QWERTY_US, KEYS_MAP_US);
    state.videoconf.width = 1920;
    state.videoconf.height = 1080;
    state.videoconf.bpp = 4;
    pool_setup();
    setupwidgets();
    render_init();
    option_add("service", "wm");
    option_add("config", "initrd:data/config/wm.mq");
    option_add("keyboard-service", "keyboard");
    option_add("mouse-service", "mouse");
    option_add("video-service", "video");
    option_add("clipboard-service", "clipboard");
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_MOUSEMOVE, onmousemove);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_MOUSESCROLL, onmousescroll);
    channel_bind(EVENT_MOUSERELEASE, onmouserelease);
    channel_bind(EVENT_VIDEOINFO, onvideoinfo);
    channel_bind(EVENT_WMGRAB, onwmgrab);
    channel_bind(EVENT_WMMAP, onwmmap);
    channel_bind(EVENT_WMREAD, onwmread);
    channel_bind(EVENT_WMRENDERDATA, onwmrenderdata);
    channel_bind(EVENT_WMRENDERFILE, onwmrenderfile);
    channel_bind(EVENT_WMUNGRAB, onwmungrab);
    channel_bind(EVENT_WMUNMAP, onwmunmap);

}

