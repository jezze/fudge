#include <fudge.h>
#include <abi.h>

#define WINDOW_MIN_WIDTH                128
#define WINDOW_MIN_HEIGHT               128
#define WIDGET_TYPE_WINDOW              1
#define DAMAGE_STATE_NONE               0
#define DAMAGE_STATE_MADE               1

struct position
{

    int x;
    int y;

};

struct size
{

    unsigned int w;
    unsigned int h;

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
    unsigned int drag;
    unsigned int resize;

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

};

struct linesegment
{

    unsigned int type;
    int p0;
    int p1;
    unsigned int color;

};

struct widget
{

    unsigned int type;
    char *id;
    char *parent;
    void *data;

};

struct window
{

    char *title;
    unsigned int focus;
    struct position position;
    struct size size;

};

struct damage
{

    unsigned int state;
    struct position position0;
    struct position position1;

};

static unsigned int optwidth = 1920;
static unsigned int optheight = 1080;
static unsigned int optbpp = 4;
/*
static unsigned int keymod = KEYMOD_NONE;
*/
static struct display display;
static struct mouse mouse;
static struct configuration configuration;
static struct damage damage;
static struct widget widgets[32];
static struct window windows[32];
static unsigned int nwidgets;
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

#define RELX0X0 1
#define RELX0X1 2
#define RELX1X1 3

#if 0
#define BORDERRECT_COLOR_NORMAL 0

static unsigned int borderrectcmap[] = {
    0xFFFF0000,
};
static struct linesegment borderrect0[1] = {
    {RELX0X1, 1, -1, BORDERRECT_COLOR_NORMAL}
};

static struct linesegment borderrect1[2] = {
    {RELX0X0, 0, 1, BORDERRECT_COLOR_NORMAL},
    {RELX1X1, -1, 0, BORDERRECT_COLOR_NORMAL}
};
#endif

#define WINDOW_COLOR_SHADOW 0
#define WINDOW_COLOR_MAIN_LIGHT 1
#define WINDOW_COLOR_MAIN_NORMAL 2
#define WINDOW_COLOR_AREA_NORMAL 3

static unsigned int windowcmapnormal[] = {
    0xFF101010,
    0xFF687888,
    0xFF485868,
    0xFF182838
};
static unsigned int windowcmapfocus[] = {
    0xFF101010,
    0xFF48C888,
    0xFF28A868,
    0xFF182838
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

static void markforpaint(int x0, int y0, int x1, int y1)
{

    switch (damage.state)
    {

    case DAMAGE_STATE_NONE:
        damage.position0.x = x0;
        damage.position0.y = y0;
        damage.position1.x = x1;
        damage.position1.y = y1;

        break;

    case DAMAGE_STATE_MADE:
        if (x0 < damage.position0.x)
            damage.position0.x = x0;

        if (y0 < damage.position0.y)
            damage.position0.y = y0;

        if (x1 > damage.position1.x)
            damage.position1.x = x1;

        if (y1 > damage.position1.y)
            damage.position1.y = y1;

        break;

    }

    damage.state = DAMAGE_STATE_MADE;

}

static void blit_line(struct display *display, unsigned int x0, unsigned int x1, unsigned int color, unsigned int y)
{

    unsigned int *buffer = display->framebuffer;
    unsigned int x;

    x0 = capvalue(x0, 0, display->size.w);
    x1 = capvalue(x1, 0, display->size.w);

    for (x = x0; x < x1; x++)
        buffer[y * display->size.w + x] = color;

}

static void blit_cmap32line(struct display *display, struct position *p, struct image *image, unsigned int *cmap, unsigned int y)
{

    unsigned int *buffer = display->framebuffer;
    unsigned char *data = image->data;
    unsigned int x;

    for (x = 0; x < image->size.w; x++)
    {

        unsigned int soffset = (y * image->size.w + x);
        unsigned int toffset = (p->y * display->size.w + p->x) + (y * display->size.w + x);

        if (data[soffset] != 0xFF)
            buffer[toffset] = cmap[data[soffset]];

    }

}

static void paintlinesegment(int x0, int x1, unsigned int *cmap, struct linesegment *p, unsigned int y)
{

    int p0;
    int p1;

    switch (p->type)
    {

    case RELX0X0:
        p0 = x0 + p->p0;
        p1 = x0 + p->p1;

        break;

    case RELX0X1:
        p0 = x0 + p->p0;
        p1 = x1 + p->p1;

        break;

    case RELX1X1:
        p0 = x1 + p->p0;
        p1 = x1 + p->p1;

        break;

    default:
        p0 = x0;
        p1 = x1;

        break;

    }

    blit_line(&display, p0, p1, cmap[p->color], y);

}

static void paintlinesegments(int x0, int x1, unsigned int *cmap, struct linesegment *ls, unsigned int n, unsigned int y)
{

    unsigned int i;

    for (i = 0; i < n; i++)
        paintlinesegment(x0, x1, cmap, &ls[i], y);

}

static void paintmouse(struct mouse *m, unsigned int y)
{

    unsigned int *cmap = mousecmap;

    blit_cmap32line(&display, &m->position, &m->image, cmap, y - m->position.y);

}

static void paintborderrect(int px0, int py0, int px1, int py1, unsigned int y)
{

#if 0
    unsigned int *cmap = borderrectcmap;
    unsigned int ly = y - py0;

    if (ly == 0 || ly == (py1 - py0) - 1)
        paintlinesegments(px0, px1, cmap, borderrect0, 1, y);

    if (ly > 1 && ly < (py1 - py0) - 2)
        paintlinesegments(px0, px1, cmap, borderrect1, 2, y);
#endif

}

static void paintwindow(struct window *w, unsigned int y)
{

    unsigned int *cmap = (w->focus) ? windowcmapfocus : windowcmapnormal;
    unsigned int ly = y - w->position.y;
    struct linesegment *segments;
    unsigned int nsegments;

    if (ly == 0 || ly == w->size.h - 1)
    {

        static struct linesegment windowborder0[1] = {
            {RELX0X1, 1, -1, WINDOW_COLOR_SHADOW}
        };

        segments = windowborder0;
        nsegments = 1;

    }

    else if (ly == 1 || ly == w->size.h - 2)
    {

        static struct linesegment windowborder1[1] = {
            {RELX0X1, 0, 0, WINDOW_COLOR_SHADOW}
        };

        segments = windowborder1;
        nsegments = 1;

    }

    else if (ly == 2 || ly == w->size.h - 3)
    {

        static struct linesegment windowborder2[3] = {
            {RELX0X0, 0, 3, WINDOW_COLOR_SHADOW},
            {RELX0X1, 3, -3, WINDOW_COLOR_MAIN_LIGHT},
            {RELX1X1, -3, 0, WINDOW_COLOR_SHADOW}
        };

        segments = windowborder2;
        nsegments = 3;

    }

    else if (ly == 3 || ly == w->size.h - 4)
    {

        static struct linesegment windowborder3[5] = {
            {RELX0X0, 0, 2, WINDOW_COLOR_SHADOW},
            {RELX0X0, 2, 2, WINDOW_COLOR_MAIN_LIGHT},
            {RELX0X1, 4, -4, WINDOW_COLOR_MAIN_NORMAL},
            {RELX1X1, -4, -2, WINDOW_COLOR_MAIN_LIGHT},
            {RELX1X1, -2, 0, WINDOW_COLOR_SHADOW}
        };

        segments = windowborder3;
        nsegments = 5;

    }

    else if (ly >= 4 && ly < 40)
    {

        static struct linesegment windowbordertitle[5] = {
            {RELX0X0, 0, 2, WINDOW_COLOR_SHADOW},
            {RELX0X0, 2, 3, WINDOW_COLOR_MAIN_LIGHT},
            {RELX0X1, 3, -3, WINDOW_COLOR_MAIN_NORMAL},
            {RELX1X1, -3, -2, WINDOW_COLOR_MAIN_LIGHT},
            {RELX1X1, -2, 0, WINDOW_COLOR_SHADOW}
        };

        segments = windowbordertitle;
        nsegments = 5;

    }

    else if (ly == 40)
    {

        static struct linesegment windowborderspacing[7] = {
            {RELX0X0, 0, 2, WINDOW_COLOR_SHADOW},
            {RELX0X0, 2, 3, WINDOW_COLOR_MAIN_LIGHT},
            {RELX0X0, 3, 4, WINDOW_COLOR_MAIN_NORMAL},
            {RELX0X1, 4, -4, WINDOW_COLOR_SHADOW},
            {RELX1X1, -4, -3, WINDOW_COLOR_MAIN_NORMAL},
            {RELX1X1, -3, -2, WINDOW_COLOR_MAIN_LIGHT},
            {RELX1X1, -2, 0, WINDOW_COLOR_SHADOW}
        };

        segments = windowborderspacing;
        nsegments = 7;

    }

    else if (ly > 40 && ly < w->size.h - 4)
    {

        static struct linesegment windowborderarea[9] = {
            {RELX0X0, 0, 2, WINDOW_COLOR_SHADOW},
            {RELX0X0, 2, 3, WINDOW_COLOR_MAIN_LIGHT},
            {RELX0X0, 3, 4, WINDOW_COLOR_MAIN_NORMAL},
            {RELX0X0, 4, 5, WINDOW_COLOR_SHADOW},
            {RELX0X1, 5, -5, WINDOW_COLOR_AREA_NORMAL},
            {RELX1X1, -5, -4, WINDOW_COLOR_SHADOW},
            {RELX1X1, -4, -3, WINDOW_COLOR_MAIN_NORMAL},
            {RELX1X1, -3, -2, WINDOW_COLOR_MAIN_LIGHT},
            {RELX1X1, -2, 0, WINDOW_COLOR_SHADOW}
        };

        segments = windowborderarea;
        nsegments = 9;

    }

    else
    {

        segments = 0;
        nsegments = 0;

    }

    paintlinesegments(w->position.x, w->position.x + w->size.w, cmap, segments, nsegments, y);

}

static int intersects(unsigned int y, unsigned int y0, unsigned int y1)
{

    return y >= y0 && y < y1;

}

static void checkbackground(unsigned int y)
{

    if (intersects(y, 0, display.size.h))
        blit_line(&display, damage.position0.x, damage.position1.x, 0xFF142434, y);

}

static void checkwindow(struct window *window, unsigned int y)
{

    if (intersects(y, window->position.y, window->position.y + window->size.h))
        paintwindow(window, y);

}

static void checkmouse(struct mouse *mouse, unsigned int y)
{

    if (intersects(y, mouse->position.y, mouse->position.y + mouse->image.size.h))
        paintmouse(mouse, y);

}

static void paint(void)
{

    if (damage.state == DAMAGE_STATE_MADE)
    {

        unsigned int y;

        for (y = damage.position0.y; y < damage.position1.y; y++)
        {

            unsigned int i;

            checkbackground(y);

            for (i = 0; i < nwidgets; i++)
            {

                struct widget *widget = &widgets[i];

                switch (widget->type)
                {

                case WIDGET_TYPE_WINDOW:
                    checkwindow(widget->data, y);

                    break;

                }

            }

            checkmouse(&mouse, y);
            paintborderrect(damage.position0.x, damage.position0.y, damage.position1.x, damage.position1.y, y);

        }

        damage.state = DAMAGE_STATE_NONE;

    }

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
    {

        if (display.framebuffer)
            paint();

    }

    file_unlink(FILE_G4);
    file_unlink(FILE_G2);
    file_unlink(FILE_G1);
    file_unlink(FILE_G0);

}

static struct window *getfocusedwindow(void)
{

    unsigned int i;

    for (i = 0; i < nwidgets; i++)
    {

        struct window *w = &windows[i];

        if (w->focus)
            return w;

    }

    return 0;

}

static void onmousemove(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousemove *mousemove = mdata;

    markforpaint(mouse.position.x, mouse.position.y, mouse.position.x + mouse.image.size.w, mouse.position.y + mouse.image.size.h);

    mouse.position.x = capvalue(mouse.position.x + mousemove->relx, 0, display.size.w);
    mouse.position.y = capvalue(mouse.position.y + mousemove->rely, 0, display.size.h);

    markforpaint(mouse.position.x, mouse.position.y, mouse.position.x + mouse.image.size.w, mouse.position.y + mouse.image.size.h);

    if (mouse.drag)
    {

        struct window *window = getfocusedwindow();

        markforpaint(window->position.x, window->position.y, window->position.x + window->size.w, window->position.y + window->size.h);

        window->position.x += mousemove->relx;
        window->position.y += mousemove->rely;

        markforpaint(window->position.x, window->position.y, window->position.x + window->size.w, window->position.y + window->size.h);

    }

    if (mouse.resize)
    {

        struct window *window = getfocusedwindow();
        int w = (int)(window->size.w) + mousemove->relx;
        int h = (int)(window->size.h) + mousemove->rely;

        if (w < WINDOW_MIN_HEIGHT)
            w = WINDOW_MIN_WIDTH;

        if (h < WINDOW_MIN_HEIGHT)
            h = WINDOW_MIN_HEIGHT;

        markforpaint(window->position.x, window->position.y, window->position.x + window->size.w, window->position.y + window->size.h);

        window->size.w = w;
        window->size.h = h;

        markforpaint(window->position.x, window->position.y, window->position.x + window->size.w, window->position.y + window->size.h);

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

    display.framebuffer = videomode->framebuffer;
    display.size.w = videomode->w;
    display.size.h = videomode->h;
    display.bpp = videomode->bpp;
    configuration.lineheight = 12 + factor * 4;
    configuration.padding = 4 + factor * 2;
    configuration.steplength = videomode->w / 12;

    loadfont(factor);

    setmouse(videomode->w / 4, videomode->h / 4, factor);

    markforpaint(0, 0, display.size.w, display.size.h);

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

    widgets[0].type = WIDGET_TYPE_WINDOW;
    widgets[0].id = "window0";
    widgets[0].parent = "";
    widgets[0].data = &windows[0];
    windows[0].title = "Window 0";
    windows[0].focus = 0;
    windows[0].size.w = 800;
    windows[0].size.h = 600;
    windows[0].position.x = 200;
    windows[0].position.y = 100;
    widgets[1].type = WIDGET_TYPE_WINDOW;
    widgets[1].id = "window1";
    widgets[1].parent = "";
    widgets[1].data = &windows[1];
    windows[1].title = "Window 1";
    windows[1].focus = 1;
    windows[1].size.w = 800;
    windows[1].size.h = 600;
    windows[1].position.x = 100;
    windows[1].position.y = 80;
    nwidgets = 2;

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

