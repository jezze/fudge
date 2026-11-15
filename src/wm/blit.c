#include <fudge.h>
#include <abi.h>
#include <image/toffee.h>
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"
#include "pool.h"
#include "cmap.h"
#include "blit.h"

#define REL0                            1
#define REL1                            2
#define REL2                            3
#define CMAP_SHADOW                     0
#define CMAP_NORMAL                     1
#define CMAP_DARK                       2
#define CMAP_LIGHT                      3
#define CMAP_ICON_COLOR                 0
#define CMAP_RECT_COLOR                 0
#define CMAP_TEXT_COLOR                 0
#define MOUSE_LINEHEIGHT                12

struct linesegment
{

    unsigned int t0;
    unsigned int t1;
    int p0;
    int p1;
    unsigned int color;

};

struct rowsegment
{

    unsigned int t0;
    unsigned int t1;
    int p0;
    int p1;
    struct linesegment *lines;
    unsigned int numlines;

};

struct mouse
{

    struct toffee toffee;
    int hotx;
    int hoty;

};

static struct mouse mice[BLIT_MOUSES];
static unsigned int mouselineheight = MOUSE_LINEHEIGHT;

static int getpoint(unsigned int type, int p, int o, int m)
{

    switch (type)
    {

    case REL0:
        return o + p;

    case REL1:
        return o + p + m / 2;

    case REL2:
        return o + p + m;

    }

    return p;

}

static struct rowsegment *findrowsegment(struct rowsegment *rows, unsigned int length, int line, int y, int h)
{

    unsigned int i;

    for (i = 0; i < length; i++)
    {

        struct rowsegment *current = &rows[i];

        if (util_intersects(line, getpoint(current->t0, current->p0, y, h), getpoint(current->t1, current->p1, y, h)))
            return current;

    }

    return 0;

}

static void blitsegment(struct blit_display *display, struct rowsegment *rows, unsigned int count, unsigned int line, struct util_region *region, int x0, int x2, unsigned int *cmap)
{

    struct rowsegment *rs = findrowsegment(rows, count, line, region->position.y, region->size.h);

    if (rs)
    {

        unsigned int i;

        for (i = 0; i < rs->numlines; i++)
        {

            struct linesegment *current = &rs->lines[i];

            blit_alphaline(display, cmap[current->color], util_max(getpoint(current->t0, current->p0, region->position.x, region->size.w), x0), util_min(getpoint(current->t1, current->p1, region->position.x, region->size.w), x2));

        }

    }

}

static void blitbitmap(struct blit_display *display, char **rows, unsigned int count, int bx, int by, int line, int x0, int x2, unsigned int *cmap)
{

    if (line >= by && line < by + (int)count)
    {

        int i;

        for (i = 0; rows[line - by][i] && bx + i < x2; i++)
        {

            int x = bx + i;

            if (x < x0)
                continue;

            switch (rows[line - by][i])
            {

            case '#':
                blit_alphaline(display, cmap[CMAP_ICON_COLOR], x, x + 1);

                break;

            case 'S':
                blit_alphaline(display, cmap[CMAP_SHADOW], x, x + 1);

                break;

            case 'N':
                blit_alphaline(display, cmap[CMAP_NORMAL], x, x + 1);

                break;

            case 'D':
                blit_alphaline(display, cmap[CMAP_DARK], x, x + 1);

                break;

            case 'L':
                blit_alphaline(display, cmap[CMAP_LIGHT], x, x + 1);

                break;

            }

        }

    }

}

static void blittoffee(struct blit_display *display, struct toffee *toffee, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    if (line >= region->position.y && line < region->position.y + (int)region->size.h)
    {

        struct toffee_line scan;
        unsigned int i;

        for (i = 0; i < toffee->nshapes; i++)
        {

            struct toffee_shape *shape = &toffee->shapes[i];
            unsigned int color = (shape->slot < NUM_PARTS) ? cmap[shape->slot] : shape->color;
            unsigned int j;

            toffee_scanline(toffee, &scan, i, region->size.w, region->size.h, line - region->position.y);

            for (j = 0; j < scan.count; j++)
                blit_alphaline(display, color, util_max(region->position.x + scan.spans[j].x0, x0), util_min(region->position.x + scan.spans[j].x2, x2));

        }

    }

}

void blit_line(struct blit_display *display, unsigned int color, int x0, int x2)
{

    int x;

    for (x = x0; x < x2; x++)
        display->linebuffer[x - display->region.position.x] = color;

}

void blit_alphaline(struct blit_display *display, unsigned int color, int x0, int x2)
{

    unsigned int alpha = (color >> 24) + 1;
    unsigned int ialpha = 256 - (color >> 24);
    unsigned int r = alpha * ((color >> 16) & 0xFF);
    unsigned int g = alpha * ((color >> 8) & 0xFF);
    unsigned int b = alpha * (color & 0xFF);

    if (color >> 24)
    {

        int x;

        if (display->opaque || (color >> 24) == 0xFF)
        {

            blit_line(display, 0xFF000000 | color, x0, x2);

            return;

        }

        for (x = x0; x < x2; x++)
        {

            unsigned int bg = display->linebuffer[x - display->region.position.x];

            display->linebuffer[x - display->region.position.x] = 0xFF000000 | (((r + ialpha * ((bg >> 16) & 0xFF)) >> 8) << 16) | (((g + ialpha * ((bg >> 8) & 0xFF)) >> 8) << 8) | ((b + ialpha * (bg & 0xFF)) >> 8);

        }

    }

}

static void blitpcfbitmap(struct blit_display *display, int rx, int x0, int x2, unsigned int color, unsigned char *data, unsigned int width)
{

    int r0 = util_max(0, x0 - rx);
    int r1 = util_min(x2 - rx, width);
    unsigned int r;

    for (r = r0; r < r1; r++)
    {

        if ((data[(r >> 3)] & (0x80 >> (r % 8))))
            blit_alphaline(display, color, rx + r, rx + r + 1);

    }

}

static void blitpcfbitmapinverted(struct blit_display *display, int rx, int x0, int x2, unsigned int color, unsigned char *data, unsigned int width)
{

    int r0 = util_max(0, x0 - rx);
    int r1 = util_min(x2 - rx, width);
    unsigned int r;

    for (r = r0; r < r1; r++)
    {

        if (!(data[(r >> 3)] & (0x80 >> (r % 8))))
            blit_alphaline(display, color, rx + r, rx + r + 1);

    }

}

void blit_text(struct blit_display *display, struct text_font *font, char *text, unsigned int length, int rx, int ry, int line, int x0, int x2, unsigned int ms, unsigned int me, unsigned int *cmap)
{

    unsigned int color = cmap[CMAP_TEXT_COLOR];
    unsigned int size;
    unsigned int i;

    for (i = 0; i < length; i += size)
    {

        unsigned int codepoint;
        struct text_atlas *atlas;
        unsigned int lline;

        size = utf8_read(text + i, length - i, &codepoint);
        atlas = text_getatlas(font, codepoint);
        lline = (line - ry) % font->lineheight - (font->lineheight - atlas->height) / 2;

        if (util_intersects(lline, 0, atlas->height))
        {

            if (util_intersects(rx, x0, x2) || util_intersects(rx + atlas->width - 1, x0, x2))
            {

                if ((i >= ms && i < me))
                    blitpcfbitmapinverted(display, rx, x0, x2, color, atlas->bdata + lline * atlas->width, atlas->width);
                else
                    blitpcfbitmap(display, rx, x0, x2, color, atlas->bdata + lline * atlas->width, atlas->width);

            }

        }

        rx += atlas->width;

    }

}

void blit_iconarrowdown(struct blit_display *display, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    static char *rows[6] = {
        "############",
        " ##########",
        "  ########",
        "   ######",
        "    ####",
        "     ##"
    };

    blitbitmap(display, rows, 6, region->position.x + (int)(region->size.w / 2) - 6, region->position.y + (int)(region->size.h / 2) - 3, line, x0, x2, cmap);

}

void blit_iconcursor(struct blit_display *display, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL0, REL2, 0, 0, CMAP_ICON_COLOR}
    };
    static struct rowsegment rows[1] = {
        {REL0, REL2, 0, 0, line0, 1}
    };

    blitsegment(display, rows, 1, line, region, x0, x2, cmap);

}

void blit_iconhamburger(struct blit_display *display, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    static char *rows[16] = {
        "################",
        "################",
        "################",
        "################",
        "",
        "",
        "################",
        "################",
        "################",
        "################",
        "",
        "",
        "################",
        "################",
        "################",
        "################"
    };

    blitbitmap(display, rows, 16, region->position.x + (int)(region->size.w / 2) - 8, region->position.y + (int)(region->size.h / 2) - 8, line, x0, x2, cmap);

}

void blit_iconminimize(struct blit_display *display, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    static char *rows[4] = {
        "################",
        "################",
        "################",
        "################"
    };

    blitbitmap(display, rows, 4, region->position.x + (int)(region->size.w / 2) - 8, region->position.y + (int)(region->size.h / 2) + 4, line, x0, x2, cmap);

}

void blit_iconx(struct blit_display *display, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    static char *rows[16] = {
        "  #          #",
        " ###        ###",
        "#####      #####",
        " #####    #####",
        "  #####  #####",
        "   ##########",
        "    ########",
        "     ######",
        "     ######",
        "    ########",
        "   ##########",
        "  #####  #####",
        " #####    #####",
        "#####      #####",
        " ###        ###",
        "  #          #"
    };

    blitbitmap(display, rows, 16, region->position.x + (int)(region->size.w / 2) - 8, region->position.y + (int)(region->size.h / 2) - 8, line, x0, x2, cmap);

}

static int scalemouse(int value)
{

    return value * (int)mouselineheight / MOUSE_LINEHEIGHT;

}

struct util_region blit_getmouseregion(unsigned int type, int mx, int my)
{

    struct mouse *mouse = &mice[type];

    return util_region(mx - scalemouse(mouse->hotx), my - scalemouse(mouse->hoty), scalemouse(mouse->toffee.width), scalemouse(mouse->toffee.height));

}

void blit_mouse(struct blit_display *display, unsigned int type, int mx, int my, int line, int x0, int x2, unsigned int *cmap)
{

    struct util_region region = blit_getmouseregion(type, mx, my);

    blittoffee(display, &mice[type].toffee, &region, line, x0, x2, cmap);

}

void blit_frame(struct blit_display *display, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL0, REL2, 0, 0, CMAP_SHADOW}
    };
    static struct linesegment line1[3] = {
        {REL0, REL0, 0, 1, CMAP_SHADOW},
        {REL0, REL2, 1, -1, CMAP_LIGHT},
        {REL2, REL2, -1, 0, CMAP_SHADOW}
    };
    static struct linesegment line2[5] = {
        {REL0, REL0, 0, 1, CMAP_SHADOW},
        {REL0, REL0, 1, 2, CMAP_LIGHT},
        {REL0, REL2, 2, -2, CMAP_DARK},
        {REL2, REL2, -2, -1, CMAP_LIGHT},
        {REL2, REL2, -1, 0, CMAP_SHADOW}
    };
    static struct linesegment line3[7] = {
        {REL0, REL0, 0, 1, CMAP_SHADOW},
        {REL0, REL0, 1, 2, CMAP_LIGHT},
        {REL0, REL0, 2, 3, CMAP_DARK},
        {REL0, REL2, 3, -3, CMAP_NORMAL},
        {REL2, REL2, -3, -2, CMAP_DARK},
        {REL2, REL2, -2, -1, CMAP_LIGHT},
        {REL2, REL2, -1, 0, CMAP_SHADOW}
    };
    static struct rowsegment rows[7] = {
        {REL0, REL0, 0, 1, line0, 1},
        {REL0, REL0, 1, 2, line1, 3},
        {REL0, REL0, 2, 3, line2, 5},
        {REL0, REL2, 3, -3, line3, 7},
        {REL2, REL2, -3, -2, line2, 5},
        {REL2, REL2, -2, -1, line1, 3},
        {REL2, REL2, -1, 0, line0, 1}
    };

    blitsegment(display, rows, 7, line, region, x0, x2, cmap);

}

void blit_raw(struct blit_display *display, unsigned int *data, unsigned int width, unsigned int height, int line, int x, int y, unsigned int w, unsigned int h, int x0, int x2)
{

    int start = util_max(x0, x);
    int end = util_min(x2, x + w);

    if (data && w && h && util_intersects(line, y, y + h) && start < end)
    {

        unsigned int *row;
        unsigned int source;
        unsigned int remainder;
        int i;

        row = data + (line - y) * height / h * width;
        source = (start - x) * width / w;
        remainder = (start - x) * width % w;

        for (i = start; i < end; i++)
        {

            blit_alphaline(display, row[source], i, i + 1);

            remainder += width;

            while (remainder >= w)
            {

                remainder -= w;
                source++;

            }

        }

    }

}

void blit_loadmouse(unsigned int type, int hotx, int hoty, char *data, unsigned int count)
{

    struct mouse *mouse = &mice[type];

    toffee_parse(&mouse->toffee, data, count);

    mouse->hotx = hotx;
    mouse->hoty = hoty;

}

void blit_setmousescale(unsigned int lineheight)
{

    mouselineheight = lineheight;

}

void blit_initdisplay(struct blit_display *display, void *framebuffer, int x, int y, unsigned int w, unsigned int h, unsigned int bpp, unsigned int *linebuffer)
{

    display->framebuffer = framebuffer;
    display->bpp = bpp;
    display->linebuffer = linebuffer;
    display->region = util_region(x, y, w, h);

}

void blit(struct blit_display *display, int line, int x0, int x2)
{

    line -= display->region.position.y;
    x0 -= display->region.position.x;
    x2 -= display->region.position.x;

    if (display->bpp == 4)
    {

        buffer_copy((unsigned int *)display->framebuffer + (line * display->region.size.w) + x0, display->linebuffer + x0, (x2 - x0) * display->bpp);

    }

    else
    {

        unsigned char *pixel = (unsigned char *)display->framebuffer + (line * display->region.size.w + x0) * display->bpp;
        int x;

        for (x = x0; x < x2; x++)
        {

            unsigned int color = display->linebuffer[x];

            pixel[0] = color;
            pixel[1] = color >> 8;
            pixel[2] = color >> 16;
            pixel += display->bpp;

        }

    }

}

