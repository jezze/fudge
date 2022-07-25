#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "place.h"
#include "render.h"

struct configuration
{

    unsigned int displaywidth;
    unsigned int displayheight;
    unsigned int displaybpp;

};

struct state
{

    struct position mouseposition;
    struct position mousemovement;
    unsigned int mousebuttonleft;
    unsigned int mousebuttonright;

};

static struct widget *rootwidget;
static struct widget *mousewidget;
static struct render_display display;
static struct configuration configuration;
static struct state state;
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

    ctrl_setvideosettings(&settings, configuration.displaywidth, configuration.displayheight, configuration.displaybpp);
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

static struct widget *getfocusedwindow(void)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {
 
        struct widget *widget = current->data;

        if (widget->type == WIDGET_TYPE_WINDOW)
        {

            struct widget_window *window = widget->data;

            if (window->focus)
                return widget;

        }

    }

    return 0;

}

static struct widget *getwidgetat(struct widget *parent, int x, int y, unsigned int type)
{

    struct list_item *current = 0;
    struct widget *last = 0;

    while ((current = pool_nextin(current, parent->id)))
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

        if (x > parent->position.x && x <= parent->position.x + parent->size.w && y > parent->position.y && y <= parent->position.y + parent->size.h)
            return parent;

    }

    return 0;

}

static void damage(struct widget *widget)
{

    struct list_item *current = 0;

    render_damagebywidget(&display, widget);

    while ((current = pool_nextin(current, widget->id)))
        damage(current->data);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_link(FILE_G0);
    file_link(FILE_G1);
    file_link(FILE_G2);
    file_link(FILE_G4);
    setupvideo();

    while (channel_process())
    {

        place_widget(rootwidget, 0, 0, display.size.w, display.size.h);
        render_paint(&display, rootwidget, mousewidget->data);

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

    state.mousemovement.x = x - state.mouseposition.x;
    state.mousemovement.y = y - state.mouseposition.y;
    state.mouseposition.x = x;
    state.mouseposition.y = y;

    damage(mousewidget);

    mousewidget->position.x = state.mouseposition.x;
    mousewidget->position.y = state.mouseposition.y;

    damage(mousewidget);

    if (state.mousebuttonleft || state.mousebuttonright)
    {

        struct widget *widget = getfocusedwindow();

        if (widget)
        {

            damage(widget);

            if (state.mousebuttonleft)
            {

                widget->position.x += state.mousemovement.x;
                widget->position.y += state.mousemovement.y;

            }

            if (state.mousebuttonright)
            {

                widget->size.w = util_max((int)(widget->size.w) + state.mousemovement.x, WINDOW_MIN_WIDTH);
                widget->size.h = util_max((int)(widget->size.h) + state.mousemovement.y, WINDOW_MIN_HEIGHT);

            }

            damage(widget);

        }

    }

}

static void onmousepress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousepress *mousepress = mdata;
    struct widget *clickedwidget = getwidgetat(rootwidget, state.mouseposition.x, state.mouseposition.y, 0);
    struct widget *clickedwindow = getwidgetat(rootwidget, state.mouseposition.x, state.mouseposition.y, WIDGET_TYPE_WINDOW);
    struct widget *focusedwindow;

    switch (mousepress->button)
    {

    case 1:
        state.mousebuttonleft = 1;
        focusedwindow = getfocusedwindow();

        if (focusedwindow)
        {

            struct widget_window *window = focusedwindow->data;

            window->focus = 0;

            damage(focusedwindow);
            damage(mousewidget);

        }

        if (clickedwindow)
        {

            struct widget_window *window = clickedwindow->data;

            window->focus = 1;

            pool_bump(clickedwindow);
            pool_bump(mousewidget);
            damage(clickedwindow);
            damage(mousewidget);

        }

        if (clickedwidget)
        {

            /* Send event */

        }

        break;

    case 2:
        state.mousebuttonright = 1;

        break;

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

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + cstring_lengthz(key);

    if (cstring_match(key, "width"))
        configuration.displaywidth = cstring_rvalue(value, cstring_length(value), 10);

    if (cstring_match(key, "height"))
        configuration.displayheight = cstring_rvalue(value, cstring_length(value), 10);

    if (cstring_match(key, "bpp"))
        configuration.displaybpp = cstring_rvalue(value, cstring_length(value), 10);

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
    unsigned int lineheight = 12 + factor * 4;
    unsigned int padding = 4 + factor * 2;

    render_setup(&display, videomode->framebuffer, videomode->w, videomode->h, videomode->bpp);

    state.mouseposition.x = videomode->w / 4;
    state.mouseposition.y = videomode->h / 4;

    loadfont(factor);
    render_setfont(0, fontdata, lineheight, padding);

    switch (factor)
    {

    case 0:
    case 1:
        widget_initimage(mousewidget->data, mousedata16, mousecmap);

        mousewidget->position.x = state.mouseposition.x;
        mousewidget->position.y = state.mouseposition.y;
        mousewidget->size.w = 12;
        mousewidget->size.h = 16;

        break;

    case 2:
    default:
        widget_initimage(mousewidget->data, mousedata24, mousecmap);

        mousewidget->position.x = state.mouseposition.x;
        mousewidget->position.y = state.mouseposition.y;
        mousewidget->size.w = 18;
        mousewidget->size.h = 24;

        break;

    }

}

void widgets_setup(void)
{

    struct widget *widget;

    widget = pool_create(WIDGET_TYPE_CONTAINER, "root", "");

    widget_initcontainer(widget->data, CONTAINER_LAYOUT_FLOAT);

    widget = pool_create(WIDGET_TYPE_FILL, "background", "root");

    widget_initfill(widget->data, 0xFF142434);

    widget = pool_create(WIDGET_TYPE_WINDOW, "window0", "root");

    widget_initwindow(widget->data, "Window 0", 0);

    widget = pool_create(WIDGET_TYPE_CONTAINER, "base0", "window0");

    widget_initcontainer(widget->data, CONTAINER_LAYOUT_VERTICAL);

    widget = pool_create(WIDGET_TYPE_BUTTON, "button0", "base0");

    widget_initbutton(widget->data, "Click Me");

    widget = pool_create(WIDGET_TYPE_BUTTON, "button1", "base0");

    widget_initbutton(widget->data, "Click Me Too");

    widget = pool_create(WIDGET_TYPE_WINDOW, "window1", "root");

    widget_initwindow(widget->data, "Window 1", 1);

    widget = pool_create(WIDGET_TYPE_CONTAINER, "base1", "window1");

    widget_initcontainer(widget->data, CONTAINER_LAYOUT_VERTICAL);

    widget = pool_create(WIDGET_TYPE_TEXTBOX, "textbox0", "base1");

    widget_inittextbox(widget->data, 30, "Hello World!\nHow are we today?");

    widget = pool_create(WIDGET_TYPE_TEXTBOX, "textbox1", "base1");

    widget_inittextbox(widget->data, 25, "I am very good thank you!");

    widget = pool_create(WIDGET_TYPE_IMAGE, "mouse", "root");

    widget_initimage(widget->data, 0, 0);

    widget = pool_getwidgetbyid("window0");
    widget->position.x = 200;
    widget->position.y = 100;
    widget->size.w = 800;
    widget->size.h = 600;
    widget = pool_getwidgetbyid("window1");
    widget->position.x = 100;
    widget->position.y = 80;
    widget->size.w = 800;
    widget->size.h = 600;
    widget = pool_getwidgetbyid("button0");
    widget->position.x = 100;
    widget->position.y = 800;
    widget->size.w = 240;
    widget->size.h = 60;

    rootwidget = pool_getwidgetbyid("root");
    mousewidget = pool_getwidgetbyid("mouse");

}

void init(void)
{

    pool_setup();
    widgets_setup();

    configuration.displaywidth = 1920;
    configuration.displayheight = 1080;
    configuration.displaybpp = 4;

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_MOUSEMOVE, onmousemove);
    channel_bind(EVENT_MOUSEPRESS, onmousepress);
    channel_bind(EVENT_MOUSERELEASE, onmouserelease);
    channel_bind(EVENT_OPTION, onoption);
    channel_bind(EVENT_VIDEOMODE, onvideomode);

}

