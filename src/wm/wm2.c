#include <fudge.h>
#include <abi.h>

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

    struct position position;
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

struct window
{

    char *title;
    unsigned int focus;
    struct position position;
    struct size size;

};

struct repaint
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
static struct repaint repaint;
static struct window window1;
static struct window window2;
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
    0xFFA0A0A0,
    0xFF808080,
    0xFF242424
};
static unsigned int windowcmapfocus[] = {
    0xFF101010,
    0xFFA8C898,
    0xFF88A878,
    0xFF242424
};
static struct linesegment windowborder0[1] = {
    {RELX0X1, 1, -1, WINDOW_COLOR_SHADOW}
};

static struct linesegment windowborder1[1] = {
    {RELX0X1, 0, 0, WINDOW_COLOR_SHADOW}
};

static struct linesegment windowborder2[3] = {
    {RELX0X0, 0, 3, WINDOW_COLOR_SHADOW},
    {RELX0X1, 3, -3, WINDOW_COLOR_MAIN_LIGHT},
    {RELX1X1, -3, 0, WINDOW_COLOR_SHADOW}
};

static struct linesegment windowborder3[5] = {
    {RELX0X0, 0, 2, WINDOW_COLOR_SHADOW},
    {RELX0X0, 2, 2, WINDOW_COLOR_MAIN_LIGHT},
    {RELX0X1, 4, -4, WINDOW_COLOR_MAIN_NORMAL},
    {RELX1X1, -4, -2, WINDOW_COLOR_MAIN_LIGHT},
    {RELX1X1, -2, 0, WINDOW_COLOR_SHADOW}
};

static struct linesegment windowbordertitle[5] = {
    {RELX0X0, 0, 2, WINDOW_COLOR_SHADOW},
    {RELX0X0, 2, 3, WINDOW_COLOR_MAIN_LIGHT},
    {RELX0X1, 3, -3, WINDOW_COLOR_MAIN_NORMAL},
    {RELX1X1, -3, -2, WINDOW_COLOR_MAIN_LIGHT},
    {RELX1X1, -2, 0, WINDOW_COLOR_SHADOW}
};

static struct linesegment windowborderspacing[7] = {
    {RELX0X0, 0, 2, WINDOW_COLOR_SHADOW},
    {RELX0X0, 2, 3, WINDOW_COLOR_MAIN_LIGHT},
    {RELX0X0, 3, 4, WINDOW_COLOR_MAIN_NORMAL},
    {RELX0X1, 4, -4, WINDOW_COLOR_SHADOW},
    {RELX1X1, -4, -3, WINDOW_COLOR_MAIN_NORMAL},
    {RELX1X1, -3, -2, WINDOW_COLOR_MAIN_LIGHT},
    {RELX1X1, -2, 0, WINDOW_COLOR_SHADOW}
};

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

static void posset(struct position *p0, struct position *p1, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{

    p0->x = x0;
    p0->y = y0;
    p1->x = x1;
    p1->y = y1;

}

static void posshrink(struct position *p0, struct position *p1, int x0, int y0, int x1, int y1)
{

    if (x0 < p0->x)
        p0->x = x0;

    if (y0 < p0->y)
        p0->y = y0;

    if (x1 > p1->x)
        p1->x = x1;

    if (y1 > p1->y)
        p1->y = y1;

}

static void markforpaint(int x0, int y0, int x1, int y1)
{

    if (repaint.state)
        posshrink(&repaint.position0, &repaint.position1, x0, y0, x1, y1);
    else
        posset(&repaint.position0, &repaint.position1, x0, y0, x1, y1);

    repaint.state = 1;

}

static void blit_line(unsigned int x0, unsigned int x1, unsigned int w, unsigned int color, unsigned int y)
{

    unsigned int *buffer = display.framebuffer;
    unsigned int x;

    for (x = x0; x < x1; x++)
        buffer[y * w + x] = color;

}

static void blit_cmap32line(struct position *p, struct image *image, unsigned int *cmap, unsigned int tw, unsigned int y)
{

    unsigned int *buffer = display.framebuffer;
    unsigned char *data = image->data;
    unsigned int x;

    for (x = 0; x < image->size.w; x++)
    {

        unsigned int soffset = (y * image->size.w + x);
        unsigned int toffset = (p->y * tw + p->x) + (y * tw + x);

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

    blit_line(p0, p1, display.size.w, cmap[p->color], y);

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
    unsigned int ly = y - m->position.y;

    blit_cmap32line(&m->position, &m->image, cmap, display.size.w, ly);

}

#if 0
static void paintborderrect(int px0, int px1, unsigned int pw, unsigned int y)
{

    unsigned int *cmap = borderrectcmap;
    unsigned int ly = y - py;

    if (ly == 0 || ly == area->size.h - 1)
        paintlinesegments(px0, px1, cmap, borderrect0, 1, y);

    if (ly > 1 && ly < area->size.h - 2)
        paintlinesegments(px0, px1, cmap, borderrect1, 2, y);

}
#endif

static void paintwindow(struct window *w, unsigned int y)
{

    unsigned int *cmap = (w->focus) ? windowcmapfocus : windowcmapnormal;
    unsigned int ly = y - w->position.y;

    if (ly == 0 || ly == w->size.h - 1)
        paintlinesegments(w->position.x, w->position.x + w->size.w, cmap, windowborder0, 1, y);

    if (ly == 1 || ly == w->size.h - 2)
        paintlinesegments(w->position.x, w->position.x + w->size.w, cmap, windowborder1, 1, y);

    if (ly == 2 || ly == w->size.h - 3)
        paintlinesegments(w->position.x, w->position.x + w->size.w, cmap, windowborder2, 3, y);

    if (ly == 3 || ly == w->size.h - 4)
        paintlinesegments(w->position.x, w->position.x + w->size.w, cmap, windowborder3, 5, y);

    if (ly >= 4 && ly < 40)
        paintlinesegments(w->position.x, w->position.x + w->size.w, cmap, windowbordertitle, 5, y);

    if (ly == 40)
        paintlinesegments(w->position.x, w->position.x + w->size.w, cmap, windowborderspacing, 7, y);

    if (ly > 40 && ly < w->size.h - 4)
        paintlinesegments(w->position.x, w->position.x + w->size.w, cmap, windowborderarea, 9, y);

}

static int intersects(unsigned int y, unsigned int y0, unsigned int y1)
{

    return y >= y0 && y < y1;

}

static void paint(void)
{

    if (repaint.state)
    {

        unsigned int y;

        for (y = repaint.position0.y; y < repaint.position1.y; y++)
        {

            if (intersects(y, display.position.y, display.position.y + display.size.h))
                blit_line(repaint.position0.x, repaint.position1.x, display.size.w, 0xFF202020, y);

            if (intersects(y, window2.position.y, window2.position.y + window2.size.h))
                paintwindow(&window2, y);

            if (intersects(y, window1.position.y, window1.position.y + window1.size.h))
                paintwindow(&window1, y);

            if (intersects(y, mouse.position.y, mouse.position.y + mouse.image.size.h))
                paintmouse(&mouse, y);

            #if 0
            paintborderrect(repaint.point0.x, repaint.point0.y, repaint.point1.x - repaint.point0.x, 0, y);
            #endif
        }

        repaint.state = 0;

    }

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

        if (display.framebuffer)
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

    markforpaint(mouse.position.x, mouse.position.y, mouse.position.x + mouse.image.size.w, mouse.position.y + mouse.image.size.h);

    mouse.position.x += mousemove->relx;
    mouse.position.y += mousemove->rely;

    if (mouse.position.x < display.position.x || mouse.position.x >= display.position.x + display.size.w)
        mouse.position.x = (mousemove->relx < 0) ? display.position.x : display.position.x + display.size.w - 1;

    if (mouse.position.y < display.position.y || mouse.position.y >= display.position.y + display.size.h)
        mouse.position.y = (mousemove->rely < 0) ? display.position.y : display.position.y + display.size.h - 1;

    markforpaint(mouse.position.x, mouse.position.y, mouse.position.x + mouse.image.size.w, mouse.position.y + mouse.image.size.h);

    if (mouse.drag)
    {

        markforpaint(window1.position.x, window1.position.y, window1.position.x + window1.size.w, window1.position.y + window1.size.h);

        window1.position.x += mousemove->relx;
        window1.position.y += mousemove->rely;

        markforpaint(window1.position.x, window1.position.y, window1.position.x + window1.size.w, window1.position.y + window1.size.h);

    }

    if (mouse.resize)
    {

        markforpaint(window1.position.x, window1.position.y, window1.position.x + window1.size.w, window1.position.y + window1.size.h);

        window1.size.w += mousemove->relx;
        window1.size.h += mousemove->rely;

        markforpaint(window1.position.x, window1.position.y, window1.position.x + window1.size.w, window1.position.y + window1.size.h);

    }

}

static void onmousepress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
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

static void onmouserelease(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
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

    display.framebuffer = videomode->framebuffer;
    display.position.x = 0;
    display.position.y = 0;
    display.size.w = videomode->w;
    display.size.h = videomode->h;
    display.bpp = videomode->bpp;
    configuration.lineheight = 12 + factor * 4;
    configuration.padding = 4 + factor * 2;
    configuration.steplength = videomode->w / 12;

    loadfont(factor);

    setmouse(videomode->w / 4, videomode->h / 4, factor);

    markforpaint(display.position.x, display.position.y, display.position.x + display.size.w, display.position.y + display.size.h);

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

    window1.title = "Example1";
    window1.focus = 1;
    window1.size.w = 800;
    window1.size.h = 600;
    window1.position.x = 100;
    window1.position.y = 80;

    window2.title = "Example2";
    window2.focus = 0;
    window2.size.w = 800;
    window2.size.h = 600;
    window2.position.x = 200;
    window2.position.y = 100;

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

