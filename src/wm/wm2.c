#include <fudge.h>
#include <abi.h>

struct position
{

    unsigned int x;
    unsigned int y;

};

struct size
{

    unsigned int w;
    unsigned int h;

};

struct rectangle
{

    struct position position;
    struct size size;

};

struct image
{

    struct size size;
    void *data;

};

struct mouse
{

    struct position position;
    struct image image;

};

struct display
{

    void *framebuffer;
    struct size size;
    unsigned int bpp;

};

struct configuration
{

    unsigned int padding;
    unsigned int lineheight;
    unsigned int steplength;
    struct display display;

};

struct window
{

    char *title;
    unsigned int active;
    struct position position;
    struct size size;

};

static unsigned int optwidth = 1024;
static unsigned int optheight = 768;
static unsigned int optbpp = 4;
/*
static unsigned int keymod = KEYMOD_NONE;
*/
static struct rectangle screen;
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
static unsigned int mousecmap[] = {
    0xFF000000,
    0xFFB05070,
    0xFFF898B8
};
static unsigned char colormap8[] = {
    0x00, 0x00, 0x00,
    0x3F, 0x3F, 0x3F,
    0x04, 0x02, 0x02,
    0x06, 0x04, 0x04,
    0x08, 0x06, 0x06,
    0x08, 0x10, 0x18,
    0x0C, 0x14, 0x1C,
    0x28, 0x10, 0x18,
    0x38, 0x20, 0x28,
    0x1C, 0x18, 0x18,
    0x3F, 0x3F, 0x3F
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
    file_seekwriteall(FILE_L1, colormap8, 3 * 11, 0);

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

static void paintline(struct rectangle *r, unsigned int color, unsigned int y)
{

    unsigned int *tdata = configuration.display.framebuffer;
    unsigned int x;

    for (x = r->position.x; x < r->position.x + r->size.w; x++)
    {

        unsigned int toffset = y * screen.size.w + x;

        tdata[toffset] = color;

    }

}

static void paintrectangle(struct rectangle *r, unsigned int color)
{

    unsigned int *tdata = configuration.display.framebuffer;
    unsigned int x;
    unsigned int y;

    for (y = r->position.y; y < r->position.y + r->size.h; y++)
    {

        for (x = r->position.x; x < r->position.x + r->size.w; x++)
        {

            unsigned int toffset = y * screen.size.w + x;

            tdata[toffset] = color;

        }

    }

}

struct colorpos
{

    int x;
    int w;
    unsigned int color;

};

static void convert(struct rectangle *r1, int x, int w, unsigned int y, struct rectangle *r)
{

    if (x >= 0)
        r->position.x = r1->position.x + x;
    else
        r->position.x = r1->position.x + (int)r1->size.w + x;

    if (w > 0)
        r->size.w = w;
    else
        r->size.w = (int)r1->size.w + (w * 2);

}

static void paintcolorpos(struct rectangle *r1, unsigned int *cmap, struct colorpos *pos, unsigned int n, unsigned int y)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct colorpos *p = &pos[i];
        struct rectangle r;

        convert(r1, p->x, p->w, y, &r);
        paintline(&r, cmap[p->color], y);

    }

}

static unsigned int bordercmap[] = {
    0xFF101010,
    0xFFA8C898,
    0xFF88A878
};

#define BORDER_TYPE_0 0
#define BORDER_TYPE_1 1
#define BORDER_TYPE_2 2
#define BORDER_TYPE_3 3
#define BORDER_TYPE_TITLE 4
#define BORDER_TYPE_SPACING 5
#define BORDER_TYPE_AREA 6
#define BORDER_COLOR_SHADOW 0
#define BORDER_COLOR_MAIN_LIGHT 1
#define BORDER_COLOR_MAIN_NORMAL 2

static struct colorpos cp0[1] = {
    {1, -1, BORDER_COLOR_SHADOW}
};

static struct colorpos cp1[1] = {
    {0, 0, BORDER_COLOR_SHADOW}
};

static struct colorpos cp2[3] = {
    {0, 3, BORDER_COLOR_SHADOW},
    {3, -3, BORDER_COLOR_MAIN_LIGHT},
    {-3, 3, BORDER_COLOR_SHADOW}
};

static struct colorpos cp3[5] = {
    {0, 2, BORDER_COLOR_SHADOW},
    {2, 2, BORDER_COLOR_MAIN_LIGHT},
    {4, -4, BORDER_COLOR_MAIN_NORMAL},
    {-4, 2, BORDER_COLOR_MAIN_LIGHT},
    {-2, 2, BORDER_COLOR_SHADOW}
};

static struct colorpos cptitle[5] = {
    {0, 2, BORDER_COLOR_SHADOW},
    {2, 1, BORDER_COLOR_MAIN_LIGHT},
    {3, -3, BORDER_COLOR_MAIN_NORMAL},
    {-3, 1, BORDER_COLOR_MAIN_LIGHT},
    {-2, 2, BORDER_COLOR_SHADOW}
};

static struct colorpos cpspacing[7] = {
    {0, 2, BORDER_COLOR_SHADOW},
    {2, 1, BORDER_COLOR_MAIN_LIGHT},
    {3, 1, BORDER_COLOR_MAIN_NORMAL},
    {4, -4, BORDER_COLOR_SHADOW},
    {-4, 1, BORDER_COLOR_MAIN_NORMAL},
    {-3, 1, BORDER_COLOR_MAIN_LIGHT},
    {-2, 2, BORDER_COLOR_SHADOW}
};

static struct colorpos cparea[7] = {
    {0, 2, BORDER_COLOR_SHADOW},
    {2, 1, BORDER_COLOR_MAIN_LIGHT},
    {3, 1, BORDER_COLOR_MAIN_NORMAL},
    {4, -4, BORDER_COLOR_SHADOW},
    {-4, 1, BORDER_COLOR_MAIN_NORMAL},
    {-3, 1, BORDER_COLOR_MAIN_LIGHT},
    {-2, 2, BORDER_COLOR_SHADOW}
};

static void paintwindow(struct window *window)
{

    struct rectangle r;
    unsigned int y = window->position.y;
    unsigned int i;

    r.position.x = window->position.x;
    r.size.w = window->size.w;

    paintcolorpos(&r, bordercmap, cp0, 1, y++);
    paintcolorpos(&r, bordercmap, cp1, 1, y++);
    paintcolorpos(&r, bordercmap, cp2, 3, y++);
    paintcolorpos(&r, bordercmap, cp3, 5, y++);

    for (i = 0; i < 36; i++)
        paintcolorpos(&r, bordercmap, cptitle, 5, y++);

    paintcolorpos(&r, bordercmap, cpspacing, 7, y++);

    for (i = 0; i < window->size.h - 45; i++)
        paintcolorpos(&r, bordercmap, cparea, 7, y++);

    paintcolorpos(&r, bordercmap, cp3, 5, y++);
    paintcolorpos(&r, bordercmap, cp2, 3, y++);
    paintcolorpos(&r, bordercmap, cp1, 1, y++);
    paintcolorpos(&r, bordercmap, cp0, 1, y++);

}

static void blit_cmap32(unsigned char *s, unsigned int *cmap, unsigned int sw, unsigned int sh, unsigned char *t, unsigned int tw, unsigned int th, unsigned int px, unsigned int py, struct rectangle *clip)
{

    unsigned int *tdata = configuration.display.framebuffer;
    unsigned int x;
    unsigned int y;

    for (y = clip->position.y; y < clip->size.h; y++)
    {

        for (x = clip->position.x; x < clip->size.w; x++)
        {

            unsigned int soffset = (y * sw + x);
            unsigned int toffset = (py * tw + px) + (y * tw + x);

            if (s[soffset] != 0xFF)
                tdata[toffset] = cmap[s[soffset]];

        }

    }

}

static void calculateclip(struct rectangle *clip, unsigned int x0, unsigned int y0, unsigned int w0, unsigned int h0, unsigned int x1, unsigned int y1, unsigned int w1, unsigned int h1)
{

    unsigned int xw0 = x0 + w0;
    unsigned int yh0 = y0 + h0;
    unsigned int xw1 = x1 + w1;
    unsigned int yh1 = y1 + h1;

    if (xw0 > xw1)
        clip->size.w = w0 - (xw0 - xw1);

    if (yh0 > yh1)
        clip->size.h = h0 - (yh0 - yh1);

}

static void paintmouse(void)
{

    struct rectangle clip;

    clip.position.x = 0;
    clip.position.y = 0;
    clip.size.w = mouse.image.size.w;
    clip.size.h = mouse.image.size.h;

    calculateclip(&clip, mouse.position.x, mouse.position.y, mouse.image.size.w, mouse.image.size.h, 0, 0, configuration.display.size.w, configuration.display.size.h);
    blit_cmap32(mouse.image.data, mousecmap, mouse.image.size.w, mouse.image.size.h, configuration.display.framebuffer, configuration.display.size.w, configuration.display.size.h, mouse.position.x, mouse.position.y, &clip);

}

static void paint(void)
{

    struct window window1;

    window1.title = "Example1";
    window1.active = 1;
    window1.size.w = 400;
    window1.size.h = 600;
    window1.position.x = 100;
    window1.position.y = 80;

    paintrectangle(&screen, 0xFF202020);
    paintwindow(&window1);
    paintmouse();

}

static void onkeypress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
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

static void onkeyrelease(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    /*
    struct event_keyrelease *keyrelease = mdata;
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keyrelease->scancode, keymod);

    keymod = keymap_modkey(keyrelease->scancode, keymod);
    */

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_link(FILE_G0);
    file_link(FILE_G1);
    file_link(FILE_G2);
    file_link(FILE_G4);
    setupvideo();

    while (channel_process(channel))
    {

        if (configuration.display.framebuffer)
            paint();

    }

    file_unlink(FILE_G4);
    file_unlink(FILE_G2);
    file_unlink(FILE_G1);
    file_unlink(FILE_G0);

}

static void onmousemove(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousemove *mousemove = mdata;

    mouse.position.x += mousemove->relx;
    mouse.position.y += mousemove->rely;

    if (mouse.position.x < screen.position.x || mouse.position.x >= screen.position.x + screen.size.w)
        mouse.position.x = (mousemove->relx < 0) ? screen.position.x : screen.position.x + screen.size.w - 1;

    if (mouse.position.y < screen.position.y || mouse.position.y >= screen.position.y + screen.size.h)
        mouse.position.y = (mousemove->rely < 0) ? screen.position.y : screen.position.y + screen.size.h - 1;

}

static void onmousepress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousepress *mousepress = mdata;

    switch (mousepress->button)
    {

    }

}

static void onmouserelease(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

}

static void onmousescroll(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "width"))
        optwidth = ascii_rvalue(value, ascii_length(value), 10);

    if (ascii_match(key, "height"))
        optheight = ascii_rvalue(value, ascii_length(value), 10);

    if (ascii_match(key, "bpp"))
        optbpp = ascii_rvalue(value, ascii_length(value), 10);

    if (ascii_match(key, "keyboard"))
        file_walk2(FILE_G1, value);

    if (ascii_match(key, "mouse"))
        file_walk2(FILE_G2, value);

    if (ascii_match(key, "video"))
    {

        if (file_walk2(FILE_G3, value))
        {

            file_walk(FILE_G4, FILE_G3, "event");
            file_walk(FILE_G5, FILE_G3, "data");

        }

    }

}

static void onvideomode(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;
    unsigned int factor = videomode->h / 320;

    configuration.display.framebuffer = videomode->framebuffer;
    configuration.display.size.w = videomode->w;
    configuration.display.size.h = videomode->h;
    configuration.display.bpp = videomode->bpp;
    configuration.lineheight = 12 + factor * 4;
    configuration.padding = 4 + factor * 2;
    configuration.steplength = videomode->w / 12;

    loadfont(factor);

    screen.position.x = 0;
    screen.position.y = 0;
    screen.size.w = videomode->w;
    screen.size.h = videomode->h;

    setmouse(videomode->w / 4, videomode->h / 4, factor);

}

static void onwmmap(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

}

static void onwmrenderdata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

}

static void onwmunmap(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "system:service/wm"))
        return;

    channel_setcallback(channel, EVENT_KEYPRESS, onkeypress);
    channel_setcallback(channel, EVENT_KEYRELEASE, onkeyrelease);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_MOUSEMOVE, onmousemove);
    channel_setcallback(channel, EVENT_MOUSEPRESS, onmousepress);
    channel_setcallback(channel, EVENT_MOUSERELEASE, onmouserelease);
    channel_setcallback(channel, EVENT_MOUSESCROLL, onmousescroll);
    channel_setcallback(channel, EVENT_OPTION, onoption);
    channel_setcallback(channel, EVENT_VIDEOMODE, onvideomode);
    channel_setcallback(channel, EVENT_WMMAP, onwmmap);
    channel_setcallback(channel, EVENT_WMRENDERDATA, onwmrenderdata);
    channel_setcallback(channel, EVENT_WMUNMAP, onwmunmap);

}

