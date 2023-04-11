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
static unsigned int numwindows;
static unsigned int paused;
static unsigned int linebuffer[3840];

static void setupvideo(void)
{

    struct ctrl_videosettings settings;
    unsigned char black[768];

    settings.width = option_getdecimal("width");
    settings.height = option_getdecimal("height");
    settings.bpp = option_getdecimal("bpp");

    buffer_clear(black, 768);

    if (!file_walk2(FILE_L0, option_getstring("video")))
        channel_warning("Could not open video");

    if (!file_walk(FILE_L1, FILE_L0, "colormap"))
        return;

    if (!file_walk(FILE_L2, FILE_L0, "ctrl"))
        return;

    file_writeall(FILE_L1, black, 768);
    file_writeall(FILE_L2, &settings, sizeof (struct ctrl_videosettings));

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

static void scrollwidget(struct widget *widget, int hamount, int vamount)
{

    if (widget->type == WIDGET_TYPE_LISTBOX)
    {

        struct widget_listbox *listbox = widget->data;

        if (listbox->overflow == ATTR_OVERFLOW_SCROLL || listbox->overflow == ATTR_OVERFLOW_HSCROLL)
            listbox->hscroll += hamount;

        if (listbox->overflow == ATTR_OVERFLOW_SCROLL || listbox->overflow == ATTR_OVERFLOW_VSCROLL)
            listbox->vscroll += vamount;

        damage(widget);

    }

    else if (widget->type == WIDGET_TYPE_TEXTBOX)
    {

        struct widget_textbox *textbox = widget->data;

        if (textbox->overflow == ATTR_OVERFLOW_SCROLL || textbox->overflow == ATTR_OVERFLOW_HSCROLL)
            textbox->hscroll += hamount;

        if (textbox->overflow == ATTR_OVERFLOW_SCROLL || textbox->overflow == ATTR_OVERFLOW_VSCROLL)
            textbox->vscroll += vamount;

        damage(widget);

    }

}

static void bumpmouse(void)
{

    if (state.mousewidget)
    {

        pool_bump(state.mousewidget);
        damage(state.mousewidget);

    }

}

static void bump(struct widget *widget)
{

    pool_bump(widget);
    damageall(widget);
    bumpmouse();

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

                widget->position.x = 128 + 128 * numwindows;
                widget->position.y = 128 + 64 * numwindows;
                widget->size.w = 640;
                widget->size.h = 480;

                bump(widget);

                numwindows++;

            }

            damageall(widget);

        }

    }

}

static void destroy(struct widget *widget)
{

    if (state.hoverwidget == widget)
        state.hoverwidget = 0;

    if (state.focusedwidget == widget)
        state.focusedwidget = 0;

    if (state.focusedwindow == widget)
        state.focusedwindow = 0;

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

static void setfocuswidget(struct widget *widget)
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

        if (widget->type == WIDGET_TYPE_SELECT)
        {

            struct list_item *current = 0;

            while ((current = pool_nextin(current, widget)))
            {

                struct widget *child = current->data;

                bump(child);

            }

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

    if (widget_setstate(widget, WIDGET_STATE_HOVER))
    {

        state.hoverwidget = widget;

        damageall(state.hoverwidget);

    }

}

static void clickwidget(struct widget *widget)
{


    switch (widget->type)
    {

    case WIDGET_TYPE_WINDOW:
        if (util_intersects(state.mousewidget->position.x, widget->position.x + widget->size.w - CONFIG_WINDOW_BUTTON_WIDTH, widget->position.x + widget->size.w) && util_intersects(state.mousewidget->position.y, widget->position.y, widget->position.y + CONFIG_WINDOW_BUTTON_HEIGHT))
            channel_send(widget->source, EVENT_TERM);

        break;

    case WIDGET_TYPE_BUTTON:
        if (state.mousebuttonleft)
        {

            struct widget_button *button = widget->data;

            if (button->onclick)
            {

                struct {struct event_wmevent wmevent; char data[128];} message;

                message.wmevent.type = 1; /* 1 is click event */
                message.wmevent.length = buffer_write(message.data, 128, strpool_getstring(button->onclick), strpool_getcstringlength(button->onclick) + 1, 0);

                channel_sendbuffer(widget->source, EVENT_WMEVENT, sizeof (struct event_wmevent) + message.wmevent.length, &message);

            }

        }

        break;

    case WIDGET_TYPE_CHOICE:
        if (state.mousebuttonleft)
        {

            struct widget_choice *choice = widget->data;

            if (choice->onclick)
            {

                struct {struct event_wmevent wmevent; char data[128];} message;

                message.wmevent.type = 1; /* 1 is click event */
                message.wmevent.length = buffer_write(message.data, 128, strpool_getstring(choice->onclick), strpool_getcstringlength(choice->onclick) + 1, 0);

                channel_sendbuffer(widget->source, EVENT_WMEVENT, sizeof (struct event_wmevent) + message.wmevent.length, &message);

            }

        }

        break;

    case WIDGET_TYPE_SELECT:
        if (state.mousebuttonleft)
        {

            struct widget_select *select = widget->data;

            if (select->onclick)
            {

                struct {struct event_wmevent wmevent; char data[128];} message;

                message.wmevent.type = 1; /* 1 is click event */
                message.wmevent.length = buffer_write(message.data, 128, strpool_getstring(select->onclick), strpool_getcstringlength(select->onclick) + 1, 0);

                channel_sendbuffer(widget->source, EVENT_WMEVENT, sizeof (struct event_wmevent) + message.wmevent.length, &message);

            }

        }

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        if (state.mousebuttonleft)
        {

            struct widget_textbutton *textbutton = widget->data;

            if (textbutton->onclick)
            {

                struct {struct event_wmevent wmevent; char data[128];} message;

                message.wmevent.type = 1; /* 1 is click event */
                message.wmevent.length = buffer_write(message.data, 128, strpool_getstring(textbutton->onclick), strpool_getcstringlength(textbutton->onclick) + 1, 0);

                channel_sendbuffer(widget->source, EVENT_WMEVENT, sizeof (struct event_wmevent) + message.wmevent.length, &message);

            }

        }

        break;

    }

}

static void keypresswidget(struct widget *widget, unsigned char scancode, unsigned int unicode, unsigned int length, unsigned int keymod)
{

    if (widget_isinteractive(widget))
    {

        struct event_wmkeypress2 wmkeypress;

        wmkeypress.scancode = scancode;
        wmkeypress.unicode = unicode;
        wmkeypress.length = length;
        wmkeypress.keymod = keymod;

        cstring_writezero(wmkeypress.pressed, 16, cstring_write(wmkeypress.pressed, 16, strpool_getstring(widget->id), 0));
        channel_sendbuffer(widget->source, EVENT_WMKEYPRESS, sizeof (struct event_wmkeypress2), &wmkeypress);

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

                unsigned int id = file_spawn(FILE_L0, "/bin/wshell");

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

        if (state.focusedwidget)
            keypresswidget(state.focusedwidget, keypress->scancode, keycode->value[0], keycode->length, state.keymod);

    }

}

static void onkeyrelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    state.keymod = keymap_modkey(keyrelease->scancode, state.keymod);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_G0, option_getstring("wm")))
        channel_warning("Could not open window manager service");

    if (!file_walk2(FILE_L0, option_getstring("keyboard")))
        channel_warning("Could not open keyboard");

    if (!file_walk(FILE_G1, FILE_L0, "event"))
        channel_warning("Could not open keyboard event");

    if (!file_walk2(FILE_L0, option_getstring("mouse")))
        channel_warning("Could not open mouse");

    if (!file_walk(FILE_G2, FILE_L0, "event"))
        channel_warning("Could not open mouse event");

    if (!file_walk2(FILE_L0, option_getstring("video")))
        channel_warning("Could not open video");

    if (!file_walk(FILE_G3, FILE_L0, "event"))
        channel_warning("Could not open video event");

    file_link(FILE_G0);
    file_link(FILE_G1);
    file_link(FILE_G2);
    file_link(FILE_G3);
    setupvideo();

    while (channel_process())
    {

        if (!paused)
        {

            place_widget(state.rootwidget, 0, 0, 0, 0, 0, display.size.w, display.size.h);

            if (display.framebuffer)
            {

                render(&display, state.mousewidget->position.x, state.mousewidget->position.y);
                render_undamage();

            }

        }

    }

    file_unlink(FILE_G3);
    file_unlink(FILE_G2);
    file_unlink(FILE_G1);
    file_unlink(FILE_G0);

}

static void onmousemove(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousemove *mousemove = mdata;
    int x = util_clamp(state.mouseposition.x + mousemove->relx, 0, display.size.w);
    int y = util_clamp(state.mouseposition.y + mousemove->rely, 0, display.size.h);
    struct widget *hoverwidget;

    state.mousemovement.x = x - state.mouseposition.x;
    state.mousemovement.y = y - state.mouseposition.y;
    state.mouseposition.x = x;
    state.mouseposition.y = y;

    hoverwidget = getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y);

    if (hoverwidget)
        sethover(hoverwidget);

    if (state.mousewidget)
    {

        damage(state.mousewidget);

        state.mousewidget->position.x = state.mouseposition.x;
        state.mousewidget->position.y = state.mouseposition.y;

        damage(state.mousewidget);

    }

    if (state.mousebuttonleft && state.clickedwidget)
    {

        if (widget_isdragable(state.clickedwidget))
        {

            damageall(state.clickedwidget);

            state.clickedwidget->position.x += state.mousemovement.x;
            state.clickedwidget->position.y += state.mousemovement.y;

            damageall(state.clickedwidget);

        }


    }

    if (state.mousebuttonright && state.focusedwindow)
    {

        damageall(state.focusedwindow);

        state.focusedwindow->size.w = util_max((int)(state.focusedwindow->size.w) + state.mousemovement.x, CONFIG_WINDOW_MIN_WIDTH);
        state.focusedwindow->size.h = util_max((int)(state.focusedwindow->size.h) + state.mousemovement.y, CONFIG_WINDOW_MIN_HEIGHT);

        damageall(state.focusedwindow);

    }

}

static void onmousepress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousepress *mousepress = mdata;
    struct widget *clickedwindow = getwidgetoftypeat(state.mouseposition.x, state.mouseposition.y, WIDGET_TYPE_WINDOW);

    state.clickedwidget = getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y);
    state.mouseclicked.x = state.mouseposition.x;
    state.mouseclicked.y = state.mouseposition.y;

    switch (mousepress->button)
    {

    case 1:
        state.mousebuttonleft = 1;

        if (clickedwindow)
            setfocuswindow(clickedwindow);

        setfocuswidget(state.clickedwidget);

        if (state.clickedwidget)
            clickwidget(state.clickedwidget);

        break;

    case 2:
        state.mousebuttonright = 1;

        if (state.clickedwidget)
            clickwidget(state.clickedwidget);

        break;

    }

}

static void onmousescroll(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousescroll *mousescroll = mdata;
    struct widget *scrollablewidget = getscrollablewidgetat(state.mouseposition.x, state.mouseposition.y);
    struct widget *hoverwidget;

    if (scrollablewidget)
        scrollwidget(scrollablewidget, 0, mousescroll->relz * 16);

    hoverwidget = getinteractivewidgetat(state.mouseposition.x, state.mouseposition.y);

    if (hoverwidget)
        sethover(hoverwidget);

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

    paused = 1;

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
    bumpmouse();

}

static void onwmungrab(unsigned int source, void *mdata, unsigned int msize)
{

    paused = 0;

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

        widget->state = WIDGET_STATE_DESTROYED;

    }

    removedestroyed(source);

}

static void setupwidgets(void)
{

    char *data =
        "+ layout id \"root\"\n"
        "  + fill in \"root\" color \"FF202020\"\n"
        "  + layout id \"desktop\" in \"root\" form \"vertical\"\n"
        "    + layout id \"menu\" in \"desktop\" form \"horizontal\"\n"
        "      + select id \"fudge\" in \"menu\" label \"FUDGE\"\n"
        "      + layout id \"items\" in \"fudge\" form \"vertical\"\n"
        "        + choice id \"terminal\" in \"items\" label \"Terminal\"\n"
        "        + choice id \"filemanager\" in \"items\" label \"File Manager\"\n"
        "        + choice id \"calculator\" in \"items\" label \"Calculator\"\n"
        "  + image id \"mouse\" in \"root\" mimetype \"image/fudge-icon-mouse\"\n";

    parser_parse(0, "", cstring_length(data), data);

    state.rootwidget = pool_getwidgetbyid(0, "root");
    state.mousewidget = pool_getwidgetbyid(0, "mouse");

}

void init(void)
{

    pool_setup();
    setupwidgets();
    option_add("width", "1920");
    option_add("height", "1080");
    option_add("bpp", "4");
    option_add("wm", "system:service/wm");
    option_add("keyboard", "system:keyboard");
    option_add("mouse", "system:mouse");
    option_add("video", "system:video/if:0");
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

