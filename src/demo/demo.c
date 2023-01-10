#include <fudge.h>
#include <math.h>
#include <abi.h>

static unsigned int *framebuffer;
static unsigned int wmax, hmax, wmid, hmid;

double constrain(double x)
{

    return math_mod(x, 2 * MATH_PI);

}

static void clearscreen(unsigned int color)
{

    if (framebuffer)
    {

        unsigned int total = wmax * hmax;
        unsigned int i;

        for (i = 0; i < total; i++)
            framebuffer[i] = color;

    }

}

void putpixel(int x, int y, unsigned int color)
{

    if (framebuffer && x < wmax && y < hmax)
    {

        unsigned int offset = y * wmax + x;

        framebuffer[offset] = color;

    }

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
    int cw = x + wmax;
    int ch = y + hmax;

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

struct matrix3x3
{

    double x0;
    double y0;
    double z0;
    double x1;
    double y1;
    double z1;
    double x2;
    double y2;
    double z2;

};

struct matrix3x3 matrix3x3_create(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2)
{

    struct matrix3x3 m;

    m.x0 = x0;
    m.y0 = y0;
    m.z0 = z0;
    m.x1 = x1;
    m.y1 = y1;
    m.z1 = z1;
    m.x2 = x2;
    m.y2 = y2;
    m.z2 = z2;

    return m;

}

struct matrix3x3 matrix3x3_zero(void)
{

    struct matrix3x3 m;

    m.x0 = 0;
    m.y0 = 0;
    m.z0 = 0;
    m.x1 = 0;
    m.y1 = 0;
    m.z1 = 0;
    m.x2 = 0;
    m.y2 = 0;
    m.z2 = 0;

    return m;

}

struct matrix3x3 matrix3x3_identity(void)
{

    struct matrix3x3 m;

    m.x0 = 1;
    m.y0 = 0;
    m.z0 = 0;
    m.x1 = 0;
    m.y1 = 1;
    m.z1 = 0;
    m.x2 = 0;
    m.y2 = 0;
    m.z2 = 1;

    return m;

}

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

struct vector3 vector3_mul_matrix3x3(struct vector3 *v, struct matrix3x3 *m)
{

    struct vector3 n;

    n.x = v->x * m->x0 + v->y * m->y0 + v->z * m->z0;
    n.y = v->x * m->x1 + v->y * m->y1 + v->z * m->z1;
    n.z = v->x * m->x2 + v->y * m->y2 + v->z * m->z2;

    return n;

}

struct vector3 vector3_arotatex(struct vector3 *v, double s, double c)
{

    struct matrix3x3 m = matrix3x3_create(1, 0, 0, 0, c, -s, 0, s, c);

    return vector3_mul_matrix3x3(v, &m);

}

struct vector3 vector3_rotatex(struct vector3 *v, double theta)
{

    return vector3_arotatex(v, math_sin(theta), math_cos(theta));

}

struct vector3 vector3_arotatey(struct vector3 *v, double s, double c)
{

    struct matrix3x3 m = matrix3x3_create(c, 0, s, 0, 1, 0, -s, 0, c);

    return vector3_mul_matrix3x3(v, &m);

}

struct vector3 vector3_rotatey(struct vector3 *v, double theta)
{

    return vector3_arotatey(v, math_sin(theta), math_cos(theta));

}

struct vector3 vector3_arotatez(struct vector3 *v, double s, double c)
{

    struct matrix3x3 m = matrix3x3_create(c, -s, 0, s, c, 0, 0, 0, 1);

    return vector3_mul_matrix3x3(v, &m);

}

struct vector3 vector3_rotatez(struct vector3 *v, double theta)
{

    return vector3_arotatez(v, math_sin(theta), math_cos(theta));

}

struct vector3 vector3_add(struct vector3 *v, double x, double y, double z)
{

    struct vector3 n;

    n.x = v->x + x;
    n.y = v->y + y;
    n.z = v->z + z;

    return n;

}

struct vector3 vector3_add_vector3(struct vector3 *v1, struct vector3 *v2)
{

    struct vector3 n;

    n.x = v1->x + v2->x;
    n.y = v1->y + v2->y;
    n.z = v1->z + v2->z;

    return n;

}

static struct vector3 cam;
static struct vector3 nodeslocal[8];
static struct vector3 nodes[8];
static struct vector3 r;
static struct vector3 dr;
static double size = 0;
static unsigned int bcolor = 0xFF001020;
static unsigned int ncolor = 0xFFFFFF00;
static unsigned int ecolor = 0xFFFFFF00;
static unsigned int ccolor = 0xFFFF00FF;

static void translate(struct vector3 *vs, unsigned int count, double x, double y, double z)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct vector3 *v = &vs[i];

        vs[i] = vector3_add(v, x, y, z);

    }

}

static void rotate(struct vector3 *vs, unsigned int count, struct vector3 *r)
{

    double sx = math_sin(r->x);
    double cx = math_cos(r->x);
    double sy = math_sin(r->y);
    double cy = math_cos(r->y);
    double sz = math_sin(r->z);
    double cz = math_cos(r->z);
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct vector3 *v = &vs[i];

        vs[i] = vector3_arotatex(v, sx, cx);
        vs[i] = vector3_arotatey(v, sy, cy);
        vs[i] = vector3_arotatez(v, sz, cz);

    }

}

static void projectnode(struct vector3 *v, unsigned int color)
{

    double scale = hmid;
    double z = (cam.z / (cam.z + v->z));
    double x = (v->x - cam.x) * z;
    double y = (v->y - cam.y) * z;

    putcircle(x * scale + wmid, y * scale + hmid, 8, color);

}

static void projectnode2(struct vector3 *v, unsigned int color)
{

    double scale = hmid;
    double z = (cam.z / (cam.z + v->z));
    double x = (v->x - cam.x) * z;
    double y = (v->y - cam.y) * z;

    putcircle(x * scale + wmid, y * scale + hmid, (-v->z + cam.z) * 40, color);

}

static void projectedge(struct vector3 *v1, struct vector3 *v2, unsigned int color)
{

    double scale = hmid;
    double z1 = (cam.z / (cam.z + v1->z));
    double x1 = (v1->x - cam.x) * z1;
    double y1 = (v1->y - cam.y) * z1;
    double z2 = (cam.z / (cam.z + v2->z));
    double x2 = (v2->x - cam.x) * z2;
    double y2 = (v2->y - cam.y) * z2;

    putline(x1 * scale + wmid, y1 * scale + hmid, x2 * scale + wmid, y2 * hmid + hmid, color);

}

static void setup_scene1(void)
{

}

static void setup_scene2(void)
{

    cam = vector3_create(0.0, 0.0, 2.0);
    r = vector3_create(0.1, 0.2, 0.3);
    dr = vector3_create(0.02, 0.02, 0.02);
    nodeslocal[0] = vector3_create(-0.5, -0.5, -0.5);
    nodeslocal[1] = vector3_create(-0.5, -0.5, 0.5);
    nodeslocal[2] = vector3_create(-0.5, 0.5, -0.5);
    nodeslocal[3] = vector3_create(-0.5, 0.5, 0.5);
    nodeslocal[4] = vector3_create(0.5, -0.5, -0.5);
    nodeslocal[5] = vector3_create(0.5, -0.5, 0.5);
    nodeslocal[6] = vector3_create(0.5, 0.5, -0.5);
    nodeslocal[7] = vector3_create(0.5, 0.5, 0.5);

}

static double qsin(double x)
{

    return math_sin(constrain(x));

}

/*
static double qcos(double x)
{

    return math_cos(constrain(x));

}
*/

static void render_scene1(unsigned int frame, unsigned int localframe)
{

    unsigned int x;
    unsigned int y;
    double dv = 0.1;

    for (y = 0; y < hmax; y++)
    {

        for (x = 0; x < wmax; x++)
        {

/*
            double dx = x + 0.5 * qsin(sec / 5.0);
            double dy = y + 0.5 * qcos(sec / 3.0);
            double dv = qsin(x * 10 + sec) + qsin(10 * (x * qsin(sec / 2.0) + y * qcos(sec / 3.0)) + sec) + qsin(math_sqrt(100 * (dx * dx + dy * dy) + 1) + sec);
*/

            unsigned char r = (unsigned char)(255.0 * math_abs(qsin(dv * MATH_PI)));
            unsigned char g = (unsigned char)(255.0 * math_abs(qsin(dv * MATH_PI + 2.0 * MATH_PI / 3.0)));
            unsigned char b = (unsigned char)(255.0 * math_abs(qsin(dv * MATH_PI + 4.0 * MATH_PI / 3.0)));
            unsigned int color = 0xFF000000 | ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;

            putpixel(x, y, color);

        }

    }

}

static void render_scene2(unsigned int frame, unsigned int localframe)
{

    r = vector3_add_vector3(&r, &dr);
    r.x = constrain(r.x);
    r.y = constrain(r.y);
    r.z = constrain(r.z);
    size = constrain(size + 0.04);

    buffer_copy(nodes, nodeslocal, sizeof (struct vector3) * 8);
    rotate(nodes, 8, &r);
    translate(nodes, 8, 0, 0, 1.0 + math_sin(size) * 1.2);

    clearscreen(bcolor);

    if (localframe >= 60 * 0)
    {

        projectnode2(&nodes[0], ccolor);
        projectnode2(&nodes[1], ccolor);
        projectnode2(&nodes[2], ccolor);
        projectnode2(&nodes[3], ccolor);
        projectnode2(&nodes[4], ccolor);
        projectnode2(&nodes[5], ccolor);
        projectnode2(&nodes[6], ccolor);
        projectnode2(&nodes[7], ccolor);

    }

    if (localframe >= 60 * 0)
    {

        projectedge(&nodes[0], &nodes[1], ecolor);
        projectedge(&nodes[1], &nodes[3], ecolor);
        projectedge(&nodes[3], &nodes[2], ecolor);
        projectedge(&nodes[2], &nodes[0], ecolor);
        projectedge(&nodes[4], &nodes[5], ecolor);
        projectedge(&nodes[5], &nodes[7], ecolor);
        projectedge(&nodes[7], &nodes[6], ecolor);
        projectedge(&nodes[6], &nodes[4], ecolor);
        projectedge(&nodes[0], &nodes[4], ecolor);
        projectedge(&nodes[1], &nodes[5], ecolor);
        projectedge(&nodes[2], &nodes[6], ecolor);
        projectedge(&nodes[3], &nodes[7], ecolor);

    }

    if (localframe >= 60 * 0)
    {

        projectnode(&nodes[0], ncolor);
        projectnode(&nodes[1], ncolor);
        projectnode(&nodes[2], ncolor);
        projectnode(&nodes[3], ncolor);
        projectnode(&nodes[4], ncolor);
        projectnode(&nodes[5], ncolor);
        projectnode(&nodes[6], ncolor);
        projectnode(&nodes[7], ncolor);

    }

}

struct scene
{

    unsigned int used;
    unsigned int framestart;
    unsigned int framestop;
    void (*setup)(void);
    void (*render)(unsigned int frame, unsigned int localframe);

};

static struct scene scenelist[] = {
    {1, 0, 10, setup_scene1, render_scene1},
    {1, 10, 6000, setup_scene2, render_scene2}
};

static void setup(void)
{

    unsigned int i;

    for (i = 0; i < 2; i++)
    {

        struct scene *scene = &scenelist[i];

        if (!scene->used)
            continue;

        scene->setup();

    }

}

static void render(unsigned int frame)
{

    unsigned int i;

    for (i = 0; i < 2; i++)
    {

        struct scene *scene = &scenelist[i];

        if (!scene->used)
            continue;

        if (frame >= scene->framestart && frame < scene->framestop)
            scene->render(frame, frame - scene->framestart);

    }

}

static void run(void)
{

    struct message message;
    unsigned int frame = 0;

    setup();

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

    channel_send(EVENT_WMUNMAP);
    channel_close();

}

static void onvideomode(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;

    framebuffer = videomode->framebuffer;
    wmax = videomode->w;
    hmax = videomode->h;
    wmid = wmax / 2;
    hmid = hmax / 2;

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
    channel_send(EVENT_WMUNGRAB);

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

