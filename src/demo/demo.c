#include <fudge.h>
#include <abi.h>

#define PI 3.1415926

static unsigned int *framebuffer;
static unsigned int w, h;
static int counter;

struct vertex
{

    double x;
    double y;

};

double abs(double x)
{

    return x > 0 ? x : -x;

}

double pow(double a, double b)
{

    double c = 1;
    unsigned int i;

    for (i = 0; i < b; i++)
        c *= a;

    return c;

}

double fact(double x)
{

    double ret = 1;
    unsigned int i;

    for (i = 1; i <= x; i++)
        ret *= i;

    return ret;

}

double sin(double x)
{

    double y = x;
    double s = -1;
    unsigned int i;

    for (i = 3; i <= 100; i += 2)
    {

        y += s * (pow(x, i) / fact(i));
        s *= -1;

    }

    return y;

}

double cos(double x)
{

    double y = 1;
    double s = -1;
    unsigned int i;

    for (i = 2; i <= 100; i += 2)
    {

        y += s * (pow(x, i) / fact(i));
        s *= -1;

    }

    return y;

}

double tan(double x)
{

    return (sin(x) / cos(x));

}

double restrain(double x)
{

    double g = 2 * PI;

    if (x > g)
        x -= g;
    else if (x < -g)
        x += g;

    return x;

}

static void clearscreen(unsigned int color)
{

    unsigned int total = w * h;
    unsigned int i;

    for (i = 0; i < total; i++)
        framebuffer[i] = color;

}

void putpixel(int x, int y, unsigned int color)
{

    unsigned int offset = y * w + x;

    framebuffer[offset] = color;

}

void putline(int x0, int y0, int x1, int y1, unsigned int color)
{

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;)
    {

        putpixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        e2 = 2 * err;

        if (e2 >= dy)
        {

            err += dy;
            x0 += sx;

        }

        if (e2 <= dx)
        {

            err += dx;
            y0 += sy;

        }

    }

}

void putsquare(int x, int y, int w, int h, unsigned int color)
{

    int cx;
    int cy;
    int cw = x + w;
    int ch = y + h;

    for (cy = y; cy < ch; cy++)
    {

        for (cx = x; cx < cw; cx++)
        {

            putpixel(cx, cy, color);

        }

    }

}

void putcircle(int xm, int ym, int r, unsigned int color)
{

   int x = -r;
   int y = 0;
   int err = 2 - 2 * r;

   do
   {

      putpixel(xm - x, ym + y, color);
      putpixel(xm - y, ym - x, color);
      putpixel(xm + x, ym - y, color);
      putpixel(xm + y, ym + x, color);

      r = err;

      if (r > x)
        err += ++x * 2 + 1;

      if (r <= y)
        err += ++y * 2 + 1;

   } while (x < 0);

}

static void run(void)
{

    struct message message;
    struct vertex p0;
    struct vertex d0;
    struct vertex c0;
    double r0;
    double s0;
    struct vertex p1;
    struct vertex d1;
    struct vertex c1;
    double r1;
    double s1;

    p0.x = 160;
    p0.y = 160;
    d0.x = PI;
    d0.y = PI;
    r0 = 100;
    s0 = 0.05;
    p1.x = 300;
    p1.y = 220;
    d1.x = 0;
    d1.y = 0;
    r1 = 80;
    s1 = 0.07;

    while (channel_pick(&message))
    {

        switch (message.header.event)
        {

        case EVENT_TIMERTICK:
            counter++;
            d0.x = restrain(d0.x + s0);
            d0.y = restrain(d0.y + s0);
            c0.x = p0.x + (r0 * cos(d0.x));
            c0.y = p0.y + (r0 * sin(d0.y));
            d1.x = restrain(d1.x + s1);
            d1.y = restrain(d1.y + s1);
            c1.x = p1.x + (r1 * cos(d1.x));
            c1.y = p1.y + (r1 * sin(d1.y));

            clearscreen(0xFF001020);
            putline(c0.x, c0.y, c1.x, c1.y, 0xFFFFFFFF);
            putsquare(c0.x - 4, c0.y - 4, 8, 8, 0xFFFFFFFF);
            putcircle(c1.x, c1.y, 8, 0xFFFFFFFF);

            break;

        case EVENT_KEYPRESS:
            break;

        case EVENT_KEYRELEASE:
            break;

        default:
            channel_dispatch(&message);

            break;

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, "system:service/wm"))
        file_notify(FILE_L0, EVENT_WMMAP, 0, 0);
    else
        channel_warning("Could not open window manager service");

}

static void onterm(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(EVENT_WMUNGRAB);
    channel_send(EVENT_WMUNMAP);
    channel_close();

}

static void onvideomode(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;

    framebuffer = videomode->framebuffer;
    w = videomode->w;
    h = videomode->h;

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_videosettings settings;

    if (!file_walk2(FILE_L0, option_getstring("keyboard")))
        channel_warning("Could not open keyboard");

    if (!file_walk(FILE_G0, FILE_L0, "event"))
        channel_warning("Could not open keyboard event");

    if (!file_walk2(FILE_L0, option_getstring("video")))
        channel_error("Could not find video device");

    if (!file_walk(FILE_L1, FILE_L0, "ctrl"))
        channel_error("Could not find video device ctrl");

    if (!file_walk(FILE_G1, FILE_L0, "event"))
        channel_warning("Could not open video event");

    if (!file_walk2(FILE_L0, option_getstring("timer")))
        channel_error("Could not find timer device");

    if (!file_walk(FILE_G2, FILE_L0, "event1"))
        channel_warning("Could not open timer event");

    channel_send(EVENT_WMGRAB);

    settings.width = option_getdecimal("width");
    settings.height = option_getdecimal("height");
    settings.bpp = option_getdecimal("bpp");

    file_link(FILE_G0);
    file_link(FILE_G1);
    file_link(FILE_G2);
    file_seekwriteall(FILE_L1, &settings, sizeof (struct ctrl_videosettings), 0);
    run();
    file_unlink(FILE_G2);
    file_unlink(FILE_G1);
    file_unlink(FILE_G0);

}

void init(void)
{

    option_add("width", "640");
    option_add("height", "480");
    option_add("bpp", "4");
    option_add("keyboard", "system:keyboard");
    option_add("timer", "system:timer/if:0");
    option_add("video", "system:video/if:0");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TERM, onterm);
    channel_bind(EVENT_VIDEOMODE, onvideomode);
    channel_bind(EVENT_WMINIT, onwminit);

}

