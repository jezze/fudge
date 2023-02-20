#include <fudge.h>
#include <math.h>
#include <abi.h>
#include "../types.h"
#include "../vector2.h"
#include "../vector3.h"
#include "../matrix3x3.h"

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

static struct vector3 projection;
static struct vector3 localvertices[8];
static struct vector3 globalvertices[8];
static struct model cube;
static double zmotion;

extern unsigned int *framebuffer;
extern unsigned int wmax, hmax, wmid, hmid;

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

static void putline(int x0, int y0, int x1, int y1, unsigned int color)
{

    int dx = math_abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -math_abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    for (;;)
    {

        int e2;

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

#if 0
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
#endif

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

void cube_render(struct scene *scene, unsigned int frame, unsigned int loopframe, unsigned int globalframe)
{

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
    clearscreen(0xFF001020);

    if (inperiod(frame, 120, scene->duration - 120))
    {

        unsigned int i;

        for (i = 0; i < cube.vcount; i++)
        {

            struct vector3 *v = &cube.gvertices[i];

            putcircle(v->x * scale + offx, v->y * scale + offy, math_abs(v->z) * scale / 2, 0xFFFF00FF);

        }


    }

    if (inperiod(frame, 360, scene->duration - 360))
    {

        unsigned int polygon0[] = {0, 1, 2, 3};
        unsigned int polygon1[] = {4, 0, 3, 7};
        unsigned int polygon2[] = {5, 4, 7, 6};
        unsigned int polygon3[] = {1, 5, 6, 2};

        putpolygon(polygon0, 4, cube.gvertices, 0xFFFFFF00, scale, offx, offy);
        putpolygon(polygon1, 4, cube.gvertices, 0xFFFFFF00, scale, offx, offy);
        putpolygon(polygon2, 4, cube.gvertices, 0xFFFFFF00, scale, offx, offy);
        putpolygon(polygon3, 4, cube.gvertices, 0xFFFFFF00, scale, offx, offy);

    }

    if (inperiod(frame, 240, scene->duration - 240))
    {

        unsigned int i;

        for (i = 0; i < cube.vcount; i++)
        {

            struct vector3 *v = &cube.gvertices[i];

            putcircle(v->x * scale + offx, v->y * scale + offy, math_abs(v->z) * scale / 8, 0xFFFFFF00);

        }

    }

}

void cube_setup(struct scene *scene)
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

