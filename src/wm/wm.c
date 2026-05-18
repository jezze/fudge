#include <fudge.h>
#include <abi.h>
#include <hash.h>
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
#define STATE_UNGRABBED     2

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
    struct widget *mousewidget;
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
    channel_send_buffer(0, video, EVENT_VIDEOCMAP, 768, &black);
    channel_send_buffer(0, video, EVENT_VIDEOCONF, sizeof (struct event_videoconf), &videoconf);
    channel_wait(0, video, EVENT_VIDEOINFO);

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

static void movewidget(struct widget *widget, int x, int y)
{

    damageall(widget);

    widget->position.x = x;
    widget->position.y = y;

    damageall(widget);

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

static void sendevent(unsigned int source, unsigned int type, unsigned int action)
{

    if (source)
    {

        struct {struct event_wmevent wmevent; char data[128];} message;
        unsigned int length = strpool_getcstringlength(action) + 1;

        message.wmevent.type = type;
        message.wmevent.length = cstring_write_fmt2(message.data, 128, 0, "%w\\0", strpool_getstring(action), &length);

        channel_send_buffer(0, source, EVENT_WMEVENT, sizeof (struct event_wmevent) + message.wmevent.length, &message);

    }

    else
    {

        char *cmd = strpool_getstring(action);

        if (buffer_match(cmd, "run=", 4))
        {

            unsigned int target = fs_spawn(1, cmd + 4);

            if (target)
            {

                channel_send_fmt1(1, target, EVENT_OPTION, "pwd=%s\n", option_getstring("pwd"));
                channel_send(1, target, EVENT_MAIN);
                channel_send(1, target, EVENT_TERM);

            }

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

            if (util_intersects(state.mousewidget->placement.position.x, widget->placement.position.x + widget->placement.size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->placement.position.x + widget->placement.size.w) && util_intersects(state.mousewidget->placement.position.y, widget->placement.position.y, widget->placement.position.y + CONFIG_WINDOW_BUTTON_HEIGHT))
            {

                channel_send(0, widget->source, EVENT_WMCLOSE);
                channel_send(0, widget->source, EVENT_TERM);

            }

        }

        break;

    }

    if (state.mousebuttonleft)
    {

        if (widget->attributes.onclick)
            sendevent(widget->source, 1, widget->attributes.onclick);

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
                    {

                        channel_send(0, state.focusedwindow->source, EVENT_WMCLOSE);
                        channel_send(0, state.focusedwindow->source, EVENT_TERM);

                    }

                }

                break;

            case KEYS_KEY_P:
                if ((state.keys.mod & KEYS_MOD_SHIFT))
                {

                    unsigned int target = fs_spawn(1, option_getstring("wshell"));

                    if (target)
                    {

                        channel_send(1, target, EVENT_MAIN);
                        channel_send(1, target, EVENT_TERM);

                    }

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

                channel_send_buffer(0, state.focusedwindow->source, EVENT_WMKEYPRESS, sizeof (struct event_wmkeypress), &wmkeypress);

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

    unsigned int keyboard = channel_lookup(option_getstring("keyboard-service"));
    unsigned int mouse = channel_lookup(option_getstring("mouse-service"));
    unsigned int video = channel_lookup(option_getstring("video-service"));

    call_announce(0, djb_hash(2, "wm"));
    channel_send(0, keyboard, EVENT_LINK);
    channel_send(0, mouse, EVENT_LINK);
    channel_send(0, video, EVENT_LINK);
    setupvideo(video);

    while (channel_process(0))
    {

        if (state.state == STATE_UNGRABBED)
        {

            setupvideo(video);

            state.state = STATE_NORMAL;

        }

        if (state.state == STATE_NORMAL)
        {

            if (display.framebuffer)
            {

                render_place(state.rootwidget, &display.region);
                render_update(&display);
                render_undamage();

            }

        }

    }

    channel_send(0, video, EVENT_UNLINK);
    channel_send(0, mouse, EVENT_UNLINK);
    channel_send(0, keyboard, EVENT_UNLINK);

}

static void onmousemove(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousemove *mousemove = mdata;
    int x = util_clamp(state.mouseposition.x + mousemove->relx, 0, display.region.size.w);
    int y = util_clamp(state.mouseposition.y + mousemove->rely, 0, display.region.size.h);

    state.mousemovement.x = x - state.mouseposition.x;
    state.mousemovement.y = y - state.mouseposition.y;
    state.mouseposition.x = x;
    state.mouseposition.y = y;

    sethover(getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y));
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
                scalewidget(state.focusedwindow, util_max((int)(state.focusedwindow->placement.size.w) + state.mousemovement.x, CONFIG_WINDOW_MIN_WIDTH), util_max((int)(state.focusedwindow->placement.size.h) + state.mousemovement.y, CONFIG_WINDOW_MIN_HEIGHT));

        }

    }

    render_setmouse(state.mousewidget->placement.position.x, state.mousewidget->placement.position.y);

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

    struct widget *scrollablewidget = getscrollablewidgetat(state.mouseposition.x, state.mouseposition.y);

    if (scrollablewidget)
    {

        struct event_mousescroll *mousescroll = mdata;

        scrollwidget(scrollablewidget, 0, mousescroll->relz * 16);

    }

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

static void onvideoinfo(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videoinfo *videoinfo = mdata;
    unsigned int factor = videoinfo->height / 320;
    unsigned int lineheight = 12 + factor * 4;
    unsigned int padding = 4 + factor * 2;

    blit_initdisplay(&display, (void *)(unsigned long)videoinfo->framebuffer, videoinfo->width, videoinfo->height, videoinfo->bpp, linebuffer);
    pool_loadfont(0, fontn[factor]);
    pool_setfont(0, lineheight, padding);
    pool_loadfont(1, fontb[factor]);
    pool_setfont(1, lineheight, padding);

    state.mouseposition.x = videoinfo->width / 4;
    state.mouseposition.y = videoinfo->height / 4;
    state.mousewidget->position = util_position(state.mouseposition.x, state.mouseposition.y);
    state.mousewidget->size = util_size(12 + factor * 4, 16 + factor * 4);
    state.mousewidget->placement = util_region(state.mousewidget->position.x, state.mousewidget->position.y, state.mousewidget->size.w, state.mousewidget->size.h);

    render_damage(0, 0, videoinfo->width, videoinfo->height);
    render_place(state.rootwidget, &display.region);
    render_update(&display);
    render_undamage();

}

static void onwmgrab(unsigned int source, void *mdata, unsigned int msize)
{

    state.state = STATE_GRABBED;

    channel_bind(EVENT_KEYPRESS, 0);
    channel_bind(EVENT_KEYRELEASE, 0);
    channel_bind(EVENT_MOUSEMOVE, 0);
    channel_bind(EVENT_MOUSEPRESS, 0);
    channel_bind(EVENT_MOUSESCROLL, 0);
    channel_bind(EVENT_MOUSERELEASE, 0);
    channel_bind(EVENT_VIDEOINFO, 0);
    channel_send(0, source, EVENT_WMACK);

}

static void onwmmap(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(0, source, EVENT_WMINIT);

}

static void onwmrenderdata(unsigned int source, void *mdata, unsigned int msize)
{

    parser_parse(source, "root", msize, mdata);
    pool_loadresources();
    placewindows(source);
    purge(source);
    bump(state.mousewidget);

}

static void onwmungrab(unsigned int source, void *mdata, unsigned int msize)
{

    state.state = STATE_UNGRABBED;

    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_MOUSEMOVE, onmousemove);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_MOUSESCROLL, onmousescroll);
    channel_bind(EVENT_MOUSERELEASE, onmouserelease);
    channel_bind(EVENT_VIDEOINFO, onvideoinfo);
    channel_send(0, source, EVENT_WMACK);

}

static void onwmunmap(unsigned int source, void *mdata, unsigned int msize)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, source)))
    {

        struct widget *widget = current->data;

        widget_setstate(widget, WIDGET_STATE_DESTROYED);

    }

    purge(source);

}

static void setupwidgets(void)
{

    char *data0 =
        "+ layout id \"root\" flow \"stretch\"\n";
    char *data1 =
        "+ fill color \"FF202020\"\n"
        "+ image id \"mouse\" mimetype \"image/fudge-icon-mouse\" display \"fixed\"\n"
        "+ layout id \"desktop\" flow \"vertical\"\n"
        "  + layout id \"menu\" in \"desktop\" flow \"horizontal\"\n"
        "    + select id \"fudge-select\" in \"menu\" label \"Fudge\"\n"
        "      + layout id \"fudge-layout\" in \"fudge-select\" flow \"vertical-stretch\"\n"
        "        + choice in \"fudge-layout\" label \"About\" onclick \"run=initrd:bin/wabout\"\n"
        "        + choice in \"fudge-layout\" label \"Settings\" onclick \"run=initrd:bin/wsettings\"\n"
        "        + choice in \"fudge-layout\" label \"Reboot\" onclick \"run=initrd:bin/reboot\"\n"
        "    + select id \"apps-select\" in \"menu\" label \"Apps\"\n"
        "      + layout id \"apps-layout\" in \"apps-select\" flow \"vertical-stretch\"\n"
        "        + choice in \"apps-layout\" label \"Shell\" onclick \"run=initrd:bin/wshell\"\n"
        "        + choice in \"apps-layout\" label \"File Manager\" onclick \"run=initrd:bin/wfile\"\n"
        "        + choice in \"apps-layout\" label \"Calculator\" onclick \"run=initrd:bin/wcalc\"\n"
        "        + choice in \"apps-layout\" label \"Test\" onclick \"run=initrd:bin/wtest\"\n";

    parser_parse(0, "", cstring_length(data0), data0);
    parser_parse(0, "root", cstring_length(data1), data1);

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
    option_add("keyboard-service", "keyboard");
    option_add("mouse-service", "mouse");
    option_add("video-service", "video");
    option_add("wshell", "initrd:bin/wshell");
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
    channel_bind(EVENT_WMUNGRAB, onwmungrab);
    channel_bind(EVENT_WMUNMAP, onwmunmap);
    channel_open();

    while (channel_process(0));

    channel_close();

}

