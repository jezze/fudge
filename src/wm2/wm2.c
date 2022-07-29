#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "place.h"
#include "render.h"

#define WINDOW_MIN_WIDTH                128
#define WINDOW_MIN_HEIGHT               128

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
        file_walk2(FILE_L1, "/data/ter-112b.pcf");

        break;

    case 1:
        file_walk2(FILE_L0, "/data/ter-114n.pcf");
        file_walk2(FILE_L1, "/data/ter-114b.pcf");

        break;

    case 2:
        file_walk2(FILE_L0, "/data/ter-116n.pcf");
        file_walk2(FILE_L1, "/data/ter-116b.pcf");

        break;

    default:
        file_walk2(FILE_L0, "/data/ter-118n.pcf");
        file_walk2(FILE_L1, "/data/ter-118b.pcf");

        break;

    }

    file_seekread(FILE_L0, fontnormal, 0x8000, 0);
    file_seekread(FILE_L1, fontbold, 0x8000, 0);

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

        if (x > parent->position.x && x <= parent->position.x + parent->size.w && y > parent->position.y && y <= parent->position.y + parent->size.h)
            return parent;

    }

    return 0;

}

static void damage(struct widget *widget)
{

    struct list_item *current = 0;

    render_damagebywidget(&display, widget);

    while ((current = pool_nextin(current, widget)))
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

        place_widget(rootwidget, 0, 0, 0, 0, display.size.w, display.size.h);
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
    render_setfont(RENDER_FONTNORMAL, fontnormal, lineheight, padding);
    render_setfont(RENDER_FONTBOLD, fontbold, lineheight, padding);

    switch (factor)
    {

    case 0:
    case 1:
        widget_setimage(mousewidget->data, mousedata16, mousecmap);

        mousewidget->position.x = state.mouseposition.x;
        mousewidget->position.y = state.mouseposition.y;
        mousewidget->size.w = 12;
        mousewidget->size.h = 16;

        break;

    case 2:
    default:
        widget_setimage(mousewidget->data, mousedata24, mousecmap);

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

    widget = pool_create(0, WIDGET_TYPE_CONTAINER, "root", "");

    widget_setattribute(widget, WIDGET_ATTR_LAYOUT, "float");
    widget_setattribute(widget, WIDGET_ATTR_PLACEMENT, "normal");

    widget = pool_create(0, WIDGET_TYPE_FILL, "background", "root");

    widget_setattribute(widget, WIDGET_ATTR_COLOR, "FF142434");

    widget = pool_create(1, WIDGET_TYPE_WINDOW, "window", "root");

    widget_setattribute(widget, WIDGET_ATTR_TITLE, "Window 0");

    widget = pool_create(1, WIDGET_TYPE_CONTAINER, "base", "window");

    widget_setattribute(widget, WIDGET_ATTR_LAYOUT, "vertical");
    widget_setattribute(widget, WIDGET_ATTR_PLACEMENT, "normal");
    widget_setattribute(widget, WIDGET_ATTR_PADDING, "16");

    widget = pool_create(1, WIDGET_TYPE_TEXT, "", "base");

    widget_setattribute(widget, WIDGET_ATTR_CONTENT, "Hello World!\nHow are we today?");

    widget = pool_create(1, WIDGET_TYPE_BUTTON, "", "base");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "Click Me");

    widget = pool_create(1, WIDGET_TYPE_TEXT, "", "base");

    widget_setattribute(widget, WIDGET_ATTR_CONTENT, "Hello again!");

    widget = pool_create(1, WIDGET_TYPE_BUTTON, "", "base");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "Click Me Too");

    widget = pool_create(2, WIDGET_TYPE_WINDOW, "window", "root");

    widget_setattribute(widget, WIDGET_ATTR_TITLE, "Shell");

    widget = pool_create(2, WIDGET_TYPE_CONTAINER, "base", "window");

    widget_setattribute(widget, WIDGET_ATTR_LAYOUT, "maximize");
    widget_setattribute(widget, WIDGET_ATTR_PLACEMENT, "stretched");
    widget_setattribute(widget, WIDGET_ATTR_PADDING, "16");

    widget = pool_create(2, WIDGET_TYPE_TEXTBOX, "prompt", "base");

    widget_setattribute(widget, WIDGET_ATTR_CONTENT, "$  ");

    widget = pool_create(3, WIDGET_TYPE_WINDOW, "window", "root");

    widget_setattribute(widget, WIDGET_ATTR_TITLE, "Calculator");

    widget = pool_create(3, WIDGET_TYPE_CONTAINER, "base", "window");

    widget_setattribute(widget, WIDGET_ATTR_LAYOUT, "vertical");
    widget_setattribute(widget, WIDGET_ATTR_PLACEMENT, "normal");

    widget = pool_create(3, WIDGET_TYPE_CONTAINER, "container", "base");

    widget_setattribute(widget, WIDGET_ATTR_LAYOUT, "vertical");
    widget_setattribute(widget, WIDGET_ATTR_PLACEMENT, "stretched");
    widget_setattribute(widget, WIDGET_ATTR_PADDING, "16");

    widget = pool_create(3, WIDGET_TYPE_TEXTBOX, "display", "container");

    widget_setattribute(widget, WIDGET_ATTR_CONTENT, "1337");
    widget_setattribute(widget, WIDGET_ATTR_MODE, "readonly");

    widget = pool_create(3, WIDGET_TYPE_GRID, "buttons", "base");

    widget_setattribute(widget, WIDGET_ATTR_COLUMNS, "4");
    widget_setattribute(widget, WIDGET_ATTR_PLACEMENT, "stretched");
    widget_setattribute(widget, WIDGET_ATTR_PADDING, "16");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-7", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "7");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-8", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "8");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-9", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "9");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-div", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "/");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-6", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "4");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-5", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "5");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-4", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "6");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-mul", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "x");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-3", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "1");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-2", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "2");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-1", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "3");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-sub", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "-");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-0", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "0");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-dot", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, ".");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-add", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "+");

    widget = pool_create(3, WIDGET_TYPE_BUTTON, "button-result", "buttons");

    widget_setattribute(widget, WIDGET_ATTR_LABEL, "=");

    widget = pool_create(0, WIDGET_TYPE_IMAGE, "mouse", "root");

    widget = pool_getwidgetbyid(1, "window");
    widget->position.x = 200;
    widget->position.y = 100;
    widget->size.w = 800;
    widget->size.h = 600;
    widget = pool_getwidgetbyid(2, "window");
    widget->position.x = 100;
    widget->position.y = 80;
    widget->size.w = 800;
    widget->size.h = 600;
    widget = pool_getwidgetbyid(3, "window");
    widget->position.x = 80;
    widget->position.y = 60;
    widget->size.w = 600;
    widget->size.h = 600;

    rootwidget = pool_getwidgetbyid(0, "root");
    mousewidget = pool_getwidgetbyid(0, "mouse");

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

