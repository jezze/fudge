#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "cache.h"
#include "attr.h"
#include "widget.h"
#include "strpool.h"
#include "pool.h"
#include "blit.h"
#include "place.h"
#include "render.h"
#include "parser.h"

struct state
{

    unsigned int paused;
    struct util_position mouseposition;
    struct util_position mousemovement;
    struct util_position mouseclicked;
    unsigned int mousebuttonleft;
    unsigned int mousebuttonright;
    struct widget *rootwidget;
    struct widget *mousewidget;
    struct widget *hoverwidget;
    struct widget *focusedwindow;
    struct widget *focusedwidget;
    struct widget *clickedwidget;
    unsigned int keymod;

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

static struct widget *getwidgetoftypeat(int x, int y, unsigned int type)
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

    int x0 = util_clamp(widget->position.x, 0, display.size.w);
    int y0 = util_clamp(widget->position.y, 0, display.size.h);
    int x2 = util_clamp(widget->position.x + widget->size.w, 0, display.size.w);
    int y2 = util_clamp(widget->position.y + widget->size.h, 0, display.size.h);

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

static void placewindows(unsigned int source)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, source)))
    {

        struct widget *widget = current->data;

        if (widget->type == WIDGET_TYPE_WINDOW)
        {

            if (widget->size.w == 0 && widget->size.h == 0)
            {

                unsigned int w8 = display.size.w / 8;
                unsigned int h8 = display.size.h / 8;

                widget->position.x = w8;
                widget->position.y = h8;
                widget->size.w = w8 * 3;
                widget->size.h = h8 * 6;

                bump(widget);

            }

            damageall(widget);

        }

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

static void sendevent(unsigned int source, unsigned int type, unsigned int action)
{

    if (source)
    {

        struct {struct event_wmevent wmevent; char data[128];} message;

        message.wmevent.type = type;
        message.wmevent.length = buffer_write(message.data, 128, strpool_getstring(action), strpool_getcstringlength(action) + 1, 0);

        channel_send_buffer(source, EVENT_WMEVENT, sizeof (struct event_wmevent) + message.wmevent.length, &message);

    }

    else
    {

        char *cmd = strpool_getstring(action);

        if (cstring_match_word(cmd, 0, "run"))
        {

            unsigned int id = call_spawn_absolute(FILE_L0, FILE_PW, cstring_get_word(cmd, 1));

            if (id)
                channel_send(id, EVENT_MAIN);

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

            if (util_intersects(state.mousewidget->position.x, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->position.x + widget->size.w) && util_intersects(state.mousewidget->position.y, widget->position.y, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT))
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

static void setclick(struct widget *widget)
{

    if (state.clickedwidget)
    {

        state.clickedwidget = 0;

    }

    if (widget)
    {

        state.clickedwidget = widget;

        clickwidget(widget);

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

    if (state.clickedwidget == widget)
        setclick(0);

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
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keypress->scancode, state.keymod);

    state.keymod = keymap_modkey(keypress->scancode, state.keymod);

    if ((state.keymod & KEYMOD_ALT))
    {

        switch (keypress->scancode)
        {

        case 0x10:
            if ((state.keymod & KEYMOD_SHIFT))
            {

                if (state.focusedwindow)
                    channel_send(state.focusedwindow->source, EVENT_TERM);

            }

            break;

        case 0x19:
            if ((state.keymod & KEYMOD_SHIFT))
            {

                unsigned int id = call_spawn_absolute(FILE_L0, FILE_PW, option_getstring("wshell"));

                if (id)
                    channel_send(id, EVENT_MAIN);

            }

            break;

        case 0x49:
            if (widget_isscrollable(state.focusedwidget))
                scrollwidget(state.focusedwidget, 0, -16);

            break;

        case 0x51:
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

            wmkeypress.scancode = keypress->scancode;
            wmkeypress.unicode = keycode->value[0];
            wmkeypress.length = keycode->length;
            wmkeypress.keymod = state.keymod;

            channel_send_buffer(state.focusedwindow->source, EVENT_WMKEYPRESS, sizeof (struct event_wmkeypress), &wmkeypress);

        }

    }

}

static void onkeyrelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    state.keymod = keymap_modkey(keyrelease->scancode, state.keymod);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!call_walk_absolute(FILE_G0, option_getstring("wm-service")))
        PANIC();

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

    call_link(FILE_G0, 8000);
    call_link(FILE_G1, 8001);
    call_link(FILE_G2, 8002);
    call_link(FILE_G3, 8003);
    setupvideo();

    while (channel_process())
    {

        if (!state.paused)
        {

            place_widget(state.rootwidget, 0, 0, 0, 0, 0, display.size.w, display.size.h, 0, 0, display.size.w, display.size.h);

            if (display.framebuffer)
            {

                render(&display, state.mousewidget->position.x, state.mousewidget->position.y);
                render_undamage();

            }

        }

    }

    call_unlink(FILE_G3);
    call_unlink(FILE_G2);
    call_unlink(FILE_G1);
    call_unlink(FILE_G0);

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

    if (state.mousebuttonleft && widget_isdragable(state.clickedwidget))
        translatewidget(state.clickedwidget, state.mousemovement.x, state.mousemovement.y);

    if (state.mousebuttonright && widget_isresizable(state.focusedwindow))
        scalewidget(state.focusedwindow, util_max((int)(state.focusedwindow->size.w) + state.mousemovement.x, CONFIG_WINDOW_MIN_WIDTH), util_max((int)(state.focusedwindow->size.h) + state.mousemovement.y, CONFIG_WINDOW_MIN_HEIGHT));

}

static void onmousepress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousepress *mousepress = mdata;
    struct widget *clickedwindow = getwidgetoftypeat(state.mouseposition.x, state.mouseposition.y, WIDGET_TYPE_WINDOW);
    struct widget *clickedwidget = getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y);

    state.mouseclicked.x = state.mouseposition.x;
    state.mouseclicked.y = state.mouseposition.y;

    switch (mousepress->button)
    {

    case 1:
        state.mousebuttonleft = 1;

        setfocuswindow(clickedwindow);
        setfocus(clickedwidget);
        setclick(clickedwidget);

        break;

    case 2:
        state.mousebuttonright = 1;

        setclick(clickedwidget);

        break;

    }

}

static void onmousescroll(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousescroll *mousescroll = mdata;

    scrollwidget(getscrollablewidgetat(state.mouseposition.x, state.mouseposition.y), 0, mousescroll->relz * 16);
    sethover(getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y));

}

static void onmouserelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mouserelease *mouserelease = mdata;

    switch (mouserelease->button)
    {

    case 1:
        state.mousebuttonleft = 0;

        break;

    case 2:
        state.mousebuttonright = 0;

        break;

    }

    state.clickedwidget = 0;

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
        state.mousewidget->position.x = state.mouseposition.x;
        state.mousewidget->position.y = state.mouseposition.y;
        state.mousewidget->size.w = 12;
        state.mousewidget->size.h = 16;

        break;

    case 2:
    default:
        state.mousewidget->position.x = state.mouseposition.x;
        state.mousewidget->position.y = state.mouseposition.y;
        state.mousewidget->size.w = 18;
        state.mousewidget->size.h = 24;

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

}

static void onwmmap(unsigned int source, void *mdata, unsigned int msize)
{

    channel_listen(source, EVENT_WMMAP);
    channel_listen(source, EVENT_WMUNMAP);
    channel_listen(source, EVENT_WMGRAB);
    channel_listen(source, EVENT_WMUNGRAB);
    channel_listen(source, EVENT_WMRENDERDATA);
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
        "        + choice in \"fudge-layout\" label \"About\" onclick \"run /bin/wabout\"\n"
        "        + choice in \"fudge-layout\" label \"Settings\" onclick \"run /bin/wsettings\"\n"
        "        + choice in \"fudge-layout\" label \"Reboot\" onclick \"run /bin/reboot\"\n"
        "    + select id \"apps-select\" in \"menu\" label \"Apps\"\n"
        "      + layout id \"apps-layout\" in \"apps-select\" flow \"vertical-stretch\"\n"
        "        + choice in \"apps-layout\" label \"Shell\" onclick \"run /bin/wshell\"\n"
        "        + choice in \"apps-layout\" label \"File Manager\" onclick \"run /bin/wfile\"\n"
        "        + choice in \"apps-layout\" label \"Calculator\" onclick \"run /bin/wcalc\"\n"
        "        + choice in \"apps-layout\" label \"Test\" onclick \"run /bin/wtest\"\n";

    parser_parse(0, "", cstring_length(data0), data0);
    parser_parse(0, "root", cstring_length(data1), data1);
    parser_parse(0, "root", cstring_length(data2), data2);

    state.rootwidget = pool_getwidgetbyid(0, "root");
    state.mousewidget = pool_getwidgetbyid(0, "mouse");

    bump(state.mousewidget);

}

void init(void)
{

    pool_setup();
    setupwidgets();
    option_add("width", "1920");
    option_add("height", "1080");
    option_add("bpp", "4");
    option_add("wm-service", "system:service/wm");
    option_add("keyboard", "system:keyboard");
    option_add("mouse", "system:mouse");
    option_add("video", "system:video/if:0");
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

