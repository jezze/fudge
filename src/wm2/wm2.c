#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"
#include "render.h"

struct configuration
{

    unsigned int displaywidth;
    unsigned int displayheight;
    unsigned int displaybpp;
    unsigned int padding;
    unsigned int lineheight;
    unsigned int steplength;

};

struct state
{

    struct position mouseposition;
    struct position mousemovement;
    unsigned int mousedrag;
    unsigned int mouseresize;

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

static int capvalue(int x, int min, int max)
{

    if (x < min)
        x = min;

    if (x > max)
        x = max;

    return x;

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

static void place_window(struct widget *widget, int x, int y, unsigned int w, unsigned int h);
static void place_widget(struct widget *widget, int x, int y, unsigned int w, unsigned int h);

static void place_button(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = 200;
    widget->size.h = 80;

}

static void place_image(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

}

static void place_layout(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct widget_layout *layout = widget->data;
    struct list_item *current = 0;

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = w;
    widget->size.h = h;

    switch (layout->type)
    {

    case LAYOUT_TYPE_VERTICAL:
        break;

    case LAYOUT_TYPE_FLOAT:
        while ((current = pool_nextin(current, widget->id)))
        {

            struct widget *child = current->data;

            place_widget(child, widget->position.x, widget->position.y, widget->size.w, widget->size.h);

        }

        break;

    }

}

static void place_window(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, widget->position.x + 16, widget->position.y + 16 + 38, widget->size.w - 24, widget->size.h - 24);

    }

}

static void place_widget(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        place_button(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_IMAGE:
        place_image(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_LAYOUT:
        place_layout(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_WINDOW:
        place_window(widget, x, y, w, h);

        break;

    }

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
    int x = capvalue(state.mouseposition.x + mousemove->relx, 0, display.size.w);
    int y = capvalue(state.mouseposition.y + mousemove->rely, 0, display.size.h);

    state.mousemovement.x = x - state.mouseposition.x;
    state.mousemovement.y = y - state.mouseposition.y;
    state.mouseposition.x = x;
    state.mouseposition.y = y;

    render_damage(&display, mousewidget->position.x, mousewidget->position.y, mousewidget->position.x + mousewidget->size.w, mousewidget->position.y + mousewidget->size.h);

    mousewidget->position.x = state.mouseposition.x;
    mousewidget->position.y = state.mouseposition.y;

    render_damage(&display, mousewidget->position.x, mousewidget->position.y, mousewidget->position.x + mousewidget->size.w, mousewidget->position.y + mousewidget->size.h);

    if (state.mousedrag || state.mouseresize)
    {

        struct widget *widget = getfocusedwindow();

        if (widget)
        {

            render_damage(&display, widget->position.x, widget->position.y, widget->position.x + widget->size.w, widget->position.y + widget->size.h);

            if (state.mousedrag)
            {

                widget->position.x += state.mousemovement.x;
                widget->position.y += state.mousemovement.y;

            }

            if (state.mouseresize)
            {

                int w = (int)(widget->size.w) + state.mousemovement.x;
                int h = (int)(widget->size.h) + state.mousemovement.y;

                if (w < WINDOW_MIN_HEIGHT)
                    w = WINDOW_MIN_WIDTH;

                if (h < WINDOW_MIN_HEIGHT)
                    h = WINDOW_MIN_HEIGHT;

                widget->size.w = w;
                widget->size.h = h;

            }

            render_damage(&display, widget->position.x, widget->position.y, widget->position.x + widget->size.w, widget->position.y + widget->size.h);

        }

    }

}

static void onmousepress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousepress *mousepress = mdata;

    switch (mousepress->button)
    {

    case 1:
        state.mousedrag = 1;

        break;

    case 2:
        state.mouseresize = 1;

        break;

    }

}

static void onmouserelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mouserelease *mouserelease = mdata;

    switch (mouserelease->button)
    {

    case 1:
        state.mousedrag = 0;

        break;

    case 2:
        state.mouseresize = 0;

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

    render_setup(&display, videomode->framebuffer, videomode->w, videomode->h, videomode->bpp);

    configuration.lineheight = 12 + factor * 4;
    configuration.padding = 4 + factor * 2;
    configuration.steplength = videomode->w / 12;
    state.mouseposition.x = videomode->w / 4;
    state.mouseposition.y = videomode->h / 4;

    loadfont(factor);

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

void init(void)
{

    pool_setup();

    rootwidget = pool_getwidgetbyid("root");
    mousewidget = pool_getwidgetbyid("mouse");

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

