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
#define EVENTDATA_SIZE      512
#define LAYOUT_DEFAULT      0
#define LAYOUT_TILING       1
#define NUM_FOCUSABLE       256
#define ROW_FAR             0x40000000
#define MOUSE_NORMAL        4
#define MOUSE_FASTEST       16
#define MOUSE_RAMP          8

struct state
{

    unsigned int state;
    struct util_position mouseposition;
    struct util_position mousemovement;
    struct util_position mouseremainder;
    unsigned int mousespeed;
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

static void loadbackground(char *path, unsigned int length)
{

    char name[256];
    char size[64];
    char data[128];
    unsigned int count;

    if (!length || length >= 256)
        return;

    buffer_copy(name, path, length);

    name[length] = 0;
    count = system_feed(0, 0, size, 63, "mapimage %s mem:background", name);
    size[count] = 0;

    if (count)
    {

        parser_parse(0, "root", cstring_write_fmt(data, 128, 0, "= background mimetype \"image/raw\" %s", size), data);
        pool_loadresources();

    }

}

static void configline(char *line, unsigned int length)
{

    char *word = line;
    unsigned int count = getword(line, length, 0, &word);
    unsigned int start = (count < length) ? count + 1 : length;
    char *rest = line + start;

    length -= start;

    if (count == 6 && buffer_match(word, "layout", 6))
    {

        state.layout = (length >= 6 && buffer_match(rest, "tiling", 6)) ? LAYOUT_TILING : LAYOUT_DEFAULT;

    }

    else if (count == 10 && buffer_match(word, "background", 10))
    {

        loadbackground(rest, length);

    }

    else if (count == 8 && buffer_match(word, "keyboard", 8))
    {

        keys_init(&state.keys, KEYS_LAYOUT_QWERTY_US, (length >= 9 && buffer_match(rest, "qwerty_SE", 9)) ? keys_map_se : (void *)0);

    }

    else if (count == 10 && buffer_match(word, "resolution", 10))
    {

        state.videoconf.width = getnumber(rest, length, 0, 10, state.videoconf.width);
        state.videoconf.height = getnumber(rest, length, 1, 10, state.videoconf.height);
        state.videoconf.bpp = getnumber(rest, length, 2, 10, state.videoconf.bpp);

    }

    else if (count == 5 && buffer_match(word, "mouse", 5))
    {

        state.mousespeed = util_clamp(getnumber(rest, length, 0, 10, MOUSE_NORMAL), 1, MOUSE_FASTEST);

    }

    else if (count == 5 && buffer_match(word, "color", 5))
    {

        unsigned int space = buffer_findbyte(rest, length, ' ');
        char name[64];

        if (space < length && space < 64)
        {

            buffer_write(name, 64, rest, space, 0);
            name[space] = '\0';
            cmap_set(name, cstring_read_value(rest + space + 1, length - space - 1, 16));

        }

    }

    else if (count == 8 && buffer_match(word, "bindings", 8))
    {

        bind_reset();

    }

    else if (count == 7 && buffer_match(word, "binding", 7))
    {

        bind_add(rest, length);

    }

}

static void configlines(char *data, unsigned int count)
{

    unsigned int length;
    char *line;

    for (line = data; line < data + count; line += length + 1)
    {

        length = buffer_findbyte(line, data + count - line, '\n');

        if (length)
            configline(line, length);

    }

}

static void loadconfig(char *config)
{

    static char data[0x4000];

    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .general -format \"layout {layout}\" %s", config));
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .general -format \"background {background}\" %s", config));
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .keyboard -format \"keyboard {layout}\" %s", config));
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .screen.resolution -format \"resolution {width} {height} {bpp}\" %s", config));
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .mouse -format \"mouse {speed}\" %s", config));
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .colors -format \"color {name} {value}\" %s", config));
    configlines("bindings", 8);
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .bindings -format \"binding {key} {mod} {action} {command}\" %s", config));

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

    return (widget->parent && widget->parent->source == widget->source) ? widget->parent : 0;

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

    if (widget->type == WIDGET_TYPE_WINDOW)
        return;

    while ((current = pool_nextin(current, widget)))
        damageall(current->data);

}

static void translatewindow(struct widget *window, int x, int y)
{

    damage(window);

    window->position.x += x;
    window->position.y += y;

    damage(window);

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

        struct {struct event_wmevent wmevent; char data[EVENTDATA_SIZE];} message;
        unsigned int count = cstring_write_fmt(message.data, EVENTDATA_SIZE - 1, 0, "%s", strpool_getstring(action));

        count += buffer_write(message.data, EVENTDATA_SIZE - 1, text, utf8_fit(text, cstring_length(text), EVENTDATA_SIZE - 1 - count), count);
        message.data[count] = '\0';
        message.wmevent.type = type;
        message.wmevent.length = count + 1;

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

static unsigned int stepleft(struct widget *widget, unsigned int index)
{

    unsigned int offset;
    struct widget *text;

    if (!index)
        return 0;

    text = pool_gettextat(widget, index - 1, &offset);

    return (text) ? index - 1 - offset + utf8_prev(strpool_getstring(text->attributes.label), offset + 1) : index - 1;

}

static unsigned int stepright(struct widget *widget, unsigned int index)
{

    unsigned int offset;
    struct widget *text = pool_gettextat(widget, index, &offset);
    unsigned int codepoint;

    if (!text || offset >= gettextlength(text))
        return index;

    return index + utf8_read(strpool_getstring(text->attributes.label) + offset, gettextlength(text) - offset, &codepoint);

}

static unsigned int moveindex(struct widget *widget, unsigned int index, unsigned int id)
{

    struct util_region cursor = render_getcursor(widget, index);
    int y = cursor.position.y + (int)cursor.size.h / 2;

    switch (id)
    {

    case KEYS_KEY_CURSORLEFT:
        return stepleft(widget, index);

    case KEYS_KEY_CURSORRIGHT:
        return stepright(widget, index);

    }

    if (!cursor.size.h)
        return index;

    switch (id)
    {

    case KEYS_KEY_CURSORUP:
        return getmarkindex(widget, cursor.position.x, y - (int)cursor.size.h);

    case KEYS_KEY_CURSORDOWN:
        return getmarkindex(widget, cursor.position.x, y + (int)cursor.size.h);

    case KEYS_KEY_HOME:
        return getmarkindex(widget, -ROW_FAR, y);

    case KEYS_KEY_END:
        return getmarkindex(widget, ROW_FAR, y);

    }

    return index;

}

static void setmark(struct widget *widget, unsigned int anchor, unsigned int point)
{

    struct list_item *current = 0;
    unsigned int start = util_min(anchor, point);
    unsigned int end = util_max(anchor, point);
    unsigned int base = 0;

    widget->markstart = anchor;
    widget->markend = point;

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

static unsigned int ismarkkey(unsigned int id)
{

    return id == KEYS_KEY_CURSORLEFT || id == KEYS_KEY_CURSORRIGHT || id == KEYS_KEY_CURSORUP || id == KEYS_KEY_CURSORDOWN || id == KEYS_KEY_HOME || id == KEYS_KEY_END;

}

static void extendmark(struct widget *textbox, unsigned int id)
{

    setmark(textbox, textbox->markstart, moveindex(textbox, textbox->markend, id));
    damageall(textbox);

}

static void setfocus(struct widget *widget)
{

    if (state.focusedwidget)
    {

        setmark(state.focusedwidget, 0, 0);
        damageall(state.focusedwidget);

        state.focusedwidget = 0;

    }

    if (widget && widget != state.focusedwindow)
    {

        state.focusedwidget = widget;

        damageall(state.focusedwidget);

        if (state.focusedwidget->type == WIDGET_TYPE_SELECT)
        {

            bumpchildren(state.focusedwidget);

        }

    }

    render_setstate(state.hoverwidget, state.focusedwidget, state.focusedwindow);

}

static void setfocuswindow(struct widget *widget)
{

    if (state.focusedwindow)
        damageall(state.focusedwindow);

    state.focusedwindow = widget;

    if (state.focusedwindow)
        bump(state.focusedwindow);

    render_setstate(state.hoverwidget, state.focusedwidget, state.focusedwindow);

}

static void sethover(struct widget *widget)
{

    if (widget == state.hoverwidget)
        return;

    if (state.hoverwidget)
        damageall(state.hoverwidget);

    state.hoverwidget = widget;

    if (state.hoverwidget)
        damageall(state.hoverwidget);

    render_setstate(state.hoverwidget, state.focusedwidget, state.focusedwindow);

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

static struct widget *getadjacentwindow(unsigned int serial, int direction)
{

    struct list_item *current = 0;
    struct widget *nearest = 0;
    int distance = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;
        int d = ((int)widget->serial - (int)serial) * direction;

        if (widget->type == WIDGET_TYPE_WINDOW && !widget->destroyed && d > 0 && (!nearest || d < distance))
        {

            nearest = widget;
            distance = d;

        }

    }

    return nearest;

}

static void cyclewindow(unsigned int older)
{

    struct widget *window = 0;

    if (state.focusedwindow)
        window = getadjacentwindow(state.focusedwindow->serial, (older) ? -1 : 1);

    if (!window)
        window = (older) ? getadjacentwindow(state.windows + 1, -1) : getadjacentwindow(0, 1);

    if (window)
    {

        setfocuswindow(window);
        setfocus(0);

    }

}

static void swapwindow(void)
{

    struct widget *first = getadjacentwindow(state.windows + 1, -1);

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

static void tilewindows(void)
{

    struct widget *menu = pool_getwidgetbyid(0, "menu");
    int top = (menu) ? menu->placement.position.y + (int)menu->placement.size.h : 0;
    unsigned int w = display.region.size.w / 2;
    unsigned int h = util_max((int)display.region.size.h - top, 0);
    struct widget *first = getadjacentwindow(state.windows + 1, -1);
    struct widget *widget;
    unsigned int count = 0;
    unsigned int i = 0;

    if (!first)
        return;

    placewindow(first, 0, top, w, h);

    for (widget = getadjacentwindow(first->serial, -1); widget; widget = getadjacentwindow(widget->serial, -1))
        count++;

    for (widget = getadjacentwindow(first->serial, -1); widget; widget = getadjacentwindow(widget->serial, -1), i++)
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

static unsigned int getmarked(struct widget *widget, unsigned int *start, unsigned int *end)
{

    unsigned int total = pool_gettextlength(widget);

    *start = util_min(util_min(widget->markstart, widget->markend), total);
    *end = util_min(util_max(widget->markstart, widget->markend), total);

    return *start < *end;

}

static unsigned int removemarked(struct widget *widget, unsigned int cursor)
{

    struct list_item *current = 0;
    unsigned int base = 0;
    unsigned int start;
    unsigned int end;

    if (!getmarked(widget, &start, &end))
        return cursor;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            unsigned int length = copytext(child);
            unsigned int from = util_clamp((int)start - (int)base, 0, length);
            unsigned int to = util_clamp((int)end - (int)base, 0, length);

            if (from < to)
            {

                buffer_copy(textbuffer + from, textbuffer + to, length - to + 1);

                child->attributes.label = strpool_updatestring(child->attributes.label, textbuffer);

            }

            base += length;

        }

    }

    setmark(widget, 0, 0);

    return start;

}

static unsigned int inserttext(struct widget *widget, unsigned int index, char *data, unsigned int count)
{

    unsigned int offset;
    struct widget *text = pool_gettextat(widget, index, &offset);
    unsigned int length = copytext(text);
    unsigned int i;

    count = utf8_fit(data, count, TEXTBOX_SIZE - 1 - length);

    for (i = length + 1; i > offset; i--)
        textbuffer[i - 1 + count] = textbuffer[i - 1];

    buffer_copy(textbuffer + offset, data, count);

    text->attributes.label = strpool_updatestring(text->attributes.label, textbuffer);

    return count;

}

static void removetext(struct widget *widget, unsigned int index, unsigned int count)
{

    unsigned int offset;
    struct widget *text = pool_gettextat(widget, index, &offset);
    unsigned int length = copytext(text);

    buffer_copy(textbuffer + offset, textbuffer + offset + count, length - offset - count + 1);

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

static void edittextbox(struct widget *widget, unsigned int id)
{

    unsigned int offset;
    struct widget *text = pool_gettextat(widget, widget->attributes.cursor, &offset);
    unsigned int printable = state.keys.code.length && state.keys.code.value[0] >= 0x20 && state.keys.code.value[0] != 0x7F;
    char *data = (char *)state.keys.code.value;
    unsigned int count = (printable) ? state.keys.code.length : 0;
    unsigned int previous = util_min(widget->attributes.cursor, pool_gettextlength(widget));
    unsigned int cursor = previous;
    unsigned int changed = 0;
    unsigned int moved = 0;
    unsigned int removed = 0;
    unsigned int start;
    unsigned int end;

    if (!text)
        return;

    if (id == KEYS_KEY_ENTER)
    {

        if (widget->attributes.onenter)
        {

            copytexts(widget);
            sendevent(widget->source, 1, widget->attributes.onenter, textbuffer);

            return;

        }

        if (!widget_isscrollable(widget))
            return;

        data = "\n";
        count = 1;

    }

    if (getmarked(widget, &start, &end) && (count || id == KEYS_KEY_BACKSPACE || id == KEYS_KEY_DELETE))
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

            unsigned int left = stepleft(widget, cursor);

            removetext(widget, left, cursor - left);

            cursor = left;

            changed = 1;

        }

        break;

    case KEYS_KEY_DELETE:
        if (!removed && cursor < pool_gettextlength(widget))
        {

            removetext(widget, cursor, stepright(widget, cursor) - cursor);

            changed = 1;

        }

        break;

    case KEYS_KEY_CURSORLEFT:
    case KEYS_KEY_CURSORRIGHT:
    case KEYS_KEY_CURSORUP:
    case KEYS_KEY_CURSORDOWN:
    case KEYS_KEY_HOME:
    case KEYS_KEY_END:
        cursor = moveindex(widget, cursor, id);
        moved = 1;

        break;

    default:
        if (count && inserttext(widget, cursor, data, count) == count)
        {

            cursor += count;
            changed = 1;

        }

        break;

    }

    if (moved && (state.keys.mod & KEYS_MOD_SHIFT))
    {

        setmark(widget, (getmarked(widget, &start, &end)) ? widget->markstart : previous, cursor);

    }

    else if (moved || changed)
    {

        setmark(widget, 0, 0);

    }

    widget->attributes.cursor = cursor;
    widget->followcursor = 1;

    if (changed)
        notifychange(widget);

    damageall(widget);

}

static unsigned int copymarked(struct widget *widget)
{

    unsigned int service = channel_lookup(option_getstring("clipboard-service"));
    struct list_item *current = 0;
    unsigned int base = 0;
    unsigned int start;
    unsigned int end;

    if (!service || !getmarked(widget, &start, &end))
        return 0;

    channel_send(1, service, EVENT_CLIPBOARDWRITE, 0, 0);

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (child->type == WIDGET_TYPE_TEXT)
        {

            char *label = strpool_getstring(child->attributes.label);
            unsigned int length = gettextlength(child);
            unsigned int offset = util_clamp((int)start - (int)base, 0, length);
            unsigned int stop = util_clamp((int)end - (int)base, 0, length);

            for (; offset < stop; offset += MESSAGE_SIZE)
                channel_send(1, service, EVENT_CLIPBOARDWRITE, util_min(stop - offset, MESSAGE_SIZE), label + offset);

            base += length;

        }

    }

    return 1;

}

static void pastetext(struct widget *widget)
{

    unsigned int service = channel_lookup(option_getstring("clipboard-service"));
    unsigned int cursor = util_min(widget->attributes.cursor, pool_gettextlength(widget));
    unsigned int multiline = widget_isscrollable(widget);
    unsigned int stopped = 0;
    struct message message;
    unsigned int offset;

    if (!service || !pool_gettextat(widget, cursor, &offset))
        return;

    cursor = removemarked(widget, cursor);

    channel_send(1, service, EVENT_CLIPBOARDREAD, 0, 0);

    while (channel_poll(1, service, EVENT_CLIPBOARDDATA, &message) && message.length)
    {

        unsigned int length = (multiline) ? message.length : buffer_findbyte(message.data, message.length, '\n');

        if (!stopped)
            cursor += inserttext(widget, cursor, message.data, length);

        if (length < message.length)
            stopped = 1;

    }

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

        widget = getparent(widget);

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

    struct list_item *current = pool_nextsource(0, source);

    while (current)
    {

        struct widget *widget = current->data;

        current = pool_nextsource(current, source);

        if (widget->destroyed)
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

        }

    }

    if (!state.focusedwindow && state.layout == LAYOUT_TILING)
        setfocuswindow(getadjacentwindow(state.windows + 1, -1));

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

static unsigned int markall(struct widget *widget)
{

    unsigned int total;

    if (!widget || widget->type != WIDGET_TYPE_BOX)
        return 0;

    total = pool_gettextlength(widget);

    setmark(widget, 0, total);

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
                wmkeypress.unicode = 0;
                wmkeypress.length = state.keys.code.length;

                buffer_copy(&wmkeypress.unicode, state.keys.code.value, wmkeypress.length);
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

static void applyconfig(struct event_videoconf *videoconf)
{

    if (videoconf->width != state.videoconf.width || videoconf->height != state.videoconf.height || videoconf->bpp != state.videoconf.bpp)
    {

        setupvideo(state.video);

    }

    else
    {

        render_damage(0, 0, display.region.size.w, display.region.size.h);
        redraw(1);

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

static int scalemove(int count, int quarters, int *remainder)
{

    int total = *remainder + count * quarters;
    int pixels = (total < 0) ? -(-total / MOUSE_NORMAL) : total / MOUSE_NORMAL;

    *remainder = total - pixels * MOUSE_NORMAL;

    return pixels;

}

static int getquarters(int relx, int rely)
{

    int size = util_max((relx < 0) ? -relx : relx, (rely < 0) ? -rely : rely);

    if (state.mousespeed <= MOUSE_NORMAL)
        return state.mousespeed;

    return MOUSE_NORMAL + (int)(state.mousespeed - MOUSE_NORMAL) * util_min(size - 1, MOUSE_RAMP) / MOUSE_RAMP;

}

static void onmousemove(struct message *message)
{

    struct event_mousemove *mousemove = message->data;
    int quarters = getquarters(mousemove->relx, mousemove->rely);
    int x = util_clamp(state.mouseposition.x + scalemove(mousemove->relx, quarters, &state.mouseremainder.x), 0, display.region.size.w);
    int y = util_clamp(state.mouseposition.y + scalemove(mousemove->rely, quarters, &state.mouseremainder.y), 0, display.region.size.h);
    unsigned int relayout = state.mousebuttonleft || state.mousebuttonright;

    state.mousemovement.x = x - state.mouseposition.x;
    state.mousemovement.y = y - state.mouseposition.y;
    state.mouseposition.x = x;
    state.mouseposition.y = y;

    sethover(gethoverwidgetat(state.mouseposition.x, state.mouseposition.y));
    render_setmouse(state.mouseposition.x, state.mouseposition.y, getmousetype(state.mouseposition.x, state.mouseposition.y));

    if (state.mousebuttonleft)
    {

        if (state.draggedthumb)
            dragthumb(state.draggedthumb);
        else if (state.focusedwidget)
        {

            if (state.focusedwidget->type == WIDGET_TYPE_BOX)
                setmark(state.focusedwidget, state.focusedwidget->markstart, getmarkindex(state.focusedwidget, state.mouseposition.x, state.mouseposition.y));

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

            if (state.layout == LAYOUT_DEFAULT && widget_isdragable(state.focusedwindow))
                translatewindow(state.focusedwindow, state.mousemovement.x, state.mousemovement.y);

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

            if (interactivewidget->type == WIDGET_TYPE_BOX)
            {

                unsigned int index = getmarkindex(interactivewidget, state.mouseposition.x, state.mouseposition.y);

                setmark(interactivewidget, index, index);

            }

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

static void bindinput(unsigned int on)
{

    channel_bind(EVENT_KEYPRESS, (on) ? onkeypress : 0);
    channel_bind(EVENT_KEYRELEASE, (on) ? onkeyrelease : 0);
    channel_bind(EVENT_MOUSEMOVE, (on) ? onmousemove : 0);
    channel_bind(EVENT_MOUSEPRESS, (on) ? onmousepress : 0);
    channel_bind(EVENT_MOUSESCROLL, (on) ? onmousescroll : 0);
    channel_bind(EVENT_MOUSERELEASE, (on) ? onmouserelease : 0);
    channel_bind(EVENT_VIDEOINFO, (on) ? onvideoinfo : 0);

}

static void onwmgrab(struct message *message)
{

    state.state = STATE_GRABBED;

    bindinput(0);
    channel_send(0, message->source, EVENT_WMACK, 0, 0);

}

static void render(unsigned int source, unsigned int count, void *data)
{

    parser_parse(source, "root", count, data);
    pool_loadresources();
    placewindows(source);
    purge(source);
    redraw(1);

}

static void onwmrenderdata(struct message *message)
{

    render(message->source, message->length, message->data);

}

static void onwmconfig(struct message *message)
{

    struct event_videoconf videoconf = state.videoconf;

    configlines(message->data, message->length);

    if (state.video)
        applyconfig(&videoconf);

}

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

    render(message->source, fs_read_path(1, message->data, data, 4096), data);

}

static void onwmungrab(struct message *message)
{

    state.state = STATE_NORMAL;

    bindinput(1);
    channel_send(0, message->source, EVENT_WMACK, 0, 0);
    setupvideo(channel_lookup(option_getstring("video-service")));

}

static void onwmunmap(struct message *message)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, message->source)))
    {

        struct widget *widget = current->data;

        widget->destroyed = 1;

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

    keys_init(&state.keys, KEYS_LAYOUT_QWERTY_US, (void *)0);
    state.videoconf.width = 1920;
    state.videoconf.height = 1080;
    state.videoconf.bpp = 4;
    state.mousespeed = MOUSE_NORMAL;
    pool_setup();
    setupwidgets();
    render_init();
    option_add("service", "wm");
    option_add("config", "initrd:data/config/wm.mq");
    option_add("keyboard-service", "keyboard");
    option_add("mouse-service", "mouse");
    option_add("video-service", "video");
    option_add("clipboard-service", "clipboard");
    bindinput(1);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_WMCONFIG, onwmconfig);
    channel_bind(EVENT_WMGRAB, onwmgrab);
    channel_bind(EVENT_WMREAD, onwmread);
    channel_bind(EVENT_WMRENDERDATA, onwmrenderdata);
    channel_bind(EVENT_WMRENDERFILE, onwmrenderfile);
    channel_bind(EVENT_WMUNGRAB, onwmungrab);
    channel_bind(EVENT_WMUNMAP, onwmunmap);

}

