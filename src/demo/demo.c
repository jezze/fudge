#include <fudge.h>
#include <math.h>
#include <abi.h>
#include "types.h"
#include "vector2.h"
#include "vector3.h"
#include "matrix3x3.h"

struct model
{

    struct vector3 *lvertices;
    struct vector3 *gvertices;
    unsigned int vcount;
    struct vector3 rotation;
    struct vector3 drotation;
    struct vector3 scale;
    struct vector3 dscale;
    struct vector3 translation;
    struct vector3 dtranslation;

};

struct scene
{

    unsigned int used;
    unsigned int framestart;
    unsigned int framestop;
    unsigned int duration;
    void (*setup)(struct scene *scene);
    void (*render)(struct scene *scene, unsigned int frame, unsigned int loopframe, unsigned int globalframe);

};

struct color
{

    double r;
    double g;
    double b;

};

static double table[256];
static struct vector3 projection;
static struct vector3 localvertices[8];
static struct vector3 globalvertices[8];
static struct model cube;
static double zmotion;
static unsigned int *framebuffer;
static unsigned int wmax, hmax, wmid, hmid;
static struct color palette[256];

static void model_init(struct model *model)
{

    model->lvertices = 0;
    model->gvertices = 0;
    model->vcount = 0;
    model->rotation = vector3_zero();
    model->drotation = vector3_zero();
    model->scale = vector3_create(1.0, 1.0, 1.0);
    model->dscale = vector3_zero();
    model->translation = vector3_zero();
    model->dtranslation = vector3_zero();

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

static void model_project(struct model *model)
{

    unsigned int i;

    for (i = 0; i < model->vcount; i++)
    {

        struct vector3 *v = &model->gvertices[i];
        double z = (projection.z / (projection.z + v->z));
        double x = (v->x + projection.x) * z;
        double y = (v->y + projection.y) * z;

        v->x = x;
        v->y = y;
        v->z = z;

    }

}

static void model_transform(struct model *model)
{

    model_prepare(model);
    model_rotatexyz(model, &model->rotation);
    model_translate(model, &model->translation);
    model_project(model);

}

static double wrapradian(double x)
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

static void putpixel(unsigned int x, unsigned int y, unsigned int color)
{

    if (framebuffer && x < wmax && y < hmax)
    {

        unsigned int offset = y * wmax + x;

        framebuffer[offset] = color;

    }

}

static void putpixel2(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{

    if (framebuffer && x < wmax && y < hmax)
    {

        unsigned int color = 0xFF000000 | (r << 16) | (g << 8) | b;
        unsigned int offset = y * wmax + x;

        framebuffer[offset] = color;

    }

}

static void putline(int x0, int y0, int x1, int y1, unsigned int color)
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

/*
static void putsquare(int x, int y, int w, int h, unsigned int color)
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
*/

static void putcircle(int xm, int ym, int r, unsigned int color)
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

static void putpolygon(unsigned int *polygon, unsigned int psize, struct vector3 *vs, unsigned int color, double scale, double offx, double offy)
{

    unsigned int i;

    for (i = 0; i < psize - 1; i++)
    {

        struct vector3 *v1 = &vs[polygon[i]];
        struct vector3 *v2 = &vs[polygon[i + 1]];

        putline(v1->x * scale + offx, v1->y * scale + offy, v2->x * scale + offx, v2->y * scale + offy, color);

    }

}

static double sine(double x)
{

    int c = math_mod(x, 256);

    return (c < 0) ? table[-c] : table[c];

}

static void makepalette(struct color *colors, unsigned int ncolors)
{

    double r[32];
    double g[32];
    double b[32];
    unsigned int max = 255 / ncolors;
    unsigned int i;

    for (i = 0; i < ncolors; i++)
    {

        unsigned int j = ((i + 1 == ncolors) ? 0 : i + 1);

        r[i] = ((colors[i].r - colors[j].r) / max);
        g[i] = ((colors[i].g - colors[j].g) / max);
        b[i] = ((colors[i].b - colors[j].b) / max);

    }

    for (i = 0; i <= max; i++)
    {

        unsigned int j;

        for (j = 0; j < ncolors; j++)
        {

            struct color *p = &palette[j * (max + 1) + i];

            p->r = colors[j].r - r[j] * i;
            p->g = colors[j].g - g[j] * i;
            p->b = colors[j].b - b[j] * i;

        }

    }

}

static unsigned int inperiod(unsigned int frame, unsigned int start, unsigned int end)
{

    return (frame >= start && frame < end);

}

static void setup_scene1(struct scene *scene)
{

    struct color colors[4];
    unsigned int i;

    for (i = 0; i < 256; i++)
        table[i] = math_ceil((((math_sin(i * 2.0 * MATH_PI / 255.0) * 255.0) + 255.0) / 2.0));

    colors[0].r = 255;
    colors[0].g = 0;
    colors[0].b = 0;
    colors[1].r = 0;
    colors[1].g = 255;
    colors[1].b = 0;
    colors[2].r = 0;
    colors[2].g = 0;
    colors[2].b = 255;
    colors[3].r = 0;
    colors[3].g = 255;
    colors[3].b = 255;

    makepalette(colors, 4);

}

static void setup_scene2(struct scene *scene)
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

static void render_plasma_plain(double period, double phase)
{

    unsigned int x;
    unsigned int y;

    for (y = 0; y < hmax; y++)
    {

        for (x = 0; x < wmax; x++)
        {

            double dx = 0.0 * (double)(x);
            double dy = 1.0 * (double)(y);
            double value = sine((dx + dy) * period + phase);
            unsigned char i = (252 * value / 255);
            struct color *p = &palette[i];

            putpixel2(x, y, p->r, p->g, p->b);

        }

    }

}

static void render_plasma_square(double period, double phase)
{

    unsigned int x;
    unsigned int y;

    for (y = 0; y < hmax; y++)
    {

        for (x = 0; x < wmax; x++)
        {

            double dx = math_abs((double)x - wmid);
            double dy = math_abs((double)y - hmid);
            double value = sine(math_max(dx, dy) * period + phase);
            unsigned char i = (252 * value / 255);
            struct color *p = &palette[i];

            putpixel2(x, y, p->r, p->g, p->b);

        }

    }

}

static void render_plasma_radial(double period, double phase)
{

    unsigned int x;
    unsigned int y;

    for (y = 0; y < hmax; y++)
    {

        for (x = 0; x < wmax; x++)
        {

            double dx = (double)x - wmid;
            double dy = (double)y - hmid;
            double value = sine(math_sqrt(dx * dx + dy * dy) * period + phase);
            unsigned char i = (252 * value / 255);
            struct color *p = &palette[i];

            putpixel2(x, y, p->r, p->g, p->b);

        }

    }

}

static void render_plasma_mixed(double period, double phase)
{

    unsigned int x;
    unsigned int y;

    for (y = 0; y < hmax; y++)
    {

        for (x = 0; x < wmax; x++)
        {

            double dx = math_abs((double)x - wmid);
            double dy = math_abs((double)y - hmid);
            double value1 = sine((dx + dy) * period + phase) / 2;
            double value2 = sine(math_max(dx, dy) * period + phase) / 2;
            double value = value1 + value2;
            unsigned char i = (252 * value / 255);
            struct color *p = &palette[i];

            putpixel2(x, y, p->r, p->g, p->b);

        }

    }

}

static void render_plasma_weird(double period, double phase)
{

    unsigned int x;
    unsigned int y;

    for (y = 0; y < hmax; y++)
    {

        for (x = 0; x < wmax; x++)
        {

            double dx = (double)x - wmid;
            double dy = (double)y - hmid;
            double value = sine(dx * dx + dy * dy) * period + phase;
            unsigned char i = (252 * value / 255);
            struct color *p = &palette[i];

            putpixel2(x, y, p->r, p->g, p->b);

        }

    }

}

static void render_scene1(struct scene *scene, unsigned int frame, unsigned int loopframe, unsigned int globalframe)
{

    double period = 0.4;
    double phase = frame * 2.0;

    if (inperiod(frame, 0, 120))
        render_plasma_plain(period, phase);

    if (inperiod(frame, 120, 240))
        render_plasma_square(period, phase);

    if (inperiod(frame, 240, 240))
        render_plasma_radial(period, phase); /* not used */

    if (inperiod(frame, 240, 360))
        render_plasma_mixed(period, phase);

    if (inperiod(frame, 360, 600))
        render_plasma_weird(period, phase);

}

static void render_scene2(struct scene *scene, unsigned int frame, unsigned int loopframe, unsigned int globalframe)
{

    unsigned int bcolor = 0xFF001020;
    unsigned int ncolor = 0xFFFFFF00;
    unsigned int ecolor = 0xFFFFFF00;
    unsigned int ccolor = 0xFFFF00FF;
    double scale = hmid;
    double offx = wmid;
    double offy = hmid;

    zmotion = wrapradian(zmotion + MATH_PI / 60);
    cube.rotation = vector3_add_vector3(&cube.rotation, &cube.drotation);
    cube.rotation.x = wrapradian(cube.rotation.x);
    cube.rotation.y = wrapradian(cube.rotation.y);
    cube.rotation.z = wrapradian(cube.rotation.z);
    cube.translation.z = 1.5 + math_sin(zmotion) * 1.5;

    model_transform(&cube);
    clearscreen(bcolor);

    if (inperiod(frame, 0, scene->duration))
    {

        unsigned int i;

        for (i = 0; i < cube.vcount; i++)
        {

            struct vector3 *v = &cube.gvertices[i];

            putcircle(v->x * scale + offx, v->y * scale + offy, math_abs(v->z) * scale / 2, ccolor);

        }


    }

    if (inperiod(frame, 240, scene->duration))
    {

        unsigned int polygon0[] = {0, 1, 2, 3};
        unsigned int polygon1[] = {4, 0, 3, 7};
        unsigned int polygon2[] = {5, 4, 7, 6};
        unsigned int polygon3[] = {1, 5, 6, 2};

        putpolygon(polygon0, 4, cube.gvertices, ecolor, scale, offx, offy);
        putpolygon(polygon1, 4, cube.gvertices, ecolor, scale, offx, offy);
        putpolygon(polygon2, 4, cube.gvertices, ecolor, scale, offx, offy);
        putpolygon(polygon3, 4, cube.gvertices, ecolor, scale, offx, offy);

    }

    if (inperiod(frame, 120, scene->duration))
    {

        unsigned int i;

        for (i = 0; i < cube.vcount; i++)
        {

            struct vector3 *v = &cube.gvertices[i];

            putcircle(v->x * scale + offx, v->y * scale + offy, math_abs(v->z) * scale / 8, ncolor);

        }

    }

}

static struct scene scenelist[] = {
    {1, 0, 0, 600, setup_scene1, render_scene1},
    {1, 0, 0, 600, setup_scene2, render_scene2}
};
static unsigned int totaltime;
static unsigned int loopframe;

static void setup(void)
{

    unsigned int i;

    for (i = 0; i < 2; i++)
    {

        struct scene *scene = &scenelist[i];

        if (!scene->used)
            continue;

        scene->framestart = totaltime;
        scene->framestop = scene->framestart + scene->duration;
        scene->setup(scene);

        totaltime += scene->duration;

    }

}

static void render(unsigned int frame)
{

    unsigned int i;

    loopframe++;

    if (loopframe > totaltime)
        loopframe = 0;

    for (i = 0; i < 2; i++)
    {

        struct scene *scene = &scenelist[i];

        if (!scene->used)
            continue;

        if (loopframe >= scene->framestart && loopframe < scene->framestop)
            scene->render(scene, loopframe - scene->framestart, loopframe, frame);

    }

}

static void run(void)
{

    struct message_header header;
    char data[MESSAGE_SIZE];
    unsigned int frame = 0;

    setup();

    while (channel_pick(&header, MESSAGE_SIZE, data))
    {

        switch (header.event)
        {

        case EVENT_TIMERTICK:
            render(frame++);

            break;

        case EVENT_KEYPRESS:
            break;

        case EVENT_KEYRELEASE:
            break;

        default:
            channel_dispatch(&header, data);

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

