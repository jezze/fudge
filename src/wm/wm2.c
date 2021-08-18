#include <fudge.h>
#include <abi.h>

struct point
{

    unsigned int x;
    unsigned int y;

};

struct rectangle
{

    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

};

struct image
{

    struct rectangle size;
    void *data;

};

struct mouse
{

    struct point position;
    struct image image;

};

struct configuration
{

    unsigned int padding;
    unsigned int lineheight;
    unsigned int steplength;
    unsigned int framebuffer;
    unsigned int w;
    unsigned int h;
    unsigned int bpp;

};

struct window
{

    char *title;
    unsigned int active;
    struct rectangle size;
    struct point position;

};

static unsigned int optwidth = 1920;
static unsigned int optheight = 1080;
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
    mouse.image.size.x = 0;
    mouse.image.size.y = 0;

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

static void paintrectangle(struct rectangle *r, unsigned int color)
{

    unsigned int *tdata = (unsigned int *)configuration.framebuffer;
    unsigned int x;
    unsigned int y;

    for (y = r->y; y < r->y + r->h; y++)
    {

        for (x = r->x; x < r->x + r->w; x++)
        {

            unsigned int toffset = y * screen.w + x;

            tdata[toffset] = color;

        }

    }

}

static void paintwindow(struct window *window)
{

    struct rectangle r1;

    r1.x = 100;
    r1.y = 80;
    r1.w = 400;
    r1.h = 600;

    paintrectangle(&r1, 0xFF88A878);

}

static void blit_cmap32(unsigned char *s, unsigned int *cmap, unsigned int sw, unsigned int sh, unsigned char *t, unsigned int tw, unsigned int th, unsigned int px, unsigned int py, struct rectangle *clip)
{

    unsigned int *tdata = (unsigned int *)configuration.framebuffer;
    unsigned int x;
    unsigned int y;

    for (y = clip->y; y < clip->h; y++)
    {

        for (x = clip->x; x < clip->w; x++)
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
        clip->w = w0 - (xw0 - xw1);

    if (yh0 > yh1)
        clip->h = h0 - (yh0 - yh1);

}

static void paintmouse(void)
{

    struct rectangle clip;

    clip.x = mouse.image.size.x;
    clip.y = mouse.image.size.y;
    clip.w = mouse.image.size.w;
    clip.h = mouse.image.size.h;

    calculateclip(&clip, mouse.position.x, mouse.position.y, mouse.image.size.w, mouse.image.size.h, 0, 0, configuration.w, configuration.h);
    blit_cmap32(mouse.image.data, mousecmap, mouse.image.size.w, mouse.image.size.h, (unsigned char *)configuration.framebuffer, configuration.w, configuration.h, mouse.position.x, mouse.position.y, &clip);

}

static void paint(void)
{

    struct window window1;

    window1.title = "Example1";
    window1.active = 1;
    window1.size.x = 0;
    window1.size.y = 0;
    window1.size.w = 400;
    window1.size.h = 300;
    window1.position.x = 200;
    window1.position.y = 140;

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

        if (configuration.framebuffer)
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

    if (mouse.position.x < screen.x || mouse.position.x >= screen.x + screen.w)
        mouse.position.x = (mousemove->relx < 0) ? screen.x : screen.x + screen.w - 1;

    if (mouse.position.y < screen.y || mouse.position.y >= screen.y + screen.h)
        mouse.position.y = (mousemove->rely < 0) ? screen.y : screen.y + screen.h - 1;

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

    configuration.framebuffer = videomode->framebuffer;
    configuration.w = videomode->w;
    configuration.h = videomode->h;
    configuration.bpp = videomode->bpp;
    configuration.lineheight = 12 + factor * 4;
    configuration.padding = 4 + factor * 2;
    configuration.steplength = videomode->w / 12;

    loadfont(factor);

    screen.x = 0;
    screen.y = 0;
    screen.w = videomode->w;
    screen.h = videomode->h;

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

