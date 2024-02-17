#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"
#include "pool.h"
#include "blit.h"

#define REL0                            1
#define REL1                            2
#define REL2                            3
#define CMAP_FRAME_SHADOW               0
#define CMAP_FRAME_NORMAL               1
#define CMAP_FRAME_DARK                 2
#define CMAP_FRAME_LIGHT                3
#define CMAP_MOUSE_SHADOW               0
#define CMAP_MOUSE_NORMAL               1
#define CMAP_MOUSE_LIGHT                2
#define CMAP_ICON_COLOR                 0
#define CMAP_RECT_COLOR                 0
#define CMAP_TEXT_COLOR                 0

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

static struct pool_pcxresource pcxresource;

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

static void blitsegment(struct blit_display *display, struct rowsegment *rows, unsigned int count, unsigned int line, int x, int y, int w, int h, int x0, int x2, unsigned int *cmap)
{

    struct rowsegment *rs = findrowsegment(rows, count, line, y, h);

    if (rs)
    {

        unsigned int i;

        for (i = 0; i < rs->numlines; i++)
        {

            struct linesegment *current = &rs->lines[i];

            blit_alphaline(display, cmap[current->color], util_max(getpoint(current->t0, current->p0, x, w), x0), util_min(getpoint(current->t1, current->p1, x, w), x2));

        }

    }

}

void blit_line(struct blit_display *display, unsigned int color, int x0, int x2)
{

    int x;

    for (x = x0; x < x2; x++)
        display->linebuffer[x] = color;

}

void blit_alphaline(struct blit_display *display, unsigned int color, int x0, int x2)
{

    unsigned char *fg = (unsigned char *)&color;
    int x;

    for (x = x0; x < x2; x++)
    {

        unsigned char *bg = (unsigned char *)&display->linebuffer[x];
        unsigned int alpha = fg[3] + 1;
        unsigned int ialpha = 256 - fg[3];

        bg[0] = ((alpha * fg[0] + ialpha * bg[0]) >> 8);
        bg[1] = ((alpha * fg[1] + ialpha * bg[1]) >> 8);
        bg[2] = ((alpha * fg[2] + ialpha * bg[2]) >> 8);
        bg[3] = 0xFF;

    }

}

void blitpcfbitmap(struct blit_display *display, int rx, int x0, int x2, unsigned int color, unsigned char *data, unsigned int width)
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

void blitpcfbitmapinverted(struct blit_display *display, int rx, int x0, int x2, unsigned int color, unsigned char *data, unsigned int width)
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
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        unsigned short index = pcf_getindex(font->data, text[i]);
        unsigned int offset = pcf_getbitmapoffset(font->data, index);
        struct pcf_metricsdata metricsdata;
        unsigned int lline;
        unsigned int height;
        unsigned int width;

        pcf_readmetricsdata(font->data, index, &metricsdata);

        height = metricsdata.ascent + metricsdata.descent;
        width = metricsdata.width;
        lline = (line - ry) % font->lineheight - (font->lineheight - height) / 2;

        if (util_intersects(lline, 0, height))
        {

            if (util_intersects(rx, x0, x2) || util_intersects(rx + width - 1, x0, x2))
            {

                if ((i >= ms && i < me))
                    blitpcfbitmapinverted(display, rx, x0, x2, color, font->bitmapdata + offset + lline * font->bitmapalign, width);
                else
                    blitpcfbitmap(display, rx, x0, x2, color, font->bitmapdata + offset + lline * font->bitmapalign, width);

            }

        }

        rx += width;

    }

}

void blit_iconarrowdown(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL1, REL1, -6, 6, CMAP_ICON_COLOR}
    };
    static struct linesegment line1[1] = {
        {REL1, REL1, -5, 5, CMAP_ICON_COLOR}
    };
    static struct linesegment line2[1] = {
        {REL1, REL1, -4, 4, CMAP_ICON_COLOR}
    };
    static struct linesegment line3[1] = {
        {REL1, REL1, -3, 3, CMAP_ICON_COLOR}
    };
    static struct linesegment line4[1] = {
        {REL1, REL1, -2, 2, CMAP_ICON_COLOR}
    };
    static struct linesegment line5[1] = {
        {REL1, REL1, -1, 1, CMAP_ICON_COLOR}
    };
    static struct rowsegment rows[6] = {
        {REL1, REL1, -3, -2, line0, 1},
        {REL1, REL1, -2, -1, line1, 1},
        {REL1, REL1, -1, 0, line2, 1},
        {REL1, REL1, 0, 1, line3, 1},
        {REL1, REL1, 1, 2, line4, 1},
        {REL1, REL1, 2, 3, line5, 1}
    };

    blitsegment(display, rows, 6, line, x, y, w, h, x0, x2, cmap);

}

void blit_iconarrowup(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL1, REL1, -1, 1, CMAP_ICON_COLOR}
    };
    static struct linesegment line1[1] = {
        {REL1, REL1, -2, 2, CMAP_ICON_COLOR}
    };
    static struct linesegment line2[1] = {
        {REL1, REL1, -3, 3, CMAP_ICON_COLOR}
    };
    static struct linesegment line3[1] = {
        {REL1, REL1, -4, 4, CMAP_ICON_COLOR}
    };
    static struct linesegment line4[1] = {
        {REL1, REL1, -5, 5, CMAP_ICON_COLOR}
    };
    static struct linesegment line5[1] = {
        {REL1, REL1, -6, 6, CMAP_ICON_COLOR}
    };
    static struct rowsegment rows[6] = {
        {REL1, REL1, -3, -2, line0, 1},
        {REL1, REL1, -2, -1, line1, 1},
        {REL1, REL1, -1, 0, line2, 1},
        {REL1, REL1, 0, 1, line3, 1},
        {REL1, REL1, 1, 2, line4, 1},
        {REL1, REL1, 2, 3, line5, 1}
    };

    blitsegment(display, rows, 6, line, x, y, w, h, x0, x2, cmap);

}

void blit_iconcursor(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL0, REL2, 0, 0, CMAP_ICON_COLOR}
    };
    static struct rowsegment rows[1] = {
        {REL0, REL2, 0, 0, line0, 1}
    };

    blitsegment(display, rows, 1, line, x, y, w, h, x0, x2, cmap);

}

void blit_icondropdown(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL1, REL1, -1, 1, CMAP_ICON_COLOR}
    };
    static struct linesegment line1[1] = {
        {REL1, REL1, -2, 2, CMAP_ICON_COLOR}
    };
    static struct linesegment line2[1] = {
        {REL1, REL1, -3, 3, CMAP_ICON_COLOR}
    };
    static struct linesegment line3[1] = {
        {REL1, REL1, -4, 4, CMAP_ICON_COLOR}
    };
    static struct linesegment line4[1] = {
        {REL1, REL1, -5, 5, CMAP_ICON_COLOR}
    };
    static struct rowsegment rows[10] = {
        {REL1, REL1, -7, -6, line0, 1},
        {REL1, REL1, -6, -5, line1, 1},
        {REL1, REL1, -5, -4, line2, 1},
        {REL1, REL1, -4, -3, line3, 1},
        {REL1, REL1, -3, -2, line4, 1},
        {REL1, REL1, 2, 3, line4, 1},
        {REL1, REL1, 3, 4, line3, 1},
        {REL1, REL1, 4, 5, line2, 1},
        {REL1, REL1, 5, 6, line1, 1},
        {REL1, REL1, 6, 7, line0, 1}
    };

    blitsegment(display, rows, 10, line, x, y, w, h, x0, x2, cmap);

}

void blit_iconhamburger(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL1, REL1, -8, 8, CMAP_ICON_COLOR}
    };
    static struct rowsegment rows[3] = {
        {REL1, REL1, -8, -4, line0, 1},
        {REL1, REL1, -2, 2, line0, 1},
        {REL1, REL1, 4, 8, line0, 1}
    };

    blitsegment(display, rows, 3, line, x, y, w, h, x0, x2, cmap);

}

void blit_iconminimize(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL1, REL1, -8, 8, CMAP_ICON_COLOR}
    };
    static struct rowsegment rows[1] = {
        {REL1, REL1, 4, 8, line0, 1}
    };

    blitsegment(display, rows, 1, line, x, y, w, h, x0, x2, cmap);

}

void blit_iconx(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL1, REL1, -3, 3, CMAP_ICON_COLOR}
    };
    static struct linesegment line1[1] = {
        {REL1, REL1, -4, 4, CMAP_ICON_COLOR}
    };
    static struct linesegment line2[1] = {
        {REL1, REL1, -5, 5, CMAP_ICON_COLOR}
    };
    static struct linesegment line3[2] = {
        {REL1, REL1, -6, -1, CMAP_ICON_COLOR},
        {REL1, REL1, 1, 6, CMAP_ICON_COLOR}
    };
    static struct linesegment line4[2] = {
        {REL1, REL1, -7, -2, CMAP_ICON_COLOR},
        {REL1, REL1, 2, 7, CMAP_ICON_COLOR}
    };
    static struct linesegment line5[2] = {
        {REL1, REL1, -8, -3, CMAP_ICON_COLOR},
        {REL1, REL1, 3, 8, CMAP_ICON_COLOR}
    };
    static struct linesegment line6[2] = {
        {REL1, REL1, -7, -4, CMAP_ICON_COLOR},
        {REL1, REL1, 4, 7, CMAP_ICON_COLOR}
    };
    static struct linesegment line7[2] = {
        {REL1, REL1, -6, -5, CMAP_ICON_COLOR},
        {REL1, REL1, 5, 6, CMAP_ICON_COLOR}
    };
    static struct rowsegment rows[15] = {
        {REL1, REL1, -8, -7, line7, 2},
        {REL1, REL1, -7, -6, line6, 2},
        {REL1, REL1, -6, -5, line5, 2},
        {REL1, REL1, -5, -4, line4, 2},
        {REL1, REL1, -4, -3, line3, 2},
        {REL1, REL1, -3, -2, line2, 1},
        {REL1, REL1, -2, -1, line1, 1},
        {REL1, REL1, -1, 1, line0, 1},
        {REL1, REL1, 1, 2, line1, 1},
        {REL1, REL1, 2, 3, line2, 1},
        {REL1, REL1, 3, 4, line3, 2},
        {REL1, REL1, 4, 5, line4, 2},
        {REL1, REL1, 5, 6, line5, 2},
        {REL1, REL1, 6, 7, line6, 2},
        {REL1, REL1, 7, 8, line7, 2}
    };

    blitsegment(display, rows, 15, line, x, y, w, h, x0, x2, cmap);

}

void blit_mouse(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL0, REL0, 0, 3, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line1[3] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 4, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line2[3] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 3, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 3, 5, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line3[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 3, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 3, 4, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 4, 6, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line4[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 4, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 4, 5, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 5, 7, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line5[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 5, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 5, 6, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 6, 8, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line6[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 6, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 6, 7, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 7, 9, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line7[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 7, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 7, 8, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 8, 10, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line8[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 8, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 8, 9, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 9, 11, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line9[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 9, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 9, 10, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 10, 12, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line10[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 10, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 10, 11, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 11, 13, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line11[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 11, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 11, 12, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 12, 14, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line12[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 12, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 12, 13, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 13, 15, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line13[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 13, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 13, 14, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 14, 16, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line14[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 14, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 14, 15, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 15, 17, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line15[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 15, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 15, 16, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 16, 18, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line16[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 7, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 7, 17, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 17, 18, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line17[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 6, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 6, 7, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 7, 18, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line18[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 5, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 5, 6, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 6, 8, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line19[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 4, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 4, 5, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 5, 7, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line20[5] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 3, CMAP_MOUSE_NORMAL},
        {REL0, REL0, 3, 4, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 4, 6, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line21[3] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 3, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 3, 5, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line22[3] = {
        {REL0, REL0, 0, 1, CMAP_MOUSE_SHADOW},
        {REL0, REL0, 1, 2, CMAP_MOUSE_LIGHT},
        {REL0, REL0, 2, 4, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line23[1] = {
        {REL0, REL0, 0, 3, CMAP_MOUSE_SHADOW}
    };
    static struct rowsegment rows[24] = {
        {REL0, REL0, 0, 1, line0, 1},
        {REL0, REL0, 1, 2, line1, 3},
        {REL0, REL0, 2, 3, line2, 3},
        {REL0, REL0, 3, 4, line3, 5},
        {REL0, REL0, 4, 5, line4, 5},
        {REL0, REL0, 5, 6, line5, 5},
        {REL0, REL0, 6, 7, line6, 5},
        {REL0, REL0, 7, 8, line7, 5},
        {REL0, REL0, 8, 9, line8, 5},
        {REL0, REL0, 9, 10, line9, 5},
        {REL0, REL0, 10, 11, line10, 5},
        {REL0, REL0, 11, 12, line11, 5},
        {REL0, REL0, 12, 13, line12, 5},
        {REL0, REL0, 13, 14, line13, 5},
        {REL0, REL0, 14, 15, line14, 5},
        {REL0, REL0, 15, 16, line15, 5},
        {REL0, REL0, 16, 17, line16, 5},
        {REL0, REL0, 17, 18, line17, 5},
        {REL0, REL0, 18, 19, line18, 5},
        {REL0, REL0, 19, 20, line19, 5},
        {REL0, REL0, 20, 21, line20, 5},
        {REL0, REL0, 21, 22, line21, 3},
        {REL0, REL0, 22, 23, line22, 3},
        {REL0, REL0, 23, 24, line23, 1}
    };

    blitsegment(display, rows, 24, line, x, y, w, h, x0, x2, cmap);

}

void blit_frame(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL0, REL2, 0, 0, CMAP_FRAME_SHADOW}
    };
    static struct linesegment line1[3] = {
        {REL0, REL0, 0, 2, CMAP_FRAME_SHADOW},
        {REL0, REL2, 2, -2, CMAP_FRAME_LIGHT},
        {REL2, REL2, -2, 0, CMAP_FRAME_SHADOW}
    };
    static struct linesegment line2[5] = {
        {REL0, REL0, 0, 2, CMAP_FRAME_SHADOW},
        {REL0, REL0, 2, 3, CMAP_FRAME_LIGHT},
        {REL0, REL2, 3, -3, CMAP_FRAME_DARK},
        {REL2, REL2, -3, -2, CMAP_FRAME_LIGHT},
        {REL2, REL2, -2, 0, CMAP_FRAME_SHADOW}
    };
    static struct linesegment line3[7] = {
        {REL0, REL0, 0, 2, CMAP_FRAME_SHADOW},
        {REL0, REL0, 2, 3, CMAP_FRAME_LIGHT},
        {REL0, REL0, 3, 5, CMAP_FRAME_DARK},
        {REL0, REL2, 5, -5, CMAP_FRAME_NORMAL},
        {REL2, REL2, -5, -3, CMAP_FRAME_DARK},
        {REL2, REL2, -3, -2, CMAP_FRAME_LIGHT},
        {REL2, REL2, -2, 0, CMAP_FRAME_SHADOW}
    };
    static struct rowsegment rows[7] = {
        {REL0, REL0, 0, 2, line0, 1},
        {REL0, REL0, 2, 3, line1, 3},
        {REL0, REL0, 3, 5, line2, 5},
        {REL0, REL2, 5, -5, line3, 7},
        {REL2, REL2, -5, -3, line2, 5},
        {REL2, REL2, -3, -2, line1, 3},
        {REL2, REL2, -2, 0, line0, 1}
    };

    blitsegment(display, rows, 7, line, x, y, w, h, x0, x2, cmap);

}

void blit_pcx(struct blit_display *display, int line, char *source, int x, int y, int x0, int x2)
{

    unsigned char buffer[4096];
    int i;

    pool_pcxload(&pcxresource, source);
    pool_pcxreadline(&pcxresource, line, y, buffer);

    for (i = x0; i < x2; i++)
    {

        if (util_intersects(line, y, y + pcxresource.height) && util_intersects(i, x, x + pcxresource.width))
        {

            unsigned int off = buffer[i - x] * 3;
            unsigned char r = pcxresource.colormap[off + 0];
            unsigned char g = pcxresource.colormap[off + 1];
            unsigned char b = pcxresource.colormap[off + 2];

            display->linebuffer[i] = (0xFF000000 | r << 16 | g << 8 | b);

        }

        else
        {

            display->linebuffer[i] = 0xFF000000;

        }

    }

}

void blit_initdisplay(struct blit_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp, unsigned int *linebuffer)
{

    display->framebuffer = framebuffer;
    display->size.w = w;
    display->size.h = h;
    display->bpp = bpp;
    display->linebuffer = linebuffer;

}

void blit(struct blit_display *display, int line, int x0, int x2)
{

    buffer_copy((unsigned int *)display->framebuffer + (line * display->size.w) + x0, display->linebuffer + x0, (x2 - x0) * display->bpp);

}

