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

#define STATE_NORMAL        0
#define STATE_GRABBED       1
#define TEXTBOX_SIZE        256

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

    struct event_videoconf videoconf;
    unsigned char black[768];

    videoconf.width = option_getdecimal("width");
    videoconf.height = option_getdecimal("height");
    videoconf.bpp = option_getdecimal("bpp");

    buffer_clear(black, 768);
    channel_send(0, video, EVENT_VIDEOCMAP, 768, &black);
    channel_send(0, video, EVENT_VIDEOCONF, sizeof (struct event_videoconf), &videoconf);
    channel_send(1, video, EVENT_INFO, 0, 0);
    channel_wait(1, video, EVENT_VIDEOINFO, 0, 0);

}

static struct widget *getinteractivewidgetat(int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_prev(current)))
    {

        struct widget *child = current->data;

        if (widget_isinteractive(child) && widget_intersects(child, x, y))
            return child;

    }

    return 0;

}

static struct widget *gethoverwidgetat(int x, int y)
{

    struct widget *widget = getinteractivewidgetat(x, y);

    if (widget)
    {

        struct widget *parent = pool_getwidgetbyid(widget->source, strpool_getstring(widget->attributes.in));

        if (parent && parent->type == WIDGET_TYPE_ITEM)
            return parent;

    }

    return widget;

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

    case WIDGET_TYPE_TEXTBOX:
        if (widget->attributes.mode != ATTR_MODE_READONLY)
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

    while ((current = pool_nextin(current, widget)))
        damageall(current->data);

}

static void translatewidget(struct widget *widget, int x, int y)
{

    damageall(widget);

    widget->position.x += x;
    widget->position.y += y;

    damageall(widget);

}

static void scalewidget(struct widget *widget, unsigned int w, unsigned int h)
{

    damageall(widget);

    widget->size.w = w;
    widget->size.h = h;

    damageall(widget);

}

static void scrollwidget(struct widget *widget, int x, int y)
{

    if (widget->attributes.overflow == ATTR_OVERFLOW_SCROLL || widget->attributes.overflow == ATTR_OVERFLOW_HSCROLL)
        widget->scroll.x += x;

    if (widget->attributes.overflow == ATTR_OVERFLOW_SCROLL || widget->attributes.overflow == ATTR_OVERFLOW_VSCROLL)
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

static void setfocus(struct widget *widget)
{

    if (state.focusedwidget)
    {

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

                unsigned int w8 = display.region.size.w / 8;
                unsigned int h8 = display.region.size.h / 8;

                widget->position.x = w8;
                widget->position.y = h8;
                widget->size.w = w8 * 3;
                widget->size.h = h8 * 6;

                setfocuswindow(widget);
                setfocus(0);

            }

            damageall(widget);

        }

    }

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

static void edittextbox(struct widget *widget, unsigned int id)
{

    struct widget *text = gettextchild(widget);
    char buffer[TEXTBOX_SIZE];
    unsigned int length;
    unsigned int cursor;

    if (!text)
        return;

    length = cstring_write_fmt(buffer, TEXTBOX_SIZE, 0, "%s\\0", strpool_getstring(text->attributes.label)) - 1;
    cursor = util_min(widget->attributes.cursor, length);

    switch (id)
    {

    case KEYS_KEY_ENTER:
        if (widget->attributes.onenter)
            sendevent(widget->source, 1, widget->attributes.onenter, buffer);

        return;

    case KEYS_KEY_BACKSPACE:
        if (cursor > 0)
        {

            buffer_copy(buffer + cursor - 1, buffer + cursor, length - cursor + 1);

            cursor--;

        }

        break;

    case KEYS_KEY_DELETE:
        if (cursor < length)
            buffer_copy(buffer + cursor, buffer + cursor + 1, length - cursor);

        break;

    case KEYS_KEY_CURSORLEFT:
        if (cursor > 0)
            cursor--;

        break;

    case KEYS_KEY_CURSORRIGHT:
        if (cursor < length)
            cursor++;

        break;

    case KEYS_KEY_HOME:
        cursor = 0;

        break;

    case KEYS_KEY_END:
        cursor = length;

        break;

    default:
        if (state.keys.code.length == 1 && state.keys.code.value[0] >= 0x20 && state.keys.code.value[0] < 0x7F && length + 1 < TEXTBOX_SIZE)
        {

            unsigned int i;

            for (i = length + 1; i > cursor; i--)
                buffer[i] = buffer[i - 1];

            buffer[cursor] = state.keys.code.value[0];
            cursor++;

        }

        break;

    }

    text->attributes.label = strpool_updatestring(text->attributes.label, buffer);
    widget->attributes.cursor = cursor;
    widget->followcursor = 1;

    damageall(widget);

}

static void movecursor(struct widget *widget)
{

    struct widget *text = gettextchild(widget);

    if (text)
    {

        struct text_font *font = pool_getfont(text->attributes.weight);
        char *label = strpool_getstring(text->attributes.label);
        unsigned int length = strpool_getcstringlength(text->attributes.label);
        int x = util_max(state.mouseposition.x - text->placement.position.x, 0);
        int y = util_clamp(state.mouseposition.y - text->placement.position.y, 0, util_max((int)text->placement.size.h - 1, 0));
        unsigned int cursor = text_getoffsetat(font, label, length, text->attributes.wrap, text->placement.size.w, text->rowstart.x, x, y);

        if (cursor < length)
        {

            struct text_info info = text_info(font, label, cursor, text->attributes.wrap, text->placement.size.w, text->rowstart.x);
            unsigned int index = label[cursor];

            if (x - info.lastrow.x > (int)font->atlas[index].width / 2)
                cursor++;

        }

        widget->attributes.cursor = cursor;

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

        }

        break;

    case WIDGET_TYPE_ITEM:
        if (state.mousebuttonleft)
        {

            struct widget *select = getselect(widget);
            struct widget *text = gettextchild(widget);

            if (select && select->attributes.onselect)
                sendevent(select->source, 1, select->attributes.onselect, (text) ? strpool_getstring(text->attributes.label) : "");

        }

        break;

    case WIDGET_TYPE_TEXTBOX:
        if (state.mousebuttonleft && widget->attributes.mode != ATTR_MODE_READONLY)
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

static void markwidget(struct widget *widget)
{

    if (widget->type == WIDGET_TYPE_TEXT)
    {

        int x0 = state.mousepressed.x - widget->placement.position.x;
        int y0 = state.mousepressed.y - widget->placement.position.y;
        int x1 = state.mouseposition.x - widget->placement.position.x;
        int y1 = state.mouseposition.y - widget->placement.position.y;

        widget->markstart = text_getoffsetat(pool_getfont(widget->attributes.weight), strpool_getstring(widget->attributes.label), strpool_getcstringlength(widget->attributes.label), widget->attributes.wrap, widget->placement.size.w, widget->rowstart.x, x0, y0);
        widget->markend = text_getoffsetat(pool_getfont(widget->attributes.weight), strpool_getstring(widget->attributes.label), strpool_getcstringlength(widget->attributes.label), widget->attributes.wrap, widget->placement.size.w, widget->rowstart.x, x1, y1);

    }

    else
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
        {

            struct widget *child = current->data;

            markwidget(child);

        }

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

            damageall(widget);
            pool_destroy(widget);

            current = 0;

        }

    }

}

static void redraw(unsigned int relayout)
{

    if (state.state == STATE_NORMAL && display.framebuffer)
    {

        if (relayout)
            render_place(state.rootwidget, &display.region);

        render_update(&display);
        render_undamage();

    }

}

static void onkeypress(struct message *message)
{

    struct event_keypress *keypress = message->data;
    unsigned int id = keys_getcode(&state.keys, keypress->scancode);

    if (id)
    {

        if ((state.keys.mod & KEYS_MOD_ALT))
        {

            switch (id)
            {

            case KEYS_KEY_Q:
                if ((state.keys.mod & KEYS_MOD_SHIFT))
                {

                    if (state.focusedwindow)
                        channel_send(0, state.focusedwindow->source, EVENT_INTERRUPT, 0, 0);

                }

                break;

            case KEYS_KEY_P:
                if ((state.keys.mod & KEYS_MOD_SHIFT))
                    launch("initrd:bin/wshell");

                break;

            case KEYS_KEY_PAGEUP:
                if (widget_isscrollable(state.focusedwidget))
                    scrollwidget(state.focusedwidget, 0, -16);

                break;

            case KEYS_KEY_PAGEDOWN:
                if (widget_isscrollable(state.focusedwidget))
                    scrollwidget(state.focusedwidget, 0, 16);

                break;

            }

        }

        else if (state.focusedwidget && state.focusedwidget->type == WIDGET_TYPE_TEXTBOX && state.focusedwidget->attributes.mode != ATTR_MODE_READONLY)
        {

            edittextbox(state.focusedwidget, id);

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

static void onmain(struct message *message)
{

    unsigned int keyboard = channel_lookup(option_getstring("keyboard-service"));
    unsigned int mouse = channel_lookup(option_getstring("mouse-service"));
    unsigned int video = channel_lookup(option_getstring("video-service"));

    call_announce(0, cstring_length(option_getstring("service")), option_getstring("service"));
    channel_send(0, keyboard, EVENT_LINK, 0, 0);
    channel_send(0, mouse, EVENT_LINK, 0, 0);
    setupvideo(video);
    channel_hold(0);
    channel_send(0, mouse, EVENT_UNLINK, 0, 0);
    channel_send(0, keyboard, EVENT_UNLINK, 0, 0);

}

static void onmousemove(struct message *message)
{

    struct event_mousemove *mousemove = message->data;
    int x = util_clamp(state.mouseposition.x + mousemove->relx, 0, display.region.size.w);
    int y = util_clamp(state.mouseposition.y + mousemove->rely, 0, display.region.size.h);

    state.mousemovement.x = x - state.mouseposition.x;
    state.mousemovement.y = y - state.mouseposition.y;
    state.mouseposition.x = x;
    state.mouseposition.y = y;

    sethover(gethoverwidgetat(state.mouseposition.x, state.mouseposition.y));
    render_setmouse(state.mouseposition.x, state.mouseposition.y, getmousetype(state.mouseposition.x, state.mouseposition.y));

    if (state.mousebuttonleft)
    {

        if (state.focusedwidget)
            markwidget(state.focusedwidget);

        if (!state.focusedwidget && state.focusedwindow)
        {

            if (widget_isdragable(state.focusedwindow))
                translatewidget(state.focusedwindow, state.mousemovement.x, state.mousemovement.y);

        }

    }

    if (state.mousebuttonright)
    {

        if (state.focusedwindow)
        {

            if (widget_isresizable(state.focusedwindow))
                scalewidget(state.focusedwindow, util_max((int)(state.focusedwindow->placement.size.w) + state.mousemovement.x, CONFIG_WINDOW_MIN_WIDTH), util_max((int)(state.focusedwindow->placement.size.h) + state.mousemovement.y, CONFIG_WINDOW_MIN_HEIGHT));

        }

    }

    redraw(state.mousebuttonleft || state.mousebuttonright);

}

static void onmousepress(struct message *message)
{

    struct event_mousepress *mousepress = message->data;
    struct widget *window = getwidgetoftypeat(WIDGET_TYPE_WINDOW, state.mouseposition.x, state.mouseposition.y);
    struct widget *interactivewidget = getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y);

    state.mousepressed.x = state.mouseposition.x;
    state.mousepressed.y = state.mouseposition.y;

    switch (mousepress->button)
    {

    case 1:
        state.mousebuttonleft = 1;

        setfocuswindow(window);
        setfocus(interactivewidget);

        if (interactivewidget)
        {

            clickwidget(interactivewidget);
            markwidget(interactivewidget);

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

        scrollwidget(scrollablewidget, 0, mousescroll->relz * 16);

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
    pool_setup();
    setupwidgets();
    render_init();
    option_add("service", "wm");
    option_add("width", "1920");
    option_add("height", "1080");
    option_add("bpp", "4");
    option_add("keyboard-service", "keyboard");
    option_add("mouse-service", "mouse");
    option_add("video-service", "video");
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_MOUSEMOVE, onmousemove);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_MOUSESCROLL, onmousescroll);
    channel_bind(EVENT_MOUSERELEASE, onmouserelease);
    channel_bind(EVENT_VIDEOINFO, onvideoinfo);
    channel_bind(EVENT_WMGRAB, onwmgrab);
    channel_bind(EVENT_WMMAP, onwmmap);
    channel_bind(EVENT_WMRENDERDATA, onwmrenderdata);
    channel_bind(EVENT_WMRENDERFILE, onwmrenderfile);
    channel_bind(EVENT_WMUNGRAB, onwmungrab);
    channel_bind(EVENT_WMUNMAP, onwmunmap);

}

