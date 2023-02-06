#include <fudge.h>
#include <math.h>
#include <abi.h>
#include "../types.h"

struct color
{

    double r;
    double g;
    double b;

};

static double table[256];
static struct color palette[256];
extern unsigned int *framebuffer;
extern unsigned int wmax, hmax, wmid, hmid;

static void putpixel(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{

    if (framebuffer && x < wmax && y < hmax)
    {

        unsigned int color = 0xFF000000 | (r << 16) | (g << 8) | b;
        unsigned int offset = y * wmax + x;

        framebuffer[offset] = color;

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

            putpixel(x, y, p->r, p->g, p->b);

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

            putpixel(x, y, p->r, p->g, p->b);

        }

    }

}

#if 0
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

            putpixel(x, y, p->r, p->g, p->b);

        }

    }

}
#endif

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

            putpixel(x, y, p->r, p->g, p->b);

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

            putpixel(x, y, p->r, p->g, p->b);

        }

    }

}

void plasma_render(struct scene *scene, unsigned int frame, unsigned int loopframe, unsigned int globalframe)
{

    double period = 0.4;
    double phase = frame * 2.0;

    if (inperiod(frame, 0, 60))
        render_plasma_plain(period, phase);

    if (inperiod(frame, 60, 120))
        render_plasma_square(period, phase);

#if 0
    if (inperiod(frame, 120, 180))
        render_plasma_radial(period, phase);
#endif

    if (inperiod(frame, 120, 180))
        render_plasma_mixed(period, phase);

    if (inperiod(frame, 180, 240))
        render_plasma_weird(period, phase);

}

void plasma_setup(struct scene *scene)
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

