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

struct state
{

    unsigned int paused;
    struct util_position mouseposition;
    struct util_position mousemovement;
    struct util_position mousepressed;
    struct util_position mousereleased;
    unsigned int mousebuttonleft;
    unsigned int mousebuttonright;
    struct widget *rootwidget;
    struct widget *mousewidget;
    struct widget *hoverwidget;
    struct widget *focusedwindow;
    struct widget *focusedwidget;
    struct keys keys;

};

static struct blit_display display;
static struct state state;
static unsigned int linebuffer[3840];

static void setupvideo(void)
{

    struct ctrl_videosettings settings;
    unsigned char black[768];

    settings.width = option_getdecimal("width");
    settings.height = option_getdecimal("height");
    settings.bpp = option_getdecimal("bpp");

    buffer_clear(black, 768);

    if (!call_walk_absolute(FILE_L0, option_getstring("video")))
        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Video device not found: %s\n", option_getstring("video"));

    if (!call_walk_relative(FILE_L1, FILE_L0, "colormap"))
        return;

    if (!call_walk_relative(FILE_L2, FILE_L0, "ctrl"))
        return;

    call_notify(FILE_L1, EVENT_DATA, 768, black);
    call_notify(FILE_L2, EVENT_CONFIG, sizeof (struct ctrl_videosettings), &settings);

}

static struct widget *getinteractivewidgetat(int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_prev(current)))
    {

        struct widget *child = current->data;

        if (child == state.mousewidget)
            continue;

        if (widget_isinteractive(child) && widget_intersects(child, x, y))
            return child;

    }

    return 0;

}

static struct widget *getscrollablewidgetat(int x, int y)
{

    struct list_item *current = 0;

    while ((current = pool_prev(current)))
    {

        struct widget *child = current->data;

        if (child == state.mousewidget)
            continue;

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

    int x0 = util_clamp(widget->bb.x, 0, display.size.w);
    int y0 = util_clamp(widget->bb.y, 0, display.size.h);
    int x2 = util_clamp(widget->bb.x + widget->bb.w, 0, display.size.w);
    int y2 = util_clamp(widget->bb.y + widget->bb.h, 0, display.size.h);

    render_damage(x0, y0, x2, y2);

}

static void damageall(struct widget *widget)
{

    struct list_item *current = 0;

    damage(widget);

    while ((current = pool_nextin(current, widget)))
        damageall(current->data);

}

static void movewidget(struct widget *widget, int x, int y)
{

    damageall(widget);

    widget->bb.x = x;
    widget->bb.y = y;

    damageall(widget);

}

static void translatewidget(struct widget *widget, int x, int y)
{

    damageall(widget);

    widget->bb.x += x;
    widget->bb.y += y;

    damageall(widget);

}

static void scalewidget(struct widget *widget, unsigned int w, unsigned int h)
{

    damageall(widget);

    widget->bb.w = w;
    widget->bb.h = h;

    damageall(widget);

}

static void scrollwidget(struct widget *widget, int hamount, int vamount)
{

    if (widget->type == WIDGET_TYPE_LISTBOX)
    {

        struct widget_listbox *listbox = widget->data;

        if (listbox->overflow == ATTR_OVERFLOW_SCROLL || listbox->overflow == ATTR_OVERFLOW_HSCROLL)
            listbox->hscroll -= hamount;

        if (listbox->overflow == ATTR_OVERFLOW_SCROLL || listbox->overflow == ATTR_OVERFLOW_VSCROLL)
            listbox->vscroll -= vamount;

        damage(widget);

    }

    else if (widget->type == WIDGET_TYPE_TEXTBOX)
    {

        struct widget_textbox *textbox = widget->data;

        if (textbox->overflow == ATTR_OVERFLOW_SCROLL || textbox->overflow == ATTR_OVERFLOW_HSCROLL)
            textbox->hscroll -= hamount;

        if (textbox->overflow == ATTR_OVERFLOW_SCROLL || textbox->overflow == ATTR_OVERFLOW_VSCROLL)
            textbox->vscroll -= vamount;

        damage(widget);

    }

}

static void bump(struct widget *widget)
{

    if (widget)
    {

        pool_bump(widget);
        damageall(widget);

    }

}

static void bumpchildren(struct widget *widget)
{

    if (widget)
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget)))
            bump(current->data);

    }

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
            bump(state.mousewidget);

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
        bump(state.mousewidget);

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

            if (widget->bb.w == 0 && widget->bb.h == 0)
            {

                unsigned int w8 = display.size.w / 8;
                unsigned int h8 = display.size.h / 8;

                widget->bb.x = w8;
                widget->bb.y = h8;
                widget->bb.w = w8 * 3;
                widget->bb.h = h8 * 6;

                setfocuswindow(widget);
                setfocus(0);

            }

            damageall(widget);

        }

    }

}

static void sendevent(unsigned int source, unsigned int type, unsigned int action)
{

    if (source)
    {

        struct {struct event_wmevent wmevent; char data[128];} message;
        unsigned int length = strpool_getcstringlength(action) + 1;

        message.wmevent.type = type;
        message.wmevent.length = cstring_write_fmt2(message.data, 128, "%w\\0", 0, strpool_getstring(action), &length);

        channel_send_buffer(source, EVENT_WMEVENT, sizeof (struct event_wmevent) + message.wmevent.length, &message);

    }

    else
    {

        char *cmd = strpool_getstring(action);

        if (buffer_match(cmd, "run=", 4))
        {

            unsigned int channel = call_spawn_absolute(FILE_L0, FILE_PW, cmd + 4);

            if (channel)
                channel_send(channel, EVENT_MAIN);

        }

    }

}

static void clickwidget(struct widget *widget)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_WINDOW:
        if (state.mousebuttonleft)
        {

            if (util_intersects(state.mousewidget->bb.x, widget->bb.x + widget->bb.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->bb.x + widget->bb.w) && util_intersects(state.mousewidget->bb.y, widget->bb.y, widget->bb.y + CONFIG_WINDOW_BUTTON_HEIGHT))
                channel_send(widget->source, EVENT_TERM);

        }

        break;

    case WIDGET_TYPE_BUTTON:
        if (state.mousebuttonleft)
        {

            struct widget_button *button = widget->data;

            if (button->onclick)
                sendevent(widget->source, 1, button->onclick);

        }

        break;

    case WIDGET_TYPE_CHOICE:
        if (state.mousebuttonleft)
        {

            struct widget_choice *choice = widget->data;

            if (choice->onclick)
                sendevent(widget->source, 1, choice->onclick);

        }

        break;

    case WIDGET_TYPE_SELECT:
        if (state.mousebuttonleft)
        {

            struct widget_select *select = widget->data;

            if (select->onclick)
                sendevent(widget->source, 1, select->onclick);

        }

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        if (state.mousebuttonleft)
        {

            struct widget_textbutton *textbutton = widget->data;

            if (textbutton->onclick)
                sendevent(widget->source, 1, textbutton->onclick);

        }

        break;

    }

}

static void markwidget(struct widget *widget)
{

    if (widget->type == WIDGET_TYPE_TEXT)
    {

        struct widget_text *text = widget->data;
        int x0 = state.mousepressed.x - widget->bb.x;
        int y0 = state.mousepressed.y - widget->bb.y;
        int x1 = state.mouseposition.x - widget->bb.x;
        int y1 = state.mouseposition.y - widget->bb.y;

        text->markstart = text_getoffsetat(pool_getfont(text->weight), strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, widget->bb.w, text->offx, x0, y0);
        text->markend = text_getoffsetat(pool_getfont(text->weight), strpool_getstring(text->content), strpool_getcstringlength(text->content), text->wrap, widget->bb.w, text->offx, x1, y1);

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

static void destroy(struct widget *widget)
{

    if (state.hoverwidget == widget)
        sethover(0);

    if (state.focusedwidget == widget)
        setfocus(0);

    if (state.focusedwindow == widget)
        setfocuswindow(0);

    damageall(widget);
    pool_destroy(widget);

}

static void removedestroyed(unsigned int source)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, source)))
    {

        struct widget *widget = current->data;

        if (widget->state == WIDGET_STATE_DESTROYED)
        {

            current = current->prev;

            destroy(widget);

        }

    }

}

static void onkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keypress *keypress = mdata;
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
                        channel_send(state.focusedwindow->source, EVENT_TERM);

                }

                break;

            case KEYS_KEY_P:
                if ((state.keys.mod & KEYS_MOD_SHIFT))
                {

                    unsigned int channel = call_spawn_absolute(FILE_L0, FILE_PW, option_getstring("wshell"));

                    if (channel)
                        channel_send(channel, EVENT_MAIN);

                }

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

                channel_send_buffer(state.focusedwindow->source, EVENT_WMKEYPRESS, sizeof (struct event_wmkeypress), &wmkeypress);

            }

        }

    }

}

static void onkeyrelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    keys_getcode(&state.keys, keyrelease->scancode);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!call_walk_absolute(FILE_L0, option_getstring("keyboard")))
        PANIC();

    if (!call_walk_relative(FILE_G1, FILE_L0, "event"))
        PANIC();

    if (!call_walk_absolute(FILE_L0, option_getstring("mouse")))
        PANIC();

    if (!call_walk_relative(FILE_G2, FILE_L0, "event"))
        PANIC();

    if (!call_walk_absolute(FILE_L0, option_getstring("video")))
        PANIC();

    if (!call_walk_relative(FILE_G3, FILE_L0, "event"))
        PANIC();

    call_announce(option_getdecimal("wm-service"));
    call_link(FILE_G1, 8001);
    call_link(FILE_G2, 8002);
    call_link(FILE_G3, 8003);
    setupvideo();

    while (channel_process())
    {

        if (!state.paused)
        {

            render_place(state.rootwidget, 0, 0, 0, 0, display.size.w, display.size.h, 0, 0, display.size.w, display.size.h);
            render_cache();

            if (display.framebuffer)
            {

                render_update(&display, state.mousewidget->bb.x, state.mousewidget->bb.y);
                render_undamage();

            }

        }

    }

    call_unlink(FILE_G3);
    call_unlink(FILE_G2);
    call_unlink(FILE_G1);

}

static void onmousemove(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousemove *mousemove = mdata;
    int x = util_clamp(state.mouseposition.x + mousemove->relx, 0, display.size.w);
    int y = util_clamp(state.mouseposition.y + mousemove->rely, 0, display.size.h);

    state.mousemovement.x = x - state.mouseposition.x;
    state.mousemovement.y = y - state.mouseposition.y;
    state.mouseposition.x = x;
    state.mouseposition.y = y;

    sethover(getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y));

    if (state.mousewidget)
        movewidget(state.mousewidget, state.mouseposition.x, state.mouseposition.y);

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
                scalewidget(state.focusedwindow, util_max((int)(state.focusedwindow->bb.w) + state.mousemovement.x, CONFIG_WINDOW_MIN_WIDTH), util_max((int)(state.focusedwindow->bb.h) + state.mousemovement.y, CONFIG_WINDOW_MIN_HEIGHT));

        }

    }

}

static void onmousepress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousepress *mousepress = mdata;
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

}

static void onmousescroll(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousescroll *mousescroll = mdata;
    struct widget *scrollablewidget = getscrollablewidgetat(state.mouseposition.x, state.mouseposition.y);

    if (scrollablewidget)
        scrollwidget(scrollablewidget, 0, mousescroll->relz * 16);

    sethover(getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y));

}

static void onmouserelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mouserelease *mouserelease = mdata;

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

}

static void onvideomode(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;
    unsigned int factor = videomode->h / 320;

    blit_initdisplay(&display, videomode->framebuffer, videomode->w, videomode->h, videomode->bpp, linebuffer);
    render_damage(0, 0, videomode->w, videomode->h);
    pool_loadfont(factor);

    state.mouseposition.x = videomode->w / 4;
    state.mouseposition.y = videomode->h / 4;

    switch (factor)
    {

    case 0:
    case 1:
        util_initbox(&state.mousewidget->bb, state.mouseposition.x, state.mouseposition.y, 12, 16);

        break;

    case 2:
    default:
        util_initbox(&state.mousewidget->bb, state.mouseposition.x, state.mouseposition.y, 18, 24);

        break;

    }

}

static void onwmgrab(unsigned int source, void *mdata, unsigned int msize)
{

    state.paused = 1;

    channel_bind(EVENT_KEYPRESS, 0);
    channel_bind(EVENT_KEYRELEASE, 0);
    channel_bind(EVENT_MOUSEMOVE, 0);
    channel_bind(EVENT_MOUSEPRESS, 0);
    channel_bind(EVENT_MOUSESCROLL, 0);
    channel_bind(EVENT_MOUSERELEASE, 0);
    channel_bind(EVENT_VIDEOMODE, 0);
    channel_send(source, EVENT_WMACK);

}

static void onwmmap(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(source, EVENT_WMINIT);

}

static void onwmrenderdata(unsigned int source, void *mdata, unsigned int msize)
{

    parser_parse(source, "root", msize, mdata);
    placewindows(source);
    removedestroyed(source);
    bump(state.mousewidget);

}

static void onwmungrab(unsigned int source, void *mdata, unsigned int msize)
{

    state.paused = 0;

    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_MOUSEMOVE, onmousemove);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_MOUSESCROLL, onmousescroll);
    channel_bind(EVENT_MOUSERELEASE, onmouserelease);
    channel_bind(EVENT_VIDEOMODE, onvideomode);
    channel_send(source, EVENT_WMACK);
    setupvideo();

}

static void onwmunmap(unsigned int source, void *mdata, unsigned int msize)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, source)))
    {

        struct widget *widget = current->data;

        widget_setstate(widget, WIDGET_STATE_DESTROYED);

    }

    removedestroyed(source);

}

static void setupwidgets(void)
{

    char *data0 =
        "+ layout id \"root\"\n";
    char *data1 =
        "+ fill color \"FF202020\"\n"
        "+ image id \"mouse\" mimetype \"image/fudge-icon-mouse\"\n";
    char *data2 =
        "+ layout id \"desktop\" flow \"vertical\"\n"
        "  + layout id \"menu\" in \"desktop\" flow \"horizontal\"\n"
        "    + select id \"fudge-select\" in \"menu\" label \"Fudge\"\n"
        "      + layout id \"fudge-layout\" in \"fudge-select\" flow \"vertical-stretch\"\n"
        "        + choice in \"fudge-layout\" label \"About\" onclick \"run=/bin/wabout\"\n"
        "        + choice in \"fudge-layout\" label \"Settings\" onclick \"run=/bin/wsettings\"\n"
        "        + choice in \"fudge-layout\" label \"Reboot\" onclick \"run=/bin/reboot\"\n"
        "    + select id \"apps-select\" in \"menu\" label \"Apps\"\n"
        "      + layout id \"apps-layout\" in \"apps-select\" flow \"vertical-stretch\"\n"
        "        + choice in \"apps-layout\" label \"Shell\" onclick \"run=/bin/wshell\"\n"
        "        + choice in \"apps-layout\" label \"File Manager\" onclick \"run=/bin/wfile\"\n"
        "        + choice in \"apps-layout\" label \"Calculator\" onclick \"run=/bin/wcalc\"\n"
        "        + choice in \"apps-layout\" label \"Test\" onclick \"run=/bin/wtest\"\n";

    parser_parse(0, "", cstring_length(data0), data0);
    parser_parse(0, "root", cstring_length(data1), data1);
    parser_parse(0, "root", cstring_length(data2), data2);

    state.rootwidget = pool_getwidgetbyid(0, "root");
    state.mousewidget = pool_getwidgetbyid(0, "mouse");

    bump(state.mousewidget);

}

void init(void)
{

    keys_init(&state.keys, KEYS_LAYOUT_QWERTY_US, KEYS_MAP_US);
    pool_setup();
    setupwidgets();
    render_init();
    option_add("width", "1920");
    option_add("height", "1080");
    option_add("bpp", "4");
    option_add("wm-service", "12345");
    option_add("keyboard", "system:keyboard");
    option_add("mouse", "system:mouse");
    option_add("video", "system:video/if.0");
    option_add("wshell", "initrd:/bin/wshell");
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_MOUSEMOVE, onmousemove);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_MOUSESCROLL, onmousescroll);
    channel_bind(EVENT_MOUSERELEASE, onmouserelease);
    channel_bind(EVENT_VIDEOMODE, onvideomode);
    channel_bind(EVENT_WMGRAB, onwmgrab);
    channel_bind(EVENT_WMMAP, onwmmap);
    channel_bind(EVENT_WMRENDERDATA, onwmrenderdata);
    channel_bind(EVENT_WMUNGRAB, onwmungrab);
    channel_bind(EVENT_WMUNMAP, onwmunmap);

}

