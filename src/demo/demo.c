#include <fudge.h>
#include <math.h>
#include <abi.h>
#include "types.h"
#include "vector2.h"
#include "vector3.h"
#include "matrix3x3.h"

static unsigned int *framebuffer;
static unsigned int wmax, hmax, wmid, hmid;

double wrapradian(double x)
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

struct model
{

    struct vector3 *lvertices;
    struct vector3 *gvertices;
    unsigned int vcount;
    struct vector3 rotation;
    struct vector3 drotation;
    struct vector3 scale;
    struct vector3 dscale;
    struct vector3 translate;
    struct vector3 dtranslate;

};

static struct vector3 projection;
static struct vector3 localvertices[8];
static struct vector3 globalvertices[8];
static struct model cube;
static unsigned int bcolor = 0xFF001020;
static unsigned int ncolor = 0xFFFFFF00;
static unsigned int ecolor = 0xFFFFFF00;
static unsigned int ccolor = 0xFFFF00FF;

static void model_init(struct model *model)
{

    model->lvertices = 0;
    model->gvertices = 0;
    model->vcount = 0;
    model->rotation = vector3_zero();
    model->drotation = vector3_zero();
    model->scale = vector3_create(1.0, 1.0, 1.0);
    model->dscale = vector3_zero();
    model->translate = vector3_zero();
    model->dtranslate = vector3_zero();

}

static void model_load(struct model *model, struct vector3 *lvertices, struct vector3 *gvertices, unsigned int vcount)
{

    model->lvertices = lvertices;
    model->gvertices = gvertices;
    model->vcount = vcount;

}

static void model_prepare(struct model *model)
{

    unsigned int i;

    for (i = 0; i < model->vcount; i++)
        model->gvertices[i] = model->lvertices[i];

}

static void model_translate(struct model *model, struct vector3 *v)
{

    unsigned int i;

    for (i = 0; i < model->vcount; i++)
        model->gvertices[i] = vector3_add_vector3(&model->gvertices[i], v);

}

static void model_rotatexyz(struct model *model, struct vector3 *r)
{

    struct matrix3x3 mx = matrix3x3_getrotationx(math_sin(r->x), math_cos(r->x));
    struct matrix3x3 my = matrix3x3_getrotationy(math_sin(r->y), math_cos(r->y));
    struct matrix3x3 mz = matrix3x3_getrotationz(math_sin(r->z), math_cos(r->z));
    unsigned int i;

    for (i = 0; i < model->vcount; i++)
    {

        model->gvertices[i] = vector3_mul_matrix3x3(&model->gvertices[i], &mx);
        model->gvertices[i] = vector3_mul_matrix3x3(&model->gvertices[i], &my);
        model->gvertices[i] = vector3_mul_matrix3x3(&model->gvertices[i], &mz);

    }

}

static void projectnode(struct vector3 *v, unsigned int color)
{

    double scale = hmid;
    double z = (projection.z / (projection.z + v->z));
    double x = (v->x + projection.x) * z;
    double y = (v->y + projection.y) * z;

    putcircle(x * scale + wmid, y * scale + hmid, 8, color);

}

static void projectnode2(struct vector3 *v, unsigned int color)
{

    double scale = hmid;
    double z = (projection.z / (projection.z + v->z));
    double x = (v->x + projection.x) * z;
    double y = (v->y + projection.y) * z;

    putcircle(x * scale + wmid, y * scale + hmid, math_abs(v->z) * scale / 10, color);

}

static void projectedge(struct vector3 *v1, struct vector3 *v2, unsigned int color)
{

    double scale = hmid;
    double z1 = (projection.z / (projection.z + v1->z));
    double x1 = (v1->x + projection.x) * z1;
    double y1 = (v1->y + projection.y) * z1;
    double z2 = (projection.z / (projection.z + v2->z));
    double x2 = (v2->x + projection.x) * z2;
    double y2 = (v2->y + projection.y) * z2;

    putline(x1 * scale + wmid, y1 * scale + hmid, x2 * scale + wmid, y2 * hmid + hmid, color);

}

static void setup_scene1(void)
{

}

static void setup_scene2(void)
{

    projection = vector3_create(0.0, 0.0, 1.0);
    localvertices[0] = vector3_create(-0.5, -0.5, -0.5);
    localvertices[1] = vector3_create(0.5, -0.5, -0.5);
    localvertices[2] = vector3_create(0.5, 0.5, -0.5);
    localvertices[3] = vector3_create(-0.5, 0.5, -0.5);
    localvertices[4] = vector3_create(-0.5, -0.5, 0.5);
    localvertices[5] = vector3_create(0.5, -0.5, 0.5);
    localvertices[6] = vector3_create(0.5, 0.5, 0.5);
    localvertices[7] = vector3_create(-0.5, 0.5, 0.5);

    model_init(&cube);
    model_load(&cube, localvertices, globalvertices, 8);

    cube.drotation.y = MATH_PI / 60;

}

static double qsin(double x)
{

    return math_sin(wrapradian(x));

}

/*
static double qcos(double x)
{

    return math_cos(wrapradian(x));

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

    cube.rotation = vector3_add_vector3(&cube.rotation, &cube.drotation);
    cube.rotation.x = wrapradian(cube.rotation.x);
    cube.rotation.y = wrapradian(cube.rotation.y);
    cube.rotation.z = wrapradian(cube.rotation.z);
    cube.dtranslate.z = wrapradian(cube.dtranslate.z + 0.04);
    cube.translate.z = 1.5 + math_sin(cube.dtranslate.z) * 1.5;

    model_prepare(&cube);
    model_rotatexyz(&cube, &cube.rotation);
    model_translate(&cube, &cube.translate);
    clearscreen(bcolor);

    if (localframe >= 60 * 0)
    {

        unsigned int i;

        for (i = 0; i < cube.vcount; i++)
            projectnode2(&cube.gvertices[i], ccolor);

    }

    if (localframe >= 60 * 0)
    {

        unsigned int polygon0[] = {0, 1, 2, 3};
        unsigned int polygon1[] = {4, 0, 3, 7};
        unsigned int polygon2[] = {5, 4, 7, 6};
        unsigned int polygon3[] = {1, 5, 6, 2};
        unsigned int i;

        for (i = 0; i < 3; i++)
        {

            projectedge(&cube.gvertices[polygon0[i]], &cube.gvertices[polygon0[i + 1]], ecolor);
            projectedge(&cube.gvertices[polygon1[i]], &cube.gvertices[polygon1[i + 1]], ecolor);
            projectedge(&cube.gvertices[polygon2[i]], &cube.gvertices[polygon2[i + 1]], ecolor);
            projectedge(&cube.gvertices[polygon3[i]], &cube.gvertices[polygon3[i + 1]], ecolor);

        }

    }

    if (localframe >= 60 * 0)
    {

        unsigned int i;

        for (i = 0; i < cube.vcount; i++)
            projectnode(&cube.gvertices[i], ncolor);

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

