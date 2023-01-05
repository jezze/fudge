#include <fudge.h>
#include <math.h>
#include <abi.h>

static unsigned int *framebuffer;
static unsigned int w, h;

double restrain(double x)
{

    double g = 2 * MATH_PI;

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

    int dx = math_abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -math_abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

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

struct vector2
{

    double x;
    double y;

};

struct vector3
{

    double x;
    double y;
    double z;

};

struct vector2 vector2_create(double x, double y)
{

    struct vector2 n;

    n.x = x;
    n.y = y;

    return n;

}

struct vector2 vector2_add(struct vector2 *v, double x, double y)
{

    struct vector2 n;

    n.x = v->x + x;
    n.y = v->y + y;

    return n;

}

struct vector2 vector2_add2(struct vector2 *v1, struct vector2 *v2)
{

    return vector2_add(v1, v2->x, v2->y);

}

struct vector2 vector2_mul(struct vector2 *v, double x, double y)
{

    struct vector2 n;

    n.x = v->x * x;
    n.y = v->y * y;

    return n;

}

struct vector2 vector2_mul2(struct vector2 *v1, struct vector2 *v2)
{

    return vector2_mul(v1, v2->x, v2->y);

}

struct vector3 vector3_create(double x, double y, double z)
{

    struct vector3 n;

    n.x = x;
    n.y = y;
    n.z = z;

    return n;

}

struct vector3 vector3_rotatex(struct vector3 *v, double theta)
{

    double s = math_sin(theta);
    double c = math_cos(theta);
    struct vector3 n;

    n.x = v->x;
    n.y = v->y * c - v->z * s;
    n.z = v->z * c + v->y * s;

    return n;

}

struct vector3 vector3_rotatey(struct vector3 *v, double theta)
{

    double s = math_sin(theta);
    double c = math_cos(theta);
    struct vector3 n;

    n.x = v->x * c + v->z * s;
    n.y = v->y;
    n.z = v->z * c - v->x * s;

    return n;

}

struct vector3 vector3_rotatez(struct vector3 *v, double theta)
{

    double s = math_sin(theta);
    double c = math_cos(theta);
    struct vector3 n;

    n.x = v->x * c - v->y * s;
    n.y = v->y * c + v->x * s;
    n.z = v->z;

    return n;

}

struct vector3 vector3_add(struct vector3 *v, double x, double y, double z)
{

    struct vector3 n;

    n.x = v->x + x;
    n.y = v->y + y;
    n.z = v->z + z;

    return n;

}

struct point
{

    struct vector2 position;
    struct vector2 radius;
    double angle;
    double speed;

};

static struct point pp0;
static struct point pp1;
static struct vector2 t0;
static struct vector2 c0;
static struct vector2 t1;
static struct vector2 c1;
static struct vector3 nodes[8];

static void translate(double x, double y)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
        nodes[i] = vector3_add(&nodes[i], x, y, 0);

}

static void rotatex(double theta)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
        nodes[i] = vector3_rotatex(&nodes[i], theta);

}

static void rotatey(double theta)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
        nodes[i] = vector3_rotatey(&nodes[i], theta);

}

static void rotatez(double theta)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
        nodes[i] = vector3_rotatez(&nodes[i], theta);

}

static void setup_scene1(void)
{

    pp0.position = vector2_create(280, 200);
    pp0.radius = vector2_create(180, 100);
    pp0.angle = MATH_PI;
    pp0.speed = 0.05;
    pp1.position = vector2_create(360, 220);
    pp1.radius = vector2_create(80, 80);
    pp1.angle = 0;
    pp1.speed = 0.07;

}

static void setup_scene2(void)
{

    nodes[0] = vector3_create(-100, -100, -100);
    nodes[1] = vector3_create(-100, -100, 100);
    nodes[2] = vector3_create(-100, 100, -100);
    nodes[3] = vector3_create(-100, 100, 100);
    nodes[4] = vector3_create(100, -100, -100);
    nodes[5] = vector3_create(100, -100, 100);
    nodes[6] = vector3_create(100, 100, -100);
    nodes[7] = vector3_create(100, 100, 100);

    rotatez(MATH_PI / 4);
    rotatey(MATH_PI / 4);
    rotatex(MATH_PI / 4);
    translate(320, 240);

}

static void render_scene1(unsigned int frame)
{

    pp0.angle = restrain(pp0.angle + pp0.speed);
    t0 = vector2_mul(&pp0.radius, math_cos(pp0.angle), math_sin(pp0.angle));
    c0 = vector2_add2(&pp0.position, &t0);
    pp1.angle = restrain(pp1.angle + pp1.speed);
    t1 = vector2_mul(&pp1.radius, math_cos(pp1.angle), math_sin(pp1.angle));
    c1 = vector2_add2(&pp1.position, &t1);

    clearscreen(0xFF001020);
    putline(c0.x, c0.y, c1.x, c1.y, 0xFFFFFFFF);
    putsquare(c0.x - 4, c0.y - 4, 8, 8, 0xFFFFFFFF);
    putcircle(c1.x, c1.y, 8, 0xFFFFFFFF);

}

static void putvertex(struct vector3 *v, unsigned int color)
{

    putcircle(v->x, v->y, 8, color);

}

static void putedge(struct vector3 *v1, struct vector3 *v2, unsigned int color)
{

    putline(v1->x, v1->y, v2->x, v2->y, color);

}

static void render_scene2(unsigned int frame)
{

    putedge(&nodes[0], &nodes[1], 0xFFFFFFFF);
    putedge(&nodes[1], &nodes[3], 0xFFFFFFFF);
    putedge(&nodes[3], &nodes[2], 0xFFFFFFFF);
    putedge(&nodes[2], &nodes[0], 0xFFFFFFFF);
    putedge(&nodes[4], &nodes[5], 0xFFFFFFFF);
    putedge(&nodes[5], &nodes[7], 0xFFFFFFFF);
    putedge(&nodes[7], &nodes[6], 0xFFFFFFFF);
    putedge(&nodes[6], &nodes[4], 0xFFFFFFFF);
    putedge(&nodes[0], &nodes[4], 0xFFFFFFFF);
    putedge(&nodes[1], &nodes[5], 0xFFFFFFFF);
    putedge(&nodes[2], &nodes[6], 0xFFFFFFFF);
    putedge(&nodes[3], &nodes[7], 0xFFFFFFFF);
    putvertex(&nodes[0], 0xFFFFFFFF);
    putvertex(&nodes[1], 0xFFFFFFFF);
    putvertex(&nodes[2], 0xFFFFFFFF);
    putvertex(&nodes[3], 0xFFFFFFFF);
    putvertex(&nodes[4], 0xFFFFFFFF);
    putvertex(&nodes[5], 0xFFFFFFFF);
    putvertex(&nodes[6], 0xFFFFFFFF);
    putvertex(&nodes[7], 0xFFFFFFFF);

}

static void render(unsigned int frame)
{

    if (frame < 10)
    {

        render_scene1(frame);

    }

    else
    {

        render_scene1(frame);
        render_scene2(frame);

    }

}

static void run(void)
{

    struct message message;
    unsigned int frame = 0;

    setup_scene1();
    setup_scene2();

    while (channel_pick(&message))
    {

        switch (message.header.event)
        {

        case EVENT_TIMERTICK:
            render(frame++);

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

    settings.width = option_getdecimal("width");
    settings.height = option_getdecimal("height");
    settings.bpp = option_getdecimal("bpp");

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
    file_link(FILE_G0);
    file_link(FILE_G1);
    file_link(FILE_G2);
    file_writeall(FILE_L1, &settings, sizeof (struct ctrl_videosettings));
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

