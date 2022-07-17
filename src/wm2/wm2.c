#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"
#include "render.h"

struct configuration
{

    unsigned int padding;
    unsigned int lineheight;
    unsigned int steplength;

};

static unsigned int optwidth = 1920;
static unsigned int optheight = 1080;
static unsigned int optbpp = 4;
/*
static unsigned int keymod = KEYMOD_NONE;
*/
static struct render_display display;
static struct mouse mouse;
static struct configuration configuration;
static unsigned char fontdata[0x8000];
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
unsigned char mousedata16[] = {
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

static void setupvideo(void)
{

    struct ctrl_videosettings settings;
    unsigned char black[768];

    ctrl_setvideosettings(&settings, optwidth, optheight, optbpp);
    buffer_clear(black, 768);

    if (!file_walk(FILE_L0, FILE_G3, "ctrl"))
        return;

    if (!file_walk(FILE_L1, FILE_G3, "colormap"))
        return;

    file_seekwriteall(FILE_L1, black, 768, 0);
    file_seekwriteall(FILE_L0, &settings, sizeof (struct ctrl_videosettings), 0);

}

static void setmouse(unsigned int x, unsigned int y, unsigned int factor)
{

    mouse.position.x = x;
    mouse.position.y = y;

    switch (factor)
    {

    case 0:
    case 1:
        mouse.image.size.w = 12;
        mouse.image.size.h = 16;
        mouse.image.data = mousedata16;

        break;

    case 2:
    default:
        mouse.image.size.w = 18;
        mouse.image.size.h = 24;
        mouse.image.data = mousedata24;

        break;

    }

}

static void loadfont(unsigned int factor)
{

    switch (factor)
    {

    case 0:
        file_walk2(FILE_L0, "/data/ter-112n.pcf");

        break;

    case 1:
        file_walk2(FILE_L0, "/data/ter-114n.pcf");

        break;

    case 2:
        file_walk2(FILE_L0, "/data/ter-116n.pcf");

        break;

    default:
        file_walk2(FILE_L0, "/data/ter-118n.pcf");

        break;

    }

    file_seekread(FILE_L0, fontdata, 0x8000, 0);

}

static int capvalue(int x, int min, int max)
{

    if (x < min)
        x = min;

    if (x > max)
        x = max;

    return x;

}

static struct window *getfocusedwindow(void)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {
 
        struct widget *widget = current->data;

        if (widget->type == WIDGET_TYPE_WINDOW)
        {

            struct window *window = widget->data;

            if (window->focus)
                return window;

        }

    }

    return 0;

}

static void place_layout(struct widget *widget, struct layout *layout, int x, int y, unsigned int w, unsigned int h)
{

    layout->position.x = x;
    layout->position.y = y;
    layout->size.w = w;
    layout->size.h = h;

    if (layout->type == LAYOUT_TYPE_FLOAT)
    {

        struct list_item *current = 0;

        while ((current = pool_nextin(current, widget->id)))
        {

        }

    }

}

static void place_window(struct widget *widget, struct window *window, int x, int y, unsigned int w, unsigned int h)
{

    struct list_item *current = 0;

    window->position.x = x;
    window->position.y = y;
    window->size.w = w;
    window->size.h = h;

    while ((current = pool_nextin(current, widget->id)))
    {

    }

}

static void place_widget(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_LAYOUT:
        place_layout(widget, widget->data, x, y, w, h);

        break;

    case WIDGET_TYPE_WINDOW:
        place_window(widget, widget->data, x, y, w, h);

        break;

    }

}

static void place(unsigned int w, unsigned int h)
{

    struct widget *widget = pool_getwidgetbyid("root");

    if (widget)
        place_widget(widget, 0, 0, w, h);

}

static void onkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    /*
    struct event_keypress *keypress = mdata;
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keypress->scancode, keymod);

    keymod = keymap_modkey(keypress->scancode, keymod);

    switch (keypress->scancode)
    {

    }
    */

}

static void onkeyrelease(unsigned int source, void *mdata, unsigned int msize)
{

    /*
    struct event_keyrelease *keyrelease = mdata;
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keyrelease->scancode, keymod);

    keymod = keymap_modkey(keyrelease->scancode, keymod);
    */

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_link(FILE_G0);
    file_link(FILE_G1);
    file_link(FILE_G2);
    file_link(FILE_G4);
    setupvideo();

    while (channel_process())
        render_paint(&display, &mouse);

    file_unlink(FILE_G4);
    file_unlink(FILE_G2);
    file_unlink(FILE_G1);
    file_unlink(FILE_G0);

}

static void onmousemove(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousemove *mousemove = mdata;

    render_damage(&display, mouse.position.x, mouse.position.y, mouse.position.x + mouse.image.size.w, mouse.position.y + mouse.image.size.h);

    mouse.position.x = capvalue(mouse.position.x + mousemove->relx, 0, display.size.w);
    mouse.position.y = capvalue(mouse.position.y + mousemove->rely, 0, display.size.h);

    render_damage(&display, mouse.position.x, mouse.position.y, mouse.position.x + mouse.image.size.w, mouse.position.y + mouse.image.size.h);

    if (mouse.drag)
    {

        struct window *window = getfocusedwindow();

        if (window)
        {

            render_damage(&display, window->position.x, window->position.y, window->position.x + window->size.w, window->position.y + window->size.h);

            window->position.x += mousemove->relx;
            window->position.y += mousemove->rely;

            render_damage(&display, window->position.x, window->position.y, window->position.x + window->size.w, window->position.y + window->size.h);

        }

    }

    if (mouse.resize)
    {

        struct window *window = getfocusedwindow();

        if (window)
        {

            int w = (int)(window->size.w) + mousemove->relx;
            int h = (int)(window->size.h) + mousemove->rely;

            if (w < WINDOW_MIN_HEIGHT)
                w = WINDOW_MIN_WIDTH;

            if (h < WINDOW_MIN_HEIGHT)
                h = WINDOW_MIN_HEIGHT;

            render_damage(&display, window->position.x, window->position.y, window->position.x + window->size.w, window->position.y + window->size.h);

            window->size.w = w;
            window->size.h = h;

            render_damage(&display, window->position.x, window->position.y, window->position.x + window->size.w, window->position.y + window->size.h);

        }

    }

}

static void onmousepress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousepress *mousepress = mdata;

    switch (mousepress->button)
    {

    case 1:
        mouse.drag = 1;

        break;

    case 2:
        mouse.resize = 1;

        break;

    }

}

static void onmouserelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mouserelease *mouserelease = mdata;

    switch (mouserelease->button)
    {

    case 1:
        mouse.drag = 0;

        break;

    case 2:
        mouse.resize = 0;

        break;

    }

}

static void onmousescroll(unsigned int source, void *mdata, unsigned int msize)
{

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + cstring_lengthz(key);

    if (cstring_match(key, "width"))
        optwidth = cstring_rvalue(value, cstring_length(value), 10);

    if (cstring_match(key, "height"))
        optheight = cstring_rvalue(value, cstring_length(value), 10);

    if (cstring_match(key, "bpp"))
        optbpp = cstring_rvalue(value, cstring_length(value), 10);

    if (cstring_match(key, "keyboard"))
        file_walk2(FILE_G1, value);

    if (cstring_match(key, "mouse"))
        file_walk2(FILE_G2, value);

    if (cstring_match(key, "video"))
    {

        if (file_walk2(FILE_G3, value))
        {

            file_walk(FILE_G4, FILE_G3, "event");
            file_walk(FILE_G5, FILE_G3, "data");

        }

    }

}

static void onvideomode(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;
    unsigned int factor = videomode->h / 320;

    render_setup(&display, videomode->framebuffer, videomode->w, videomode->h, videomode->bpp);

    configuration.lineheight = 12 + factor * 4;
    configuration.padding = 4 + factor * 2;
    configuration.steplength = videomode->w / 12;

    loadfont(factor);
    setmouse(videomode->w / 4, videomode->h / 4, factor);
    place(display.size.w, display.size.h);

}

static void onwmmap(unsigned int source, void *mdata, unsigned int msize)
{

}

static void onwmrenderdata(unsigned int source, void *mdata, unsigned int msize)
{

}

static void onwmunmap(unsigned int source, void *mdata, unsigned int msize)
{

}

void init(void)
{

    pool_setup();

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_MOUSEMOVE, onmousemove);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_MOUSERELEASE, onmouserelease);
    channel_bind(EVENT_MOUSESCROLL, onmousescroll);
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_VIDEOMODE, onvideomode);
    channel_bind(EVENT_WMMAP, onwmmap);
    channel_bind(EVENT_WMRENDERDATA, onwmrenderdata);
    channel_bind(EVENT_WMUNMAP, onwmunmap);

}

