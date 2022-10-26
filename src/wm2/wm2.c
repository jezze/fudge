#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "blit.h"
#include "place.h"
#include "render.h"
#include "parser.h"

#define WINDOW_MIN_WIDTH                128
#define WINDOW_MIN_HEIGHT               128

struct state
{

    struct widget_position mouseposition;
    struct widget_position mousemovement;
    unsigned int mousebuttonleft;
    unsigned int mousebuttonright;
    struct widget *rootwidget;
    struct widget *mousewidget;
    struct widget *hoverwidget;
    struct widget *focusedwindow;
    struct widget *focusedwidget;
    unsigned int keymod;

};

static struct blit_display display;
static struct blit_damage displaydamage;
static struct state state;
static unsigned int numwindows;
static unsigned char fontnormal[0x8000];
static unsigned char fontbold[0x8000];
static unsigned char mousedata24[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static unsigned char mousedata16[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0xFF,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00,
    0x00, 0x02, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,
    0x00, 0x02, 0x01, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static unsigned int mousecmap[] = {
    0xFF000000,
    0xFFB05070,
    0xFFF898B8
};

static void setupvideo(void)
{

    struct ctrl_videosettings settings;
    unsigned char black[768];

    settings.width = option_getdecimal("width");
    settings.height = option_getdecimal("height");
    settings.bpp = option_getdecimal("bpp");

    buffer_clear(black, 768);

    if (!file_walk(FILE_L0, FILE_G3, "ctrl"))
        return;

    if (!file_walk(FILE_L1, FILE_G3, "colormap"))
        return;

    file_seekwriteall(FILE_L1, black, 768, 0);
    file_seekwriteall(FILE_L0, &settings, sizeof (struct ctrl_videosettings), 0);

}

static void loadfont(unsigned int factor)
{

    switch (factor)
    {

    case 0:
        file_walk2(FILE_L0, "/data/font/ter-112n.pcf");
        file_walk2(FILE_L1, "/data/font/ter-112b.pcf");

        break;

    case 1:
        file_walk2(FILE_L0, "/data/font/ter-114n.pcf");
        file_walk2(FILE_L1, "/data/font/ter-114b.pcf");

        break;

    case 2:
        file_walk2(FILE_L0, "/data/font/ter-116n.pcf");
        file_walk2(FILE_L1, "/data/font/ter-116b.pcf");

        break;

    default:
        file_walk2(FILE_L0, "/data/font/ter-118n.pcf");
        file_walk2(FILE_L1, "/data/font/ter-118b.pcf");

        break;

    }

    file_read(FILE_L0, fontnormal, 0x8000);
    file_read(FILE_L1, fontbold, 0x8000);

}

static struct widget *gethoverwidgetat(struct widget *parent, int x, int y)
{

    struct list_item *current = 0;
    struct widget *last = 0;

    while ((current = pool_nextin(current, parent)))
    {
 
        struct widget *child = current->data;
        struct widget *match = gethoverwidgetat(child, x, y);

        if (match)
            last = match;

    }

    if (last)
        return last;

    switch (parent->type)
    {

    case WIDGET_TYPE_BUTTON:
    case WIDGET_TYPE_SELECT:
    case WIDGET_TYPE_TEXTBOX:
    case WIDGET_TYPE_WINDOW:
        if (util_intersects(x, parent->position.x, parent->position.x + parent->size.w) && util_intersects(y, parent->position.y, parent->position.y + parent->size.h))
            return parent;

        break;

    }

    return 0;

}

static struct widget *getwidgetat(struct widget *parent, int x, int y, unsigned int type)
{

    struct list_item *current = 0;
    struct widget *last = 0;

    while ((current = pool_nextin(current, parent)))
    {
 
        struct widget *child = current->data;
        struct widget *match = getwidgetat(child, x, y, type);

        if (match)
            last = match;

    }

    if (last)
        return last;

    if (!type || type == parent->type)
    {

        if (util_intersects(x, parent->position.x, parent->position.x + parent->size.w) && util_intersects(y, parent->position.y, parent->position.y + parent->size.h))
            return parent;

    }

    return 0;

}

static void damage(struct widget *widget)
{

    struct list_item *current = 0;
    int x0 = util_clamp(widget->position.x, 0, display.size.w);
    int y0 = util_clamp(widget->position.y, 0, display.size.h);
    int x1 = util_clamp(widget->position.x + widget->size.w, 0, display.size.w);
    int y1 = util_clamp(widget->position.y + widget->size.h, 0, display.size.h);

    render_damage(&displaydamage, x0, y0, x1, y1);

    while ((current = pool_nextin(current, widget)))
        damage(current->data);

}

static void bump(struct widget *widget)
{

    pool_bump(widget);
    pool_bump(state.mousewidget);
    damage(widget);
    damage(state.mousewidget);

}

static void setfocus(struct widget *widget)
{

    if (state.focusedwidget)
    {

        widget_setstate(state.focusedwidget, WIDGET_STATE_FOCUSOFF);
        widget_setstate(state.focusedwidget, WIDGET_STATE_NORMAL);
        damage(state.focusedwidget);

        state.focusedwidget = 0;

    }

    if (widget_setstate(widget, WIDGET_STATE_FOCUS))
    {

        state.focusedwidget = widget;

        damage(state.focusedwidget);

    }

}

static void setfocuswindow(struct widget *widget)
{

    if (state.focusedwindow)
    {

        widget_setstate(state.focusedwindow, WIDGET_STATE_FOCUSOFF);
        widget_setstate(state.focusedwindow, WIDGET_STATE_NORMAL);
        damage(state.focusedwindow);

        state.focusedwindow = 0;

    }

    if (widget_setstate(widget, WIDGET_STATE_FOCUS))
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
        damage(state.hoverwidget);

        state.hoverwidget = 0;

    }

    if (widget_setstate(widget, WIDGET_STATE_HOVER))
    {

        state.hoverwidget = widget;

        damage(state.hoverwidget);

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
                    channel_sendto(state.focusedwindow->source, EVENT_TERM);

            }

            break;

        case 0x19:
            if ((state.keymod & KEYMOD_SHIFT))
            {

                unsigned int id = file_spawn("/bin/wshell2");

                if (id)
                    channel_sendto(id, EVENT_MAIN);

            }

            break;

        }

    }

    else
    {

        if (state.focusedwidget)
        {

            struct event_wmkeypress2 wmkeypress;

            wmkeypress.scancode = keypress->scancode;
            wmkeypress.unicode = keycode->value[0];
            wmkeypress.length = keycode->length;
            wmkeypress.keymod = state.keymod;

            cstring_writezero(wmkeypress.pressed, 16, cstring_write(wmkeypress.pressed, 16, pool_getstring(state.focusedwidget->id), 0));
            channel_sendbufferto(state.focusedwidget->source, EVENT_WMKEYPRESS, sizeof (struct event_wmkeypress2), &wmkeypress);

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

    if (!file_walk2(FILE_G0, option_getstring("wm")))
        channel_warning("Could not open window manager service");

    if (!file_walk2(FILE_G1, option_getstring("keyboard")))
        channel_warning("Could not open keyboard");

    if (!file_walk2(FILE_G2, option_getstring("mouse")))
        channel_warning("Could not open mouse");

    if (!file_walk2(FILE_G3, option_getstring("video")))
        channel_warning("Could not open video");

    if (!file_walk(FILE_G4, FILE_G3, "event"))
        channel_warning("Could not open video event");

    file_link(FILE_G0);
    file_link(FILE_G1);
    file_link(FILE_G2);
    file_link(FILE_G4);
    setupvideo();

    while (channel_process())
    {

        place_widget(state.rootwidget, 0, 0, 0, 0, display.size.w, display.size.h);
        render(&display, &displaydamage);

    }

    file_unlink(FILE_G4);
    file_unlink(FILE_G2);
    file_unlink(FILE_G1);
    file_unlink(FILE_G0);

}

static void onmousemove(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousemove *mousemove = mdata;
    int x = util_clamp(state.mouseposition.x + mousemove->relx, 0, display.size.w);
    int y = util_clamp(state.mouseposition.y + mousemove->rely, 0, display.size.h);
    struct widget *hoverwidget = gethoverwidgetat(state.rootwidget, state.mouseposition.x, state.mouseposition.y);

    if (hoverwidget)
        sethover(hoverwidget);

    state.mousemovement.x = x - state.mouseposition.x;
    state.mousemovement.y = y - state.mouseposition.y;
    state.mouseposition.x = x;
    state.mouseposition.y = y;

    if (state.mousewidget)
    {

        damage(state.mousewidget);

        state.mousewidget->position.x = state.mouseposition.x;
        state.mousewidget->position.y = state.mouseposition.y;

        damage(state.mousewidget);

    }

    if (state.focusedwindow)
    {

        if (state.mousebuttonleft && (state.keymod & KEYMOD_ALT))
        {

            damage(state.focusedwindow);

            state.focusedwindow->position.x += state.mousemovement.x;
            state.focusedwindow->position.y += state.mousemovement.y;

            damage(state.focusedwindow);

        }

        if (state.mousebuttonright && (state.keymod & KEYMOD_ALT))
        {

            damage(state.focusedwindow);

            state.focusedwindow->size.w = util_max((int)(state.focusedwindow->size.w) + state.mousemovement.x, WINDOW_MIN_WIDTH);
            state.focusedwindow->size.h = util_max((int)(state.focusedwindow->size.h) + state.mousemovement.y, WINDOW_MIN_HEIGHT);

            damage(state.focusedwindow);

        }

    }

}

static void onmousepress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousepress *mousepress = mdata;
    struct widget *clickedwindow = getwidgetat(state.rootwidget, state.mouseposition.x, state.mouseposition.y, WIDGET_TYPE_WINDOW);
    struct widget *clickedwidget = gethoverwidgetat(state.rootwidget, state.mouseposition.x, state.mouseposition.y);

    switch (mousepress->button)
    {

    case 1:
        state.mousebuttonleft = 1;

        if (clickedwindow)
        {

            setfocuswindow(clickedwindow);
            damage(state.mousewidget);

        }

        if (clickedwidget)
        {

            struct event_wmclick wmclick;

            setfocus(clickedwidget);
            cstring_writezero(wmclick.clicked, 16, cstring_write(wmclick.clicked, 16, pool_getstring(clickedwidget->id), 0));
            channel_sendbufferto(clickedwidget->source, EVENT_WMCLICK, sizeof (struct event_wmclick), &wmclick);

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

    if (state.focusedwidget && state.focusedwidget->type == WIDGET_TYPE_TEXTBOX)
    {

        struct widget_textbox *textbox = state.focusedwidget->data;

        textbox->scroll += mousescroll->relz * 8;

        damage(state.focusedwidget);

    }

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

}

static void onvideomode(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;
    unsigned int factor = videomode->h / 320;
    unsigned int lineheight = 12 + factor * 4;
    unsigned int padding = 4 + factor * 2;
    struct widget_image *mouseimage = state.mousewidget->data;

    blit_initdisplay(&display, videomode->framebuffer, videomode->w, videomode->h, videomode->bpp);
    render_damage(&displaydamage, 0, 0, videomode->w, videomode->h);

    state.mouseposition.x = videomode->w / 4;
    state.mouseposition.y = videomode->h / 4;

    loadfont(factor);
    pool_setfont(POOL_FONTNORMAL, fontnormal, lineheight, padding);
    pool_setfont(POOL_FONTBOLD, fontbold, lineheight, padding);

    switch (factor)
    {

    case 0:
    case 1:
        state.mousewidget->position.x = state.mouseposition.x;
        state.mousewidget->position.y = state.mouseposition.y;
        state.mousewidget->size.w = 12;
        state.mousewidget->size.h = 16;
        mouseimage->data = mousedata16;
        mouseimage->cmap = mousecmap;

        break;

    case 2:
    default:
        state.mousewidget->position.x = state.mouseposition.x;
        state.mousewidget->position.y = state.mouseposition.y;
        state.mousewidget->size.w = 18;
        state.mousewidget->size.h = 24;
        mouseimage->data = mousedata24;
        mouseimage->cmap = mousecmap;

        break;

    }

}

static void onwmmap(unsigned int source, void *mdata, unsigned int msize)
{

    channel_sendto(source, EVENT_WMINIT);

}

static void onwmrenderdata(unsigned int source, void *mdata, unsigned int msize)
{

    struct list_item *current = 0;

    parser_parse(source, "root", msize, mdata);

    while ((current = pool_nextsource(current, source)))
    {

        struct widget *widget = current->data;

        if (widget->type == WIDGET_TYPE_WINDOW)
        {

            if (widget->size.w == 0 && widget->size.h == 0)
            {

                widget->position.x = 64 + 128 * numwindows;
                widget->position.y = 64 + 64 * numwindows;
                widget->size.w = 640;
                widget->size.h = 640;

                bump(widget);

                numwindows++;

            }

            damage(widget);

        }

    }

}

static void onwmunmap(unsigned int source, void *mdata, unsigned int msize)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {
 
        struct widget *child = current->data;

        if (child->source == source)
        {

            current = current->prev;

            damage(child);
            pool_destroy(child);

        }

    }

}

static void setupwidgets(void)
{

    char *data =
        "+ container id \"root\" layout \"float\"\n"
        "+ fill in \"root\" color \"FF202020\"\n"
        "+ image id \"mouse\" in \"root\" type \"internal\"\n";

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
    option_add("keyboard", "system:keyboard/event");
    option_add("mouse", "system:mouse/event");
    option_add("video", "system:video/if:0");
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_MOUSEMOVE, onmousemove);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_MOUSESCROLL, onmousescroll);
    channel_bind(EVENT_MOUSERELEASE, onmouserelease);
    channel_bind(EVENT_VIDEOMODE, onvideomode);
    channel_bind(EVENT_WMMAP, onwmmap);
    channel_bind(EVENT_WMRENDERDATA, onwmrenderdata);
    channel_bind(EVENT_WMUNMAP, onwmunmap);

}

