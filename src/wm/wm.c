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
#define FONTS               4
#define SLIDER_STEP         0x0800
#define CLOCK_TIMER         1
#define ROOT                1
#define ROOTRECORDS         2
#define DISPLAYS            4
#define ANYDISPLAY          DISPLAYS
#define INPUTS              16
#define KEYMAPS             4
#define INPUT_KEYBOARD      1
#define INPUT_MOUSE         2
#define INPUT_TOUCH         3
#define DOUBLETAP_DRAG      0
#define DOUBLETAP_RIGHT     1
#define DEVICENAME          32
#define TOUCH_PADCOUNTS     1600
#define TOUCH_WHEELCOUNTS   24
#define TOUCH_TAPTICKS      12
#define LINEBUFFER          3840

struct listing
{

    struct message_readresponse response;
    struct record records[ROOTRECORDS];

};

struct statresult
{

    struct message_statresponse response;
    struct record record;

};

struct display
{

    struct message_videoconf videoconf;
    struct blit_display blit;
    unsigned int video;
    char device[DEVICENAME];

};

struct input
{

    char name[DEVICENAME];
    unsigned int type;
    unsigned int target;
    unsigned int speed;
    unsigned int doubletap;
    unsigned int keymap;

};

struct inputs
{

    struct input list[INPUTS];
    unsigned int count;

};

struct linked
{

    unsigned int targets[INPUTS];
    unsigned int count;

};

struct keymap
{

    char path[256];
    struct keys_map map[KEYS_MAPSIZE];
    unsigned int count;
    struct keys keys;

};

struct state
{

    unsigned int state;
    struct util_position mouseposition;
    struct util_position mousemovement;
    struct util_position mouseremainder;
    unsigned int mousebuttonleft;
    unsigned int mousebuttonright;
    struct widget *rootwidget;
    struct widget *hoverwidget;
    struct widget *focusedwindow;
    struct widget *focusedwidget;
    struct keys keys;
    unsigned int layout;
    unsigned int windows;
    struct display displays[DISPLAYS];
    struct widget *roots[DISPLAYS];
    unsigned int ndisplays;
    struct inputs inputs;
    struct linked linked;
    struct keymap keymaps[KEYMAPS];
    struct keys plainkeys;
    unsigned int active;
    unsigned int opaque;
    struct widget *draggedthumb;
    unsigned int clock;
    unsigned int timer;
    int scrollgrab;
    int slidergrab;

};

static struct state state;
static struct {unsigned int active; unsigned int id; int lastx; int lasty; unsigned int downtime; unsigned int travel; unsigned int fingers; unsigned int button; unsigned int time; unsigned int tapped; unsigned int taptime; unsigned int dragging; unsigned int secondtap; int accx; int accy; int accw; int motionx; int motiony; int wheel;} touch;
static unsigned int linebuffer[LINEBUFFER];
static char *fontn[FONTS] = {
    "initrd:data/font/ter-112n.pcf",
    "initrd:data/font/ter-114n.pcf",
    "initrd:data/font/ter-116n.pcf",
    "initrd:data/font/ter-118n.pcf",
};
static char *fontb[FONTS] = {
    "initrd:data/font/ter-112b.pcf",
    "initrd:data/font/ter-114b.pcf",
    "initrd:data/font/ter-116b.pcf",
    "initrd:data/font/ter-118b.pcf",
};

static void setupvideo(unsigned int video)
{

    unsigned char black[768];

    buffer_clear(black, 768);
    channel_send(0, video, MESSAGE_TYPE_VIDEOCMAP, 768, &black);
    channel_send(0, video, MESSAGE_TYPE_VIDEOCONF, sizeof (struct message_videoconf), &state.displays[0].videoconf);
    channel_send(1, video, MESSAGE_TYPE_INFO, 0, 0);
    channel_wait(1, video, MESSAGE_TYPE_VIDEOINFO, 0, 0);

}

static void setupdisplays(void)
{

    unsigned int i;

    for (i = 1; i < state.ndisplays; i++)
    {

        unsigned int video = channel_lookup(state.displays[i].device);

        state.displays[i].video = video;

        if (video)
        {

            channel_send(0, video, MESSAGE_TYPE_VIDEOCONF, sizeof (struct message_videoconf), &state.displays[i].videoconf);
            channel_send(1, video, MESSAGE_TYPE_INFO, 0, 0);
            channel_wait(1, video, MESSAGE_TYPE_VIDEOINFO, 0, 0);

        }

    }

}

static unsigned int findword(char *data, unsigned int count, unsigned int index)
{

    unsigned int start = 0;
    unsigned int i;

    for (i = 0; i <= count; i++)
    {

        if (i == count || data[i] == ' ' || data[i] == '\n')
        {

            if (i > start && !index--)
                return start;

            start = i + 1;

        }

    }

    return count;

}

static unsigned int measureword(char *data, unsigned int count, unsigned int start)
{

    unsigned int i;

    for (i = start; i < count; i++)
    {

        if (data[i] == ' ' || data[i] == '\n')
            break;

    }

    return i - start;

}

static unsigned int getnumber(char *data, unsigned int count, unsigned int index, unsigned int base, unsigned int fallback)
{

    unsigned int start = findword(data, count, index);
    unsigned int length = measureword(data, count, start);

    return (length) ? cstring_read_value(data + start, length, base) : fallback;

}

static void loadbackground(char *path, unsigned int length)
{

    if (length && length < 256)
    {

        char name[256];
        char size[64];
        unsigned int count;

        buffer_copy(name, path, length);

        name[length] = 0;
        count = system_feed(0, 0, size, 63, "mapimage %s mem:background", name);
        size[count] = 0;

        if (count)
        {

            char data[128];

            unsigned int i;

            parser_parse(0, "root", cstring_write_fmt(data, 128, 0, "= background mimetype \"image/raw\" %s", size), data);

            for (i = 1; i < DISPLAYS; i++)
                parser_parse(0, "root", cstring_write_fmt(data, 128, 0, "= background%u mimetype \"image/raw\" %s", &i, size), data);

            pool_loadresources();

        }

    }

}

static void setkey(struct keymap *keymap, char *line, unsigned int length)
{

    unsigned int scancode = getnumber(line, length, 0, 16, KEYS_MAPSIZE);

    if (scancode < KEYS_MAPSIZE)
    {

        unsigned int i;

        for (i = 0; i < 4; i++)
        {

            struct keys_code *code = &keymap->map[scancode].code[i];

            code->length = utf8_write((char *)code->value, 4, getnumber(line, length, i + 1, 16, 0));

        }

        keymap->count++;

    }

}

static void loadkeymap(struct keymap *keymap, char *path)
{

    static char data[0x2000];
    unsigned int count = 0;
    unsigned int linelength;
    char *line;

    buffer_clear(keymap->map, KEYS_MAPSIZE * sizeof (struct keys_map));
    cstring_write_fmt(keymap->path, 256, 0, "%s\\0", path);

    keymap->count = 0;

    if (cstring_length(path))
        count = system_feed(0, 0, data, 0x2000, "mq -query .keys -format \"{scancode} {levels}\" %s", path);

    for (line = data; line < data + count; line += linelength + 1)
    {

        linelength = buffer_findbyte(line, data + count - line, '\n');

        if (linelength)
            setkey(keymap, line, linelength);

    }

    keys_init(&keymap->keys, KEYS_LAYOUT_QWERTY_US, (keymap->count) ? keymap->map : (void *)0);

}

/* the nth keyboard keeps its map in the nth slot, reloaded only when its path changes */
static unsigned int getkeymap(char *path)
{

    unsigned int slot = 0;
    unsigned int i;

    for (i = 0; i + 1 < state.inputs.count; i++)
    {

        if (state.inputs.list[i].type == INPUT_KEYBOARD)
            slot++;

    }

    if (slot >= KEYMAPS)
        return KEYMAPS;

    if (!cstring_match(state.keymaps[slot].path, path) || !state.keymaps[slot].keys.map)
        loadkeymap(&state.keymaps[slot], path);

    return slot;

}

static void addinput(unsigned int type, char *rest, unsigned int length)
{

    unsigned int nameoffset = findword(rest, length, 0);
    unsigned int namelength = measureword(rest, length, nameoffset);
    unsigned int valueoffset = findword(rest, length, 1);
    unsigned int valuelength = measureword(rest, length, valueoffset);

    if (state.inputs.count < INPUTS && namelength && namelength < DEVICENAME)
    {

        struct input *input = &state.inputs.list[state.inputs.count++];

        buffer_clear(input, sizeof (struct input));
        cstring_write_fmt(input->name, DEVICENAME, 0, "%w\\0", rest + nameoffset, &namelength);

        input->type = type;
        input->speed = MOUSE_NORMAL;
        input->keymap = KEYMAPS;

        switch (type)
        {

        case INPUT_KEYBOARD:
            if (valuelength && valuelength < 256)
            {

                char path[256];

                cstring_write_fmt(path, 256, 0, "%w\\0", rest + valueoffset, &valuelength);
                input->keymap = getkeymap(path);

            }

            break;

        case INPUT_MOUSE:
            input->speed = util_clamp(getnumber(rest, length, 1, 10, MOUSE_NORMAL), 1, MOUSE_FASTEST);

            break;

        case INPUT_TOUCH:
            input->doubletap = (valuelength == 5 && buffer_match(rest + valueoffset, "right", 5)) ? DOUBLETAP_RIGHT : DOUBLETAP_DRAG;

            break;

        }

    }

}

static void configline(char *line, unsigned int length)
{

    unsigned int offset = findword(line, length, 0);
    unsigned int count = measureword(line, length, offset);
    unsigned int start = (offset + count < length) ? offset + count + 1 : length;
    char *word = line + offset;
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

    else if (count == 8 && buffer_match(word, "displays", 8))
    {

        state.ndisplays = 0;

    }

    else if (count == 7 && buffer_match(word, "display", 7))
    {

        if (state.ndisplays < DISPLAYS)
        {

            struct display *display = &state.displays[state.ndisplays];
            unsigned int deviceoffset = findword(rest, length, 3);
            unsigned int devicelength = measureword(rest, length, deviceoffset);

            display->videoconf.width = getnumber(rest, length, 0, 10, display->videoconf.width);
            display->videoconf.height = getnumber(rest, length, 1, 10, display->videoconf.height);
            display->videoconf.bpp = getnumber(rest, length, 2, 10, display->videoconf.bpp);

            if (devicelength && devicelength < DEVICENAME)
                cstring_write_fmt(display->device, DEVICENAME, 0, "%w\\0", rest + deviceoffset, &devicelength);
            else
                cstring_write_fmt(display->device, DEVICENAME, 0, "video:%u\\0", &state.ndisplays);

            state.ndisplays++;

        }

    }

    else if (count == 6 && buffer_match(word, "inputs", 6))
    {

        state.inputs.count = 0;

    }

    else if (count == 8 && buffer_match(word, "keyboard", 8))
    {

        addinput(INPUT_KEYBOARD, rest, length);

    }

    else if (count == 5 && buffer_match(word, "mouse", 5))
    {

        addinput(INPUT_MOUSE, rest, length);

    }

    else if (count == 8 && buffer_match(word, "touchpad", 8))
    {

        addinput(INPUT_TOUCH, rest, length);

    }

    else if (count == 12 && buffer_match(word, "transparency", 12))
    {

        unsigned int i;

        state.opaque = (length >= 3 && buffer_match(rest, "off", 3));

        for (i = 0; i < DISPLAYS; i++)
            state.displays[i].blit.opaque = state.opaque;

    }

    else if (count == 5 && buffer_match(word, "color", 5))
    {

        unsigned int space = buffer_findbyte(rest, length, ' ');

        if (space < length && space < 64)
        {

            char name[64];

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

static void loadreference(char *data, char *config, char *key, char *query, char *format)
{

    char path[256];
    unsigned int length = buffer_findbyte(path, system_feed(0, 0, path, 255, "mq -query %s %s", key, config), '\n');

    if (!length)
        return;

    path[length] = '\0';

    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query %s -format \"%s\" %s", query, format, path));

}

static void loadconfig(char *config)
{

    static char data[0x4000];

    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .general -format \"layout {layout}\" %s", config));
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .general -format \"background {background}\" %s", config));
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .general -format \"transparency {transparency}\" %s", config));
    configlines("displays", 8);
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .displays -format \"display {width} {height} {bpp} {device}\" %s", config));
    configlines("inputs", 6);
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .keyboards -format \"keyboard {device} {map}\" %s", config));
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .mice -format \"mouse {device} {speed}\" %s", config));
    configlines(data, system_feed(0, 0, data, 0x4000, "mq -query .touchpads -format \"touchpad {device} {doubletap}\" %s", config));
    loadreference(data, config, ".general.theme", ".colors", "color {name} {value}");
    configlines("bindings", 8);
    loadreference(data, config, ".bindings", ".bindings", "binding {key} {mod} {action} {command}");

}

static unsigned int islinked(struct linked *linked, unsigned int target)
{

    unsigned int i;

    for (i = 0; i < linked->count; i++)
    {

        if (linked->targets[i] == target)
            return 1;

    }

    return 0;

}

static struct input *findinput(unsigned int source)
{

    unsigned int i;

    for (i = 0; i < state.inputs.count; i++)
    {

        if (state.inputs.list[i].target == source)
            return &state.inputs.list[i];

    }

    return 0;

}

static void applyinputs(void)
{

    struct linked linked;
    unsigned int i;

    linked.count = 0;

    for (i = 0; i < state.inputs.count; i++)
    {

        struct input *input = &state.inputs.list[i];

        input->target = channel_lookup(input->name);

        if (input->target && !islinked(&linked, input->target))
            linked.targets[linked.count++] = input->target;

    }

    for (i = 0; i < state.linked.count; i++)
    {

        if (!islinked(&linked, state.linked.targets[i]))
            channel_send(0, state.linked.targets[i], MESSAGE_TYPE_UNLINK, 0, 0);

    }

    for (i = 0; i < linked.count; i++)
    {

        if (!islinked(&state.linked, linked.targets[i]))
            channel_send(0, linked.targets[i], MESSAGE_TYPE_LINK, 0, 0);

    }

    buffer_copy(state.linked.targets, linked.targets, linked.count * sizeof (unsigned int));

    state.linked.count = linked.count;

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

    if (widget->type != WIDGET_TYPE_ITEM)
    {

        struct widget *parent;

        parent = getparent(widget);

        return (parent && parent->type == WIDGET_TYPE_ITEM) ? parent : 0;

    }

    return widget;

}

static struct widget *getselecteditem(struct widget *widget)
{

    struct widget *item = getitem(widget);

    if (!item)
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            if (child->type == WIDGET_TYPE_ITEM && child->anchor)
                return child;

        }

        return 0;

    }

    return item;

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

    if (widget)
    {

        unsigned int button;

        switch (widget->type)
        {

        case WIDGET_TYPE_BUTTON:
        case WIDGET_TYPE_CHECKBOX:
        case WIDGET_TYPE_ITEM:
        case WIDGET_TYPE_SELECT:
        case WIDGET_TYPE_SLIDER:
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

static struct util_region getactiveregion(void)
{

    return state.displays[state.active].blit.region;

}

static struct util_region getarea(void)
{

    struct util_region area = util_region(0, 0, 0, 0);
    unsigned int i;

    for (i = 0; i < DISPLAYS; i++)
    {

        struct util_region *region = &state.displays[i].blit.region;

        if (state.displays[i].blit.framebuffer)
        {

            area.size.w = util_max(area.size.w, region->position.x + region->size.w);
            area.size.h = util_max(area.size.h, region->position.y + region->size.h);

        }

    }

    return area;

}

static unsigned int finddisplayat(int x, int y)
{

    unsigned int i;

    for (i = 0; i < DISPLAYS; i++)
    {

        struct util_region *region = &state.displays[i].blit.region;

        if (state.displays[i].blit.framebuffer && x >= region->position.x && x < region->position.x + (int)region->size.w && y >= region->position.y && y < region->position.y + (int)region->size.h)
            return i;

    }

    return DISPLAYS;

}

static unsigned int getdisplayat(int x, int y)
{

    unsigned int display = finddisplayat(x, y);

    return (display == DISPLAYS) ? 0 : display;

}

static unsigned int getdisplayof(struct widget *widget)
{

    return getdisplayat(widget->position.x + (int)widget->size.w / 2, widget->position.y + (int)widget->size.h / 2);

}

static void update(void)
{

    unsigned int i;

    for (i = 0; i < DISPLAYS; i++)
    {

        struct display *display = &state.displays[i];
        struct util_region updated;

        if (!display->blit.framebuffer)
            continue;

        updated = render_update(&display->blit);

        if (updated.size.w && updated.size.h)
        {

            struct message_videoflush videoflush;

            videoflush.x = updated.position.x;
            videoflush.y = updated.position.y;
            videoflush.width = updated.size.w;
            videoflush.height = updated.size.h;

            channel_send(0, display->video, MESSAGE_TYPE_VIDEOFLUSH, sizeof (struct message_videoflush), &videoflush);

        }

    }

    render_undamage();

}

static void damage(struct widget *widget)
{

    struct util_region area = getarea();
    int x0 = util_clamp(widget->position.x, 0, area.size.w);
    int y0 = util_clamp(widget->position.y, 0, area.size.h);
    int x2 = util_clamp(widget->position.x + widget->size.w, 0, area.size.w);
    int y2 = util_clamp(widget->position.y + widget->size.h, 0, area.size.h);

    render_damage(x0, y0, x2, y2);

    x0 = util_clamp(widget->placement.position.x, 0, area.size.w);
    y0 = util_clamp(widget->placement.position.y, 0, area.size.h);
    x2 = util_clamp(widget->placement.position.x + widget->placement.size.w, 0, area.size.w);
    y2 = util_clamp(widget->placement.position.y + widget->placement.size.h, 0, area.size.h);

    render_damage(x0, y0, x2, y2);

}

static void damageall(struct widget *widget)
{

    damage(widget);

    if (widget->type != WIDGET_TYPE_WINDOW)
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
            damageall(current->data);

    }

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

        struct {struct message_wmevent wmevent; char data[EVENTDATA_SIZE];} message;
        unsigned int count = cstring_write_fmt(message.data, EVENTDATA_SIZE - 1, 0, "%s", strpool_getstring(action));

        count += buffer_write(message.data, EVENTDATA_SIZE - 1, text, utf8_fit(text, cstring_length(text), EVENTDATA_SIZE - 1 - count), count);
        message.data[count] = '\0';
        message.wmevent.type = type;
        message.wmevent.length = count + 1;

        channel_send(0, source, MESSAGE_TYPE_WMEVENT, sizeof (struct message_wmevent) + message.wmevent.length, &message);

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

    if (parent && parent->attributes.onselect)
    {

        struct list_item *current = 0;
        unsigned int count = 0;
        char text[16];

        while ((current = pool_nextin(current, parent)))
        {

            struct widget *child = current->data;

            if (child->type == WIDGET_TYPE_ITEM && child->selected)
                count++;

        }

        cstring_write_fmt(text, 16, 0, "%u\\0", &count);
        sendevent(parent->source, 1, parent->attributes.onselect, text);

    }

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

    if (index)
    {

        unsigned int offset;
        struct widget *text;

        text = pool_gettextat(widget, index - 1);
        offset = pool_gettextoffset(widget, index - 1);

        return (text) ? index - 1 - offset + utf8_prev(strpool_getstring(text->attributes.label), offset + 1) : index - 1;

    }

    return 0;

}

static unsigned int stepright(struct widget *widget, unsigned int index)
{

    unsigned int offset = pool_gettextoffset(widget, index);
    struct widget *text = pool_gettextat(widget, index);

    if (text && offset < gettextlength(text))
    {

        unsigned int codepoint;

        return index + utf8_read(strpool_getstring(text->attributes.label) + offset, gettextlength(text) - offset, &codepoint);

    }

    return index;

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

                struct util_region region = getactiveregion();
                struct util_size limit = util_size(region.size.w - region.size.w / 8 * 2, region.size.h - region.size.h / 8 * 2);

                widget->position.x = region.position.x + region.size.w / 8;
                widget->position.y = region.position.y + region.size.h / 8;
                widget->size = render_getwindowsize(widget, &limit);
                widget->size.w = util_min(util_max(widget->size.w, region.size.w / 2), limit.w);
                widget->size.h = util_min(util_max(widget->size.h, region.size.h / 2), limit.h);

                widget->serial = ++state.windows;

                setfocuswindow(widget);
                setfocus(0);
                damageall(widget);

            }

            else if (state.layout == LAYOUT_DEFAULT)
            {

                struct util_region region = getactiveregion();
                struct util_size minimum = render_getwindowminsize(widget, &region.size);

                if (widget->size.w < minimum.w || widget->size.h < minimum.h)
                    scalewidget(widget, util_max(widget->size.w, minimum.w), util_max(widget->size.h, minimum.h));

            }

        }

    }

}

static struct widget *getadjacentwindow(unsigned int serial, int direction, unsigned int display)
{

    struct list_item *current = 0;
    struct widget *nearest = 0;
    int distance = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;
        int d = ((int)widget->serial - (int)serial) * direction;

        if (widget->type == WIDGET_TYPE_WINDOW && !widget->destroyed && d > 0 && (!nearest || d < distance) && (display == ANYDISPLAY || getdisplayof(widget) == display))
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
        window = getadjacentwindow(state.focusedwindow->serial, (older) ? -1 : 1, state.active);

    if (!window)
        window = (older) ? getadjacentwindow(state.windows + 1, -1, state.active) : getadjacentwindow(0, 1, state.active);

    if (window)
    {

        setfocuswindow(window);
        setfocus(0);

    }

}

static void swapwindow(void)
{

    struct widget *first = getadjacentwindow(state.windows + 1, -1, state.active);

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

static unsigned int getadjacentdisplay(int direction)
{

    unsigned int i;

    for (i = 1; i < DISPLAYS; i++)
    {

        unsigned int display = (state.active + ((direction > 0) ? i : DISPLAYS - i)) % DISPLAYS;

        if (state.displays[display].blit.framebuffer)
            return display;

    }

    return state.active;

}

static void switchdisplay(unsigned int display)
{

    struct util_region region;

    if (display == state.active)
        return;

    state.active = display;
    region = getactiveregion();
    state.mouseposition.x = region.position.x + region.size.w / 2;
    state.mouseposition.y = region.position.y + region.size.h / 2;

    setfocuswindow(getadjacentwindow(state.windows + 1, -1, display));
    setfocus(0);

}

static void movetodisplay(unsigned int display)
{

    struct widget *window = state.focusedwindow;

    if (window && display != state.active)
    {

        struct util_region from = state.displays[getdisplayof(window)].blit.region;
        struct util_region to = state.displays[display].blit.region;
        unsigned int w = util_min(window->size.w, to.size.w);
        unsigned int h = util_min(window->size.h, to.size.h);
        int x = util_clamp(to.position.x + window->position.x - from.position.x, to.position.x, to.position.x + (int)(to.size.w - w));
        int y = util_clamp(to.position.y + window->position.y - from.position.y, to.position.y, to.position.y + (int)(to.size.h - h));

        placewindow(window, x, y, w, h);

        window->serial = ++state.windows;

        switchdisplay(display);

    }

}

static void tiledisplay(unsigned int display)
{

    struct util_region region = state.displays[display].blit.region;
    struct widget *menu = (display) ? 0 : pool_getwidgetbyid(0, "menu");
    int top = (menu) ? menu->placement.position.y + (int)menu->placement.size.h : region.position.y;
    unsigned int w = region.size.w / 2;
    struct widget *first = getadjacentwindow(state.windows + 1, -1, display);

    if (first)
    {

        unsigned int h = util_max(region.position.y + (int)region.size.h - top, 0);
        struct widget *widget;
        unsigned int count = 0;
        unsigned int i = 0;

        for (widget = getadjacentwindow(first->serial, -1, display); widget; widget = getadjacentwindow(widget->serial, -1, display))
            count++;

        placewindow(first, region.position.x, top, w, h);

        for (widget = getadjacentwindow(first->serial, -1, display); widget; widget = getadjacentwindow(widget->serial, -1, display), i++)
            placewindow(widget, region.position.x + w, top + h * i / count, region.size.w - w, h * (i + 1) / count - h * i / count);

    }

}

static void tilewindows(void)
{

    unsigned int i;

    for (i = 0; i < DISPLAYS; i++)
    {

        if (state.displays[i].blit.framebuffer)
            tiledisplay(i);

    }

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

static unsigned int getmarkstart(struct widget *widget)
{

    return util_min(util_min(widget->markstart, widget->markend), pool_gettextlength(widget));

}

static unsigned int getmarkend(struct widget *widget)
{

    return util_min(util_max(widget->markstart, widget->markend), pool_gettextlength(widget));

}

static unsigned int hasmarked(struct widget *widget)
{

    return getmarkstart(widget) < getmarkend(widget);

}

static unsigned int removemarked(struct widget *widget, unsigned int cursor)
{

    unsigned int start = getmarkstart(widget);
    unsigned int end = getmarkend(widget);

    if (start < end)
    {

        struct list_item *current = 0;
        unsigned int base = 0;

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

    return cursor;

}

static unsigned int inserttext(struct widget *widget, unsigned int index, char *data, unsigned int count)
{

    unsigned int offset = pool_gettextoffset(widget, index);
    struct widget *text = pool_gettextat(widget, index);
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

    unsigned int offset = pool_gettextoffset(widget, index);
    struct widget *text = pool_gettextat(widget, index);
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

    struct widget *text = pool_gettextat(widget, widget->attributes.cursor);
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

    start = getmarkstart(widget);
    end = getmarkend(widget);

    if (start < end && (count || id == KEYS_KEY_BACKSPACE || id == KEYS_KEY_DELETE))
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

        setmark(widget, (hasmarked(widget)) ? widget->markstart : previous, cursor);

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

static unsigned int createclipboard(unsigned int target, char *path)
{

    char *name = path + fs_dirlength(path);
    unsigned int parent = (target) ? fs_walkparent(1, target, path) : 0;

    if (parent)
    {

        unsigned int id;

        id = fs_walk(1, target, 0, path);

        if (id && !fs_remove(1, target, parent, id))
            return 0;

        return fs_create(1, target, parent, name, cstring_length(name));

    }

    return 0;

}

static unsigned int copymarked(struct widget *widget)
{

    unsigned int start = getmarkstart(widget);
    unsigned int end = getmarkend(widget);

    if (start < end)
    {

        struct list_item *current = 0;
        unsigned int base = 0;
        unsigned int count = 0;
        unsigned int target;
        unsigned int id;

        target = fs_auth(option_getstring("clipboard"));
        id = createclipboard(target, option_getstring("clipboard"));

        if (!id)
            return 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            if (child->type == WIDGET_TYPE_TEXT)
            {

                char *label = strpool_getstring(child->attributes.label);
                unsigned int length = gettextlength(child);
                unsigned int offset = util_clamp((int)start - (int)base, 0, length);
                unsigned int stop = util_clamp((int)end - (int)base, 0, length);

                if (offset < stop)
                    count += fs_write_all(1, target, id, label + offset, stop - offset, count);

                base += length;

            }

        }

        return 1;

    }

    return 0;

}

static void pastetext(struct widget *widget)
{

    char *path = option_getstring("clipboard");
    unsigned int cursor = util_min(widget->attributes.cursor, pool_gettextlength(widget));
    unsigned int multiline = widget_isscrollable(widget);
    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;

    if (id && pool_gettextat(widget, cursor))
    {

        unsigned int position = 0;
        unsigned int stopped = 0;
        char data[MESSAGE_SIZE];
        unsigned int count;

        cursor = removemarked(widget, cursor);

        while (!stopped && (count = fs_read(1, target, id, data, MESSAGE_SIZE, position)))
        {

            unsigned int length = (multiline) ? count : buffer_findbyte(data, count, '\n');

            cursor += inserttext(widget, cursor, data, length);
            position += count;

            if (length < count)
                stopped = 1;

        }

        widget->attributes.cursor = cursor;
        widget->followcursor = 1;

        notifychange(widget);
        damageall(widget);

    }

}

static void movecursor(struct widget *widget)
{

    struct widget *first = pool_gettextat(widget, 0);
    struct widget *last = pool_gettextat(widget, pool_gettextlength(widget));

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

static void sendselect(struct widget *item)
{

    struct widget *select = getselect(item);
    struct widget *text = gettextchild(item);

    if (select && select->attributes.onselect)
        sendevent(select->source, 1, select->attributes.onselect, (text) ? strpool_getstring(text->attributes.label) : "");

}

static void setslider(struct widget *widget, int value)
{

    unsigned int clamped = util_clamp(value, 0, 0xFFFF);

    if (clamped != widget->attributes.value)
    {

        widget->attributes.value = clamped;

        damage(widget);

        if (widget->attributes.onchange)
        {

            char text[16];

            cstring_write_fmt(text, 16, 0, "%u\\0", &clamped);
            sendevent(widget->source, 1, widget->attributes.onchange, text);

        }

    }

}

static void dragslider(struct widget *widget)
{

    int travel = (int)widget->placement.size.w - CONFIG_SLIDER_KNOB_WIDTH;

    if (travel > 0)
        setslider(widget, util_clamp(state.mouseposition.x - state.slidergrab - widget->placement.position.x, 0, travel) * 0xFFFF / travel);

}

static void grabslider(struct widget *widget)
{

    struct util_region knob = render_getsliderknob(widget);

    state.slidergrab = (util_region_intersects(&knob, state.mouseposition.x, state.mouseposition.y)) ? state.mouseposition.x - knob.position.x : CONFIG_SLIDER_KNOB_WIDTH / 2;

    dragslider(widget);

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

    case WIDGET_TYPE_SLIDER:
        if (state.mousebuttonleft)
            grabslider(widget);

        break;

    case WIDGET_TYPE_ITEM:
        if (state.mousebuttonleft)
            sendselect(widget);

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
                channel_send(0, widget->source, MESSAGE_TYPE_INTERRUPT, 0, 0);

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
        setfocuswindow(getadjacentwindow(state.windows + 1, -1, state.active));

}

static void damageregion(struct util_region *region)
{

    if (region->size.w && region->size.h)
        render_damage(region->position.x, region->position.y, region->position.x + region->size.w, region->position.y + region->size.h);

}

static void copyregion(struct util_region *to, struct util_region *from)
{

    to->position.x = from->position.x;
    to->position.y = from->position.y;
    to->size.w = from->size.w;
    to->size.h = from->size.h;

}

static unsigned int isdifferent(struct util_region *a, struct util_region *b)
{

    return a->position.x != b->position.x || a->position.y != b->position.y || a->size.w != b->size.w || a->size.h != b->size.h;

}

static void damagechanged(void)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;
        struct util_region visible = util_region_intersection(&widget->placement, &widget->clip);
        struct util_region cursor = util_region(widget->cursorposition.x, widget->cursorposition.y, widget->cursorsize.w, widget->cursorsize.h);

        cursor = util_region_intersection(&cursor, &visible);

        if (widget->changed || isdifferent(&visible, &widget->drawn) || widget->scroll.x != widget->drawnscroll.x || widget->scroll.y != widget->drawnscroll.y || widget->scrollmax != widget->drawnscrollmax)
        {

            damageregion(&widget->drawn);
            damageregion(&visible);

            copyregion(&widget->drawn, &visible);
            widget->drawnscroll.x = widget->scroll.x;
            widget->drawnscroll.y = widget->scroll.y;
            widget->drawnscrollmax = widget->scrollmax;
            widget->changed = 0;

        }

        if (isdifferent(&cursor, &widget->drawncursor))
        {

            damageregion(&widget->drawncursor);
            damageregion(&cursor);

            copyregion(&widget->drawncursor, &cursor);

        }

    }

}

static void placeroots(void)
{

    unsigned int i;

    for (i = 1; i < DISPLAYS; i++)
    {

        if (state.roots[i] && state.displays[i].blit.framebuffer)
            render_place(state.roots[i], &state.displays[i].blit.region, &state.displays[i].blit.region);

    }

}

static void redraw(unsigned int relayout)
{

    if (state.state == STATE_NORMAL && state.displays[0].blit.framebuffer)
    {

        if (relayout)
        {

            struct util_region area = getarea();

            if (state.layout == LAYOUT_TILING)
                tilewindows();

            render_place(state.rootwidget, &state.displays[0].blit.region, &area);
            placeroots();
            damagechanged();
            render_setmouse(state.mouseposition.x, state.mouseposition.y, getmousetype(state.mouseposition.x, state.mouseposition.y));

        }

        update();

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
    case WIDGET_TYPE_SLIDER:
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

static unsigned int collectitems(struct widget *widget, struct widget **list, unsigned int count)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        if (count < NUM_FOCUSABLE && child->type == WIDGET_TYPE_ITEM && child->attributes.mode != ATTR_MODE_DISABLED)
            list[count++] = child;

        count = collectitems(child, list, count);

    }

    return count;

}

static void stepitem(struct widget *select, unsigned int forward)
{

    static struct widget *list[NUM_FOCUSABLE];
    unsigned int count = collectitems(select, list, 0);
    unsigned int current;

    if (!count)
        return;

    for (current = 0; current < count; current++)
    {

        if (list[current] == state.hoverwidget)
            break;

    }

    if (current == count)
        current = (forward) ? 0 : count - 1;
    else if (forward && current + 1 < count)
        current++;
    else if (!forward && current > 0)
        current--;

    sethover(list[current]);
    showwidget(list[current]);

}

static void chooseitem(struct widget *select)
{

    struct widget *item = (state.hoverwidget) ? getitem(state.hoverwidget) : 0;

    if (!item || getselect(item) != select)
        return;

    setfocus(item);
    sendselect(item);

    if (item->attributes.onclick)
        sendevent(item->source, 1, item->attributes.onclick, "");

}

static unsigned int markall(struct widget *widget)
{

    if (widget && widget->type == WIDGET_TYPE_BOX)
    {

        unsigned int total;

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

    return 0;

}

static unsigned int dobinding(struct bind_binding *binding)
{

    switch (binding->action)
    {

    case BIND_ACTION_CLOSE:
        if (state.focusedwindow)
            channel_send(0, state.focusedwindow->source, MESSAGE_TYPE_INTERRUPT, 0, 0);

        break;

    case BIND_ACTION_NEXT:
        cyclewindow(1);

        break;

    case BIND_ACTION_PREVIOUS:
        cyclewindow(0);

        break;

    case BIND_ACTION_NEXTDISPLAY:
        switchdisplay(getadjacentdisplay(1));

        break;

    case BIND_ACTION_PREVIOUSDISPLAY:
        switchdisplay(getadjacentdisplay(-1));

        break;

    case BIND_ACTION_MOVENEXTDISPLAY:
        movetodisplay(getadjacentdisplay(1));

        break;

    case BIND_ACTION_MOVEPREVIOUSDISPLAY:
        movetodisplay(getadjacentdisplay(-1));

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

static unsigned int isslidekey(unsigned int id)
{

    return id == KEYS_KEY_CURSORLEFT || id == KEYS_KEY_CURSORRIGHT || id == KEYS_KEY_HOME || id == KEYS_KEY_END;

}

static void slide(struct widget *widget, unsigned int id)
{

    int value = widget->attributes.value;

    switch (id)
    {

    case KEYS_KEY_CURSORLEFT:
        setslider(widget, value - SLIDER_STEP);

        break;

    case KEYS_KEY_CURSORRIGHT:
        setslider(widget, value + SLIDER_STEP);

        break;

    case KEYS_KEY_HOME:
        setslider(widget, 0);

        break;

    case KEYS_KEY_END:
        setslider(widget, 0xFFFF);

        break;

    }

}

static struct keys *usekeymap(unsigned int source)
{

    struct input *input = findinput(source);
    struct keys *keys = (input && input->keymap < KEYMAPS) ? &state.keymaps[input->keymap].keys : &state.plainkeys;

    state.keys.map = keys->map;
    state.keys.altgr = keys->altgr;

    return &state.keys;

}

static void onkeypress(struct message *message)
{

    struct message_keypress *keypress = message_validate(MESSAGE_TYPE_KEYPRESS, message->data, message->length);

    if (keypress)
    {

        unsigned int id = keys_getcode(usekeymap(message->source), keypress->scancode);
        struct widget *selecteditem = (state.focusedwidget) ? getselecteditem(state.focusedwidget) : 0;
        struct bind_binding *binding = bind_find(id, state.keys.mod);

        if (id && (!binding || !dobinding(binding)))
        {

            if (state.focusedwidget && state.focusedwidget->type == WIDGET_TYPE_SELECT && (id == KEYS_KEY_CURSORUP || id == KEYS_KEY_CURSORDOWN))
            {

                stepitem(state.focusedwidget, id == KEYS_KEY_CURSORDOWN);

            }

            else if (state.focusedwidget && state.focusedwidget->type == WIDGET_TYPE_SELECT && id == KEYS_KEY_ENTER)
            {

                chooseitem(state.focusedwidget);

            }

            else if (selecteditem && id == KEYS_KEY_ENTER && selecteditem->attributes.onenter)
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

            else if (state.focusedwidget && state.focusedwidget->type == WIDGET_TYPE_SLIDER && isslidekey(id))
            {

                slide(state.focusedwidget, id);

            }

            else if (state.focusedwidget && state.focusedwidget->type == WIDGET_TYPE_BOX && (state.keys.mod & KEYS_MOD_SHIFT) && ismarkkey(id))
            {

                extendmark(state.focusedwidget, id);

            }

            else
            {

                if (state.focusedwindow)
                {

                    struct message_wmkeypress wmkeypress;

                    wmkeypress.id = state.keys.id;
                    wmkeypress.scancode = keypress->scancode;
                    wmkeypress.unicode = 0;
                    wmkeypress.length = state.keys.code.length;

                    buffer_copy(&wmkeypress.unicode, state.keys.code.value, wmkeypress.length);
                    wmkeypress.keymod = state.keys.mod;

                    channel_send(0, state.focusedwindow->source, MESSAGE_TYPE_WMKEYPRESS, sizeof (struct message_wmkeypress), &wmkeypress);

                }

            }

        }

        redraw(1);

    }

}

static void onkeyrelease(struct message *message)
{

    struct message_keyrelease *keyrelease = message_validate(MESSAGE_TYPE_KEYRELEASE, message->data, message->length);

    if (keyrelease)
    {

        unsigned int id = keys_getcode(usekeymap(message->source), keyrelease->scancode);

        if (id && state.focusedwindow)
        {

            struct message_wmkeyrelease wmkeyrelease;

            wmkeyrelease.id = id;
            wmkeyrelease.scancode = keyrelease->scancode;
            wmkeyrelease.keymod = state.keys.mod;
            wmkeyrelease.unicode = 0;
            wmkeyrelease.length = 0;

            channel_send(0, state.focusedwindow->source, MESSAGE_TYPE_WMKEYRELEASE, sizeof (struct message_wmkeyrelease), &wmkeyrelease);

        }

    }

}

static void requestclock(unsigned int ms)
{

    struct message_timerrequest timerrequest;

    timerrequest.id = CLOCK_TIMER;
    timerrequest.ms = ms;
    timerrequest.repeat = 0;

    channel_send(0, state.timer, MESSAGE_TYPE_TIMERREQUEST, sizeof (struct message_timerrequest), &timerrequest);

}

static void showclock(void)
{

    struct message_clockinfo clockinfo;

    channel_send(1, state.clock, MESSAGE_TYPE_INFO, 0, 0);

    if (channel_wait(1, state.clock, MESSAGE_TYPE_CLOCKINFO, sizeof (struct message_clockinfo), &clockinfo))
    {

        unsigned char hours;
        char data[64];

        hours = (clockinfo.hours % 12) ? clockinfo.hours % 12 : 12;

        parser_parse(0, "root", cstring_write_fmt(data, 64, 0, "= clock label \"%2c:%2c %s\"\n", &hours, &clockinfo.minutes, (clockinfo.hours < 12) ? "AM" : "PM"), data);
        redraw(1);
        requestclock((60 - util_min(clockinfo.seconds, 59)) * 1000);

    }

}

static void hideclock(void)
{

    char *data = "- clock-panel\n";

    parser_parse(0, "root", cstring_length(data), data);
    purge(0);

}

static void ontimerexpire(struct message *message)
{

    struct message_timerexpire timerexpire;

    buffer_copy(&timerexpire, message->data, sizeof (struct message_timerexpire));

    if (message->source == state.timer && timerexpire.id == CLOCK_TIMER)
        showclock();

}

static unsigned int walk(unsigned int parent, unsigned int length, char *path)
{

    if (!parent || parent == ROOT)
    {

        unsigned int offset = 0;

        while (offset < length)
        {

            char *cp = path + offset;
            unsigned int cl = buffer_findbyte(cp, length - offset, '/');

            if (cl > 2 || (cl && !buffer_match(cp, "..", cl)))
                return 0;

            offset += cl + 1;

        }

        return ROOT;

    }

    return 0;

}

static void onreadrequest(struct message *message)
{

    struct message_readrequest *request = message_validate(MESSAGE_TYPE_READREQUEST, message->data, message->length);
    struct listing listing;

    if (request)
    {

        unsigned int c = 0;
        unsigned int i;

        for (i = request->offset; request->id == ROOT && i < ROOTRECORDS && (c + 1) * sizeof (struct record) <= request->count; i++)
            record_init(&listing.records[c++], ROOT, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");

        listing.response.count = c * sizeof (struct record);
        listing.response.buffer = 0;

        channel_send(0, message->source, MESSAGE_TYPE_READRESPONSE, sizeof (struct message_readresponse) + listing.response.count, &listing);

    }

}

static void onstatrequest(struct message *message)
{

    struct message_statrequest *request = message_validate(MESSAGE_TYPE_STATREQUEST, message->data, message->length);
    struct statresult result;

    if (request)
    {

        record_init(&result.record, ROOT, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");

        result.response.count = (request->id == ROOT) ? sizeof (struct record) : 0;

        channel_send(0, message->source, MESSAGE_TYPE_STATRESPONSE, sizeof (struct message_statresponse) + result.response.count, &result);

    }

}

static void onwalkrequest(struct message *message)
{

    struct message_walkrequest *request = message_validate(MESSAGE_TYPE_WALKREQUEST, message->data, message->length);
    struct message_walkresponse response;

    if (request)
    {

        response.id = walk(request->parent, request->length, (char *)(request + 1));

        channel_send(0, message->source, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response);

    }

}

static void onmain(struct message *message)
{

    loadconfig(option_getstring("config"));

    if (!state.ndisplays)
        state.ndisplays = 1;

    state.displays[0].video = channel_lookup(state.displays[0].device);

    if (call_announce(0, cstring_length(option_getstring("service")), option_getstring("service")) != MESSAGE_OK)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Could not announce service: %s\n", option_getstring("service"));

        return;

    }

    applyinputs();
    setupvideo(state.displays[0].video);
    setupdisplays();

    state.clock = channel_lookup(option_getstring("clock-service"));
    state.timer = channel_lookup(option_getstring("timer-service"));

    if (state.clock && state.timer)
        showclock();
    else
        hideclock();

    channel_hold(0);

    if (state.clock && state.timer)
        requestclock(0);

    state.inputs.count = 0;

    applyinputs();

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

static int scalemove(int total)
{

    return (total < 0) ? -(-total / MOUSE_NORMAL) : total / MOUSE_NORMAL;

}

static int getquarters(int relx, int rely, unsigned int speed)
{

    if (speed > MOUSE_NORMAL)
    {

        int size = util_max((relx < 0) ? -relx : relx, (rely < 0) ? -rely : rely);

        return MOUSE_NORMAL + (int)(speed - MOUSE_NORMAL) * util_min(size - 1, MOUSE_RAMP) / MOUSE_RAMP;

    }

    return speed;

}

static void releasebutton(unsigned int button)
{

    switch (button)
    {

    case 1:
        state.mousebuttonleft = 0;
        state.draggedthumb = 0;

        break;

    case 2:
        state.mousebuttonright = 0;

        break;

    }

}

static void movemouse(int x, int y, unsigned int buttons)
{

    unsigned int display = finddisplayat(x, y);
    unsigned int relayout;

    if (display == DISPLAYS)
    {

        struct util_region region = getactiveregion();

        x = util_clamp(x, region.position.x, region.position.x + (int)region.size.w - 1);
        y = util_clamp(y, region.position.y, region.position.y + (int)region.size.h - 1);

    }

    else
    {

        state.active = display;

    }

    /* a release dropped by a full mailbox shows up as a cleared bit */
    if (state.mousebuttonleft && !(buttons & 1))
        releasebutton(1);

    if (state.mousebuttonright && !(buttons & 2))
        releasebutton(2);

    relayout = state.mousebuttonleft || state.mousebuttonright;
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

            if (state.focusedwidget->type == WIDGET_TYPE_SLIDER)
                dragslider(state.focusedwidget);

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

        if (!state.draggedthumb && !state.focusedwidget && state.focusedwindow)
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

                struct util_region region = getactiveregion();
                struct util_size minimum = render_getwindowminsize(state.focusedwindow, &region.size);

                scalewidget(state.focusedwindow, util_max((int)(state.focusedwindow->placement.size.w) + state.mousemovement.x, minimum.w), util_max((int)(state.focusedwindow->placement.size.h) + state.mousemovement.y, minimum.h));

            }

        }

    }

    redraw(relayout);

}

static void moverelative(int relx, int rely, unsigned int buttons, unsigned int speed)
{

    int quarters = getquarters(relx, rely, speed);
    int totalx = state.mouseremainder.x + relx * quarters;
    int totaly = state.mouseremainder.y + rely * quarters;
    int pixelsx = scalemove(totalx);
    int pixelsy = scalemove(totaly);
    struct util_region area = getarea();
    int x = util_clamp(state.mouseposition.x + pixelsx, 0, (int)area.size.w - 1);
    int y = util_clamp(state.mouseposition.y + pixelsy, 0, (int)area.size.h - 1);
    state.mouseremainder.x = totalx - pixelsx * MOUSE_NORMAL;
    state.mouseremainder.y = totaly - pixelsy * MOUSE_NORMAL;

    movemouse(x, y, buttons);

}

static void onmousemove(struct message *message)
{

    struct message_mousemove *mousemove = message_validate(MESSAGE_TYPE_MOUSEMOVE, message->data, message->length);

    if (mousemove)
    {

        struct input *input = findinput(message->source);

        moverelative(mousemove->relx, mousemove->rely, mousemove->buttons, (input) ? input->speed : MOUSE_NORMAL);

    }

}

/* an absolute position covers the first display, the built-in screen a touchscreen sits on */
static void onmouseposition(struct message *message)
{

    struct message_mouseposition *mouseposition = message_validate(MESSAGE_TYPE_MOUSEPOSITION, message->data, message->length);

    if (mouseposition)
    {

        struct util_region region = (state.displays[0].blit.framebuffer) ? state.displays[0].blit.region : getarea();
        int x = region.position.x + (int)((unsigned int)mouseposition->x * (region.size.w - 1) / 65535);
        int y = region.position.y + (int)((unsigned int)mouseposition->y * (region.size.h - 1) / 65535);

        state.mouseremainder.x = 0;
        state.mouseremainder.y = 0;

        movemouse(x, y, mouseposition->buttons);

    }

}

static void grabthumb(void)
{

    struct widget *widget = getscrollablewidgetat(state.mouseposition.x, state.mouseposition.y);

    state.draggedthumb = (widget && widget_hasscrollbar(widget) && util_region_intersects(&widget->thumb, state.mouseposition.x, state.mouseposition.y)) ? widget : 0;

    if (state.draggedthumb)
        state.scrollgrab = state.mouseposition.y - widget->thumb.position.y;

}

static void pressbutton(unsigned int button)
{

    struct widget *window = getwidgetoftypeat(WIDGET_TYPE_WINDOW, state.mouseposition.x, state.mouseposition.y);
    struct widget *interactivewidget = getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y);

    {

        switch (button)
        {

        case 1:
            state.mousebuttonleft = 1;

            setfocuswindow(window);
            grabthumb();

            if (!state.draggedthumb)
                setfocus(interactivewidget);

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

}

static void onmousepress(struct message *message)
{

    struct message_mousepress *mousepress = message_validate(MESSAGE_TYPE_MOUSEPRESS, message->data, message->length);

    if (mousepress)
        pressbutton(mousepress->button);

}

static void scrollby(int relz)
{

    struct widget *scrollablewidget = getscrollablewidgetat(state.mouseposition.x, state.mouseposition.y);

    if (scrollablewidget)
        scrollwidget(scrollablewidget, relz * 16);

    sethover(gethoverwidgetat(state.mouseposition.x, state.mouseposition.y));

    redraw(1);

}

static void onmousescroll(struct message *message)
{

    struct message_mousescroll *mousescroll = message_validate(MESSAGE_TYPE_MOUSESCROLL, message->data, message->length);

    if (mousescroll)
        scrollby(mousescroll->relz);

}

static void onmouserelease(struct message *message)
{

    struct message_mouserelease *mouserelease = message_validate(MESSAGE_TYPE_MOUSERELEASE, message->data, message->length);

    if (mouserelease)
    {

        releasebutton(mouserelease->button);
        redraw(1);

    }

}

/* surface units to pointer counts, the whole range being counts; the remainder is kept so slow movement is not lost */
static int scaletouch(int delta, int *accumulator, unsigned int range, unsigned int counts)
{

    int quotient;

    if (!range)
        return 0;

    *accumulator += delta * (int)counts;
    quotient = (*accumulator >= 0) ? *accumulator / (int)range : -(-*accumulator / (int)range);
    *accumulator -= quotient * (int)range;

    return quotient;

}

static unsigned int getbuttons(void)
{

    return (state.mousebuttonleft ? 1 : 0) | (state.mousebuttonright ? 2 : 0);

}

/* motion is gathered per driver tick and moved at once, so the acceleration sees what a mouse report would */
static void flushtouch(void)
{

    while (touch.motionx || touch.motiony)
    {

        int x = util_clamp(touch.motionx, -127, 127);
        int y = util_clamp(touch.motiony, -127, 127);

        moverelative(x, y, getbuttons(), MOUSE_NORMAL);

        touch.motionx -= x;
        touch.motiony -= y;

    }

    if (touch.wheel)
        scrollby(touch.wheel);

    touch.wheel = 0;

}

static void clicktouch(unsigned int button)
{

    flushtouch();
    pressbutton(button);
    releasebutton(button);
    redraw(1);

}

/* one finger moves the pointer, two scroll; a short touch that barely moves is a tap, one finger a left click and two a right click; a touch soon after a one-finger tap either holds the left button until it lifts (drag, a quick second tap being a double click) or, set to right, makes that second tap a right click; the surface's own button is the left button */
static void ontouchcontact(struct message *message)
{

    struct message_touchcontact *contact = message_validate(MESSAGE_TYPE_TOUCHCONTACT, message->data, message->length);
    struct input *input = findinput(message->source);

    if (!contact)
        return;

    if (contact->time != touch.time)
        flushtouch();

    touch.time = contact->time;

    if (contact->button != touch.button)
    {

        flushtouch();

        if (contact->button)
            pressbutton(1);
        else
            releasebutton(1);

        touch.button = contact->button;

        redraw(1);

    }

    if (!touch.active)
    {

        if (contact->tip)
        {

            touch.active = 1;
            touch.id = contact->id;
            touch.lastx = contact->x;
            touch.lasty = contact->y;
            touch.downtime = contact->time;
            touch.travel = 0;
            touch.fingers = (contact->count) ? contact->count : 1;
            touch.accx = 0;
            touch.accy = 0;
            touch.accw = 0;

            touch.secondtap = touch.tapped && contact->time - touch.taptime <= TOUCH_TAPTICKS && !touch.button;

            if (touch.secondtap && !(input && input->doubletap == DOUBLETAP_RIGHT))
            {

                touch.dragging = 1;
                touch.secondtap = 0;

                pressbutton(1);
                redraw(1);

            }

            touch.tapped = 0;

        }

        return;

    }

    if (contact->count > touch.fingers)
        touch.fingers = contact->count;

    if (contact->id != touch.id)
        return;

    if (contact->tip)
    {

        int dx = (int)contact->x - touch.lastx;
        int dy = (int)contact->y - touch.lasty;

        touch.lastx = contact->x;
        touch.lasty = contact->y;
        touch.travel += ((dx < 0) ? -dx : dx) + ((dy < 0) ? -dy : dy);

        if (touch.fingers >= 2)
        {

            touch.wheel -= scaletouch(dy, &touch.accw, contact->ymax, TOUCH_WHEELCOUNTS);

        }

        else
        {

            touch.motionx += scaletouch(dx, &touch.accx, contact->xmax, TOUCH_PADCOUNTS);
            touch.motiony += scaletouch(dy, &touch.accy, contact->xmax, TOUCH_PADCOUNTS);

        }

        return;

    }

    touch.active = 0;

    flushtouch();

    if (touch.dragging)
    {

        releasebutton(1);
        redraw(1);

        touch.dragging = 0;

        return;

    }

    if (contact->time - touch.downtime <= TOUCH_TAPTICKS && touch.travel <= contact->xmax / 50u && !touch.button)
    {

        clicktouch((touch.fingers >= 2 || touch.secondtap) ? 2 : 1);

        touch.tapped = touch.fingers < 2 && !touch.secondtap;
        touch.taptime = contact->time;

    }

}

static unsigned int finddisplay(unsigned int video)
{

    unsigned int i;

    for (i = 0; i < DISPLAYS; i++)
    {

        if (state.displays[i].video == video)
            return i;

    }

    return DISPLAYS;

}

static int getorigin(unsigned int index)
{

    int x = 0;
    unsigned int i;

    for (i = 0; i < index; i++)
    {

        if (state.displays[i].blit.framebuffer)
            x += state.displays[i].blit.region.size.w;

    }

    return x;

}

static void movewindows(struct util_region *from, int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;
        int cx = widget->position.x + (int)widget->size.w / 2;
        int cy = widget->position.y + (int)widget->size.h / 2;

        if (widget->type == WIDGET_TYPE_WINDOW && !widget->destroyed && cx >= from->position.x && cx < from->position.x + (int)from->size.w && cy >= from->position.y && cy < from->position.y + (int)from->size.h)
            translatewindow(widget, x, y);

    }

}

static void movedisplay(unsigned int index, int x)
{

    struct blit_display *blit = &state.displays[index].blit;

    if (blit->framebuffer && blit->region.position.x != x)
    {

        struct util_region from = blit->region;

        movewindows(&from, x - from.position.x, 0);

        blit->region.position.x = x;

    }

}

static void arrangedisplays(void)
{

    unsigned int i;

    for (i = 1; i < DISPLAYS; i++)
        movedisplay(i, getorigin(i));

}

static void setupdisplay(unsigned int index, struct message_videoinfo *videoinfo)
{

    struct blit_display *blit = &state.displays[index].blit;
    int x = getorigin(index);

    movedisplay(index, x);
    blit_initdisplay(blit, (void *)(unsigned long)videoinfo->framebuffer, x, 0, videoinfo->width, videoinfo->height, videoinfo->bpp, linebuffer);

    blit->opaque = state.opaque;

}

static void removedisplay(unsigned int index)
{

    struct display *display = &state.displays[index];

    if (display->blit.framebuffer)
    {

        struct util_region from = display->blit.region;

        movewindows(&from, -from.position.x, -from.position.y);

        display->blit.framebuffer = 0;

        if (state.active == index)
            state.active = 0;

    }

    display->video = 0;

}

static void applyconfig(struct message_videoconf *videoconf, unsigned int video)
{

    unsigned int i;

    if (!state.ndisplays)
        state.ndisplays = 1;

    state.displays[0].video = channel_lookup(state.displays[0].device);

    applyinputs();
    setupdisplays();

    for (i = 1; i < DISPLAYS; i++)
    {

        if (i >= state.ndisplays || !state.displays[i].video)
            removedisplay(i);

    }

    arrangedisplays();

    if (state.displays[0].video != video || videoconf->width != state.displays[0].videoconf.width || videoconf->height != state.displays[0].videoconf.height || videoconf->bpp != state.displays[0].videoconf.bpp)
    {

        setupvideo(state.displays[0].video);

    }

    else
    {

        struct util_region area = getarea();

        render_damage(0, 0, area.size.w, area.size.h);
        redraw(1);

    }

}

static void onvideoinfo(struct message *message)
{

    struct message_videoinfo videoinfo;
    unsigned int index = finddisplay(message->source);
    unsigned int factor;
    unsigned int lineheight;

    buffer_copy(&videoinfo, message->data, sizeof (struct message_videoinfo));

    if (index == DISPLAYS || videoinfo.width > LINEBUFFER)
        return;

    if (index)
    {

        struct util_region area;

        if (!videoinfo.framebuffer)
            return;

        setupdisplay(index, &videoinfo);

        area = getarea();

        render_damage(0, 0, area.size.w, area.size.h);
        redraw(1);

        return;

    }

    factor = util_min(videoinfo.height / 320, FONTS - 1);
    lineheight = 12 + factor * 4;

    setupdisplay(0, &videoinfo);
    arrangedisplays();
    pool_loadfont(0, fontn[factor]);
    pool_setfont(0, lineheight);
    pool_loadfont(1, fontb[factor]);
    pool_setfont(1, lineheight);
    render_setunit(factor + 1);

    state.active = 0;
    state.mouseposition.x = videoinfo.width / 4;
    state.mouseposition.y = videoinfo.height / 4;

    render_setmouse(state.mouseposition.x, state.mouseposition.y, BLIT_MOUSE_ARROW);

    {

        struct util_region area = getarea();

        render_damage(0, 0, area.size.w, area.size.h);
        render_place(state.rootwidget, &state.displays[0].blit.region, &area);
        placeroots();

    }

    update();

}

static void bindinput(unsigned int on)
{

    channel_bind(MESSAGE_TYPE_KEYPRESS, (on) ? onkeypress : 0);
    channel_bind(MESSAGE_TYPE_KEYRELEASE, (on) ? onkeyrelease : 0);
    channel_bind(MESSAGE_TYPE_MOUSEMOVE, (on) ? onmousemove : 0);
    channel_bind(MESSAGE_TYPE_MOUSEPOSITION, (on) ? onmouseposition : 0);
    channel_bind(MESSAGE_TYPE_TOUCHCONTACT, (on) ? ontouchcontact : 0);
    channel_bind(MESSAGE_TYPE_MOUSEPRESS, (on) ? onmousepress : 0);
    channel_bind(MESSAGE_TYPE_MOUSESCROLL, (on) ? onmousescroll : 0);
    channel_bind(MESSAGE_TYPE_MOUSERELEASE, (on) ? onmouserelease : 0);
    channel_bind(MESSAGE_TYPE_VIDEOINFO, (on) ? onvideoinfo : 0);

}

static void onwmgrab(struct message *message)
{

    state.state = STATE_GRABBED;

    bindinput(0);
    channel_send(0, message->source, MESSAGE_TYPE_WMACK, 0, 0);

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

static void onconfig(struct message *message)
{

    struct message_videoconf videoconf = state.displays[0].videoconf;
    unsigned int video = state.displays[0].video;

    configlines(message->data, message->length);

    if (video)
        applyconfig(&videoconf, video);

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
                    channel_send(0, source, MESSAGE_TYPE_WMTEXT, util_min(length - offset, MESSAGE_SIZE), label + offset);

            }

        }

    }

    channel_send(0, source, MESSAGE_TYPE_WMTEXT, 0, 0);

}

static void onwmrenderfile(struct message *message)
{

    char data[4096];

    render(message->source, fs_read_path(1, message->data, data, 4096), data);

}

static void onwmflush(struct message *message)
{

    char id[64];
    struct widget *widget;

    cstring_write_fmt(id, 64, 0, "%s\\0", message->data);

    widget = pool_getwidgetbyid(message->source, id);

    if (widget && widget->type == WIDGET_TYPE_MEDIA)
    {

        struct util_region visible = util_region_intersection(&widget->placement, &widget->clip);

        damageregion(&visible);
        redraw(0);

    }

    channel_send(0, message->source, MESSAGE_TYPE_WMACK, 0, 0);

}

static void onwmungrab(struct message *message)
{

    state.state = STATE_NORMAL;
    state.keys.mod = 0;

    bindinput(1);
    channel_send(0, message->source, MESSAGE_TYPE_WMACK, 0, 0);
    setupvideo(state.displays[0].video);

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
    unsigned int i;

    parser_parse(0, "", cstring_length(data0), data0);
    parser_parse(0, "root", fs_read_path(1, "initrd:data/alfi/wm.alfi", data1, 4096), data1);
    pool_loadresources();

    state.rootwidget = pool_getwidgetbyid(0, "root");

    for (i = 1; i < DISPLAYS; i++)
    {

        char data2[128];
        char id[16];

        parser_parse(0, "", cstring_write_fmt(data2, 128, 0, "+ layout id \"root%u\" flow \"stretch\"\n", &i), data2);
        parser_parse(0, "", cstring_write_fmt(data2, 128, 0, "+ media id \"background%u\" in \"root%u\" source \"mem:background\"\n", &i, &i), data2);
        cstring_write_fmt(id, 16, 0, "root%u\\0", &i);

        state.roots[i] = pool_getwidgetbyid(0, id);

    }

}

void init(void)
{

    keys_init(&state.keys, KEYS_LAYOUT_QWERTY_US, (void *)0);
    keys_init(&state.plainkeys, KEYS_LAYOUT_QWERTY_US, (void *)0);
    state.displays[0].videoconf.width = 1920;
    state.displays[0].videoconf.height = 1080;
    state.displays[0].videoconf.bpp = 4;
    cstring_write_fmt(state.displays[0].device, DEVICENAME, 0, "video:0\\0");
    state.ndisplays = 1;
    pool_setup();
    setupwidgets();
    render_init();
    option_add("service", "wm");
    option_add("config", "initrd:data/config/wm.mq");
    option_add("clock-service", "clock:0");
    option_add("timer-service", "timer:0");
    option_add("clipboard", "mem:clipboard");
    bindinput(1);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_READREQUEST, onreadrequest);
    channel_bind(MESSAGE_TYPE_STATREQUEST, onstatrequest);
    channel_bind(MESSAGE_TYPE_TIMEREXPIRE, ontimerexpire);
    channel_bind(MESSAGE_TYPE_WALKREQUEST, onwalkrequest);
    channel_bind(MESSAGE_TYPE_CONFIG, onconfig);
    channel_bind(MESSAGE_TYPE_WMFLUSH, onwmflush);
    channel_bind(MESSAGE_TYPE_WMGRAB, onwmgrab);
    channel_bind(MESSAGE_TYPE_WMREAD, onwmread);
    channel_bind(MESSAGE_TYPE_WMRENDERDATA, onwmrenderdata);
    channel_bind(MESSAGE_TYPE_WMRENDERFILE, onwmrenderfile);
    channel_bind(MESSAGE_TYPE_WMUNGRAB, onwmungrab);
    channel_bind(MESSAGE_TYPE_WMUNMAP, onwmunmap);

}

