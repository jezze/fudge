#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "blit.h"

#define LINESEGMENT_TYPE_RELX0X0        1
#define LINESEGMENT_TYPE_RELX0X2        2
#define LINESEGMENT_TYPE_RELX2X2        3
#define ROWSEGMENT_TYPE_RELY0Y0         1
#define ROWSEGMENT_TYPE_RELY0Y2         2
#define ROWSEGMENT_TYPE_RELY2Y2         3
#define CMAP_PANEL_SHADOW               0
#define CMAP_PANEL_NORMAL               1
#define CMAP_PANEL_HIGHLIGHT            2
#define CMAP_FRAME_SHADOW               0
#define CMAP_FRAME_NORMAL               1
#define CMAP_FRAME_HIGHLIGHT            2
#define CMAP_MOUSE_SHADOW               0
#define CMAP_MOUSE_NORMAL               1
#define CMAP_MOUSE_HIGHLIGHT            2

struct linesegment
{

    unsigned int type;
    int p0;
    int p1;
    unsigned int color;

};

struct rowsegment
{

    unsigned int type;
    int p0;
    int p1;
    struct linesegment *lines;
    unsigned int numlines;

};

static unsigned int linebuffer[3840];
static struct pool_pcxresource pcxresource;

static struct rowsegment *findrowsegment(struct rowsegment *rows, unsigned int length, int line, int y, int h)
{

    int lline = line - y;

    if (lline >= 0 && lline < h)
    {

        unsigned int i;

        for (i = 0; i < length; i++)
        {

            struct rowsegment *current = &rows[i];

            switch (current->type)
            {

            case ROWSEGMENT_TYPE_RELY0Y0:
                if (util_intersects(lline, current->p0, current->p1))
                    return current;

                break;

            case ROWSEGMENT_TYPE_RELY0Y2:
                if (util_intersects(lline, current->p0, h + current->p1))
                    return current;

                break;

            case ROWSEGMENT_TYPE_RELY2Y2:
                if (util_intersects(lline, h + current->p0, h + current->p1))
                    return current;

                break;

            }

        }

    }

    return 0;

}

static void blitrowsegment(struct blit_display *display, int x, int w, struct rowsegment *rs, int line, int x0, int x2, unsigned int *cmap)
{

    unsigned int i;

    for (i = 0; i < rs->numlines; i++)
    {

        struct linesegment *p = &rs->lines[i];
        int p0;
        int p1;

        switch (p->type)
        {

        case LINESEGMENT_TYPE_RELX0X0:
            p0 = x + p->p0;
            p1 = x + p->p1;

            break;

        case LINESEGMENT_TYPE_RELX0X2:
            p0 = x + p->p0;
            p1 = x + w + p->p1;

            break;

        case LINESEGMENT_TYPE_RELX2X2:
            p0 = x + w + p->p0;
            p1 = x + w + p->p1;

            break;

        default:
            p0 = x;
            p1 = x + w;

            break;

        }

        p0 = util_max(p0, x0);
        p1 = util_min(p1, x2);

        blit_alphaline(display, cmap[p->color], line, p0, p1);

    }

}

void blit_line(struct blit_display *display, unsigned int color, int line, int x0, int x2)
{

    while (x0 < x2)
        linebuffer[x0++] = color;

}

void blit_alphaline(struct blit_display *display, unsigned int color, int line, int x0, int x2)
{

    unsigned char *fg = (unsigned char *)&color;

    while (x0 < x2)
    {

        unsigned char *bg = (unsigned char *)&linebuffer[x0];
        unsigned char *result = bg;
        unsigned int alpha = fg[3] + 1;
        unsigned int inv_alpha = 256 - fg[3];

        result[0] = ((alpha * fg[0] + inv_alpha * bg[0]) >> 8);
        result[1] = ((alpha * fg[1] + inv_alpha * bg[1]) >> 8);
        result[2] = ((alpha * fg[2] + inv_alpha * bg[2]) >> 8);
        result[3] = 0xFF;

        x0++;

    }

}

void blit_textnormal(struct blit_display *display, struct blit_font *font, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x2)
{

    unsigned int i;

    for (i = 0; i < length; i++)
    {

        unsigned short index = pcf_getindex(font->data, text[i]);
        unsigned int offset = pcf_getbitmapoffset(font->data, index);
        struct pcf_metricsdata metricsdata;
        unsigned int lline;
        unsigned int height;

        pcf_readmetricsdata(font->data, index, &metricsdata);

        height = metricsdata.ascent + metricsdata.descent;
        lline = (line - ry) % font->lineheight - (font->lineheight - height) / 2;

        if (util_intersects(lline, 0, height))
        {

            if (util_intersects(rx, x0, x2) || util_intersects(rx + metricsdata.width, x0, x2))
            {

                unsigned char *data = font->bitmapdata + offset + lline * font->bitmapalign;
                int r0 = util_max(0, x0 - rx);
                int r1 = util_min(x2 - rx, metricsdata.width);
                unsigned int r;

                for (r = r0; r < r1; r++)
                {

                    if (data[(r >> 3)] & (0x80 >> (r % 8)))
                        blit_line(display, color, line, rx + r, rx + r + 1);

                }

            }

        }

        rx += metricsdata.width;

    }

}

void blit_textinverted(struct blit_display *display, struct blit_font *font, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x2)
{

    unsigned int i;

    for (i = 0; i < length; i++)
    {

        unsigned short index = pcf_getindex(font->data, text[i]);
        unsigned int offset = pcf_getbitmapoffset(font->data, index);
        struct pcf_metricsdata metricsdata;
        unsigned int lline;
        unsigned int height;

        pcf_readmetricsdata(font->data, index, &metricsdata);

        height = metricsdata.ascent + metricsdata.descent;
        lline = (line - ry) % font->lineheight - (font->lineheight - height) / 2;

        if (util_intersects(lline, 0, height))
        {

            if (util_intersects(rx, x0, x2) || util_intersects(rx + metricsdata.width, x0, x2))
            {

                unsigned char *data = font->bitmapdata + offset + lline * font->bitmapalign;
                int r0 = util_max(0, x0 - rx);
                int r1 = util_min(x2 - rx, metricsdata.width);
                unsigned int r;

                for (r = r0; r < r1; r++)
                {

                    if (!(data[(r >> 3)] & (0x80 >> (r % 8))))
                        blit_line(display, color, line, rx + r, rx + r + 1);

                }

            }

        }

        rx += metricsdata.width;

    }

}

void blit_mouse(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 3, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line1[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 4, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line2[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 3, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 3, 5, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line3[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 3, 4, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 4, 6, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line4[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 4, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 4, 5, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 5, 7, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line5[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 5, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 5, 6, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 6, 8, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line6[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 6, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 6, 7, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 7, 9, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line7[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 7, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 7, 8, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 8, 10, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line8[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 8, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 8, 9, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 9, 11, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line9[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 9, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 9, 10, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 10, 12, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line10[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 10, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 10, 11, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 11, 13, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line11[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 11, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 11, 12, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 12, 14, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line12[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 12, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 12, 13, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 13, 15, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line13[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 13, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 13, 14, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 14, 16, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line14[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 14, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 14, 15, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 15, 17, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line15[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 15, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 15, 16, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 16, 18, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line16[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 7, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 7, 17, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 17, 18, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line17[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 6, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 6, 7, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 7, 18, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line18[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 5, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 5, 6, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 6, 8, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line19[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 4, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 4, 5, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 5, 7, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line20[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_MOUSE_NORMAL},
        {LINESEGMENT_TYPE_RELX0X0, 3, 4, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 4, 6, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line21[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 3, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 3, 5, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line22[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 1, CMAP_MOUSE_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 1, 2, CMAP_MOUSE_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 2, 4, CMAP_MOUSE_SHADOW}
    };
    static struct linesegment line23[1] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 3, CMAP_MOUSE_SHADOW}
    };
    static struct rowsegment rows[24] = {
        {ROWSEGMENT_TYPE_RELY0Y0, 0, 1, line0, 1},
        {ROWSEGMENT_TYPE_RELY0Y0, 1, 2, line1, 3},
        {ROWSEGMENT_TYPE_RELY0Y0, 2, 3, line2, 3},
        {ROWSEGMENT_TYPE_RELY0Y0, 3, 4, line3, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 4, 5, line4, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 5, 6, line5, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 6, 7, line6, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 7, 8, line7, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 8, 9, line8, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 9, 10, line9, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 10, 11, line10, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 11, 12, line11, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 12, 13, line12, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 13, 14, line13, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 14, 15, line14, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 15, 16, line15, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 16, 17, line16, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 17, 18, line17, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 18, 19, line18, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 19, 20, line19, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 20, 21, line20, 5},
        {ROWSEGMENT_TYPE_RELY0Y0, 21, 22, line21, 3},
        {ROWSEGMENT_TYPE_RELY0Y0, 22, 23, line22, 3},
        {ROWSEGMENT_TYPE_RELY0Y0, 23, 24, line23, 1}
    };
    struct rowsegment *rs = findrowsegment(rows, 24, line, y, h);

    if (rs)
        blitrowsegment(display, x, w, rs, line, x0, x2, cmap);

}

void blit_panel(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {LINESEGMENT_TYPE_RELX0X2, 0, 0, CMAP_PANEL_SHADOW}
    };
    static struct linesegment line1[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_PANEL_SHADOW},
        {LINESEGMENT_TYPE_RELX0X2, 2, -2, CMAP_PANEL_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX2X2, -2, 0, CMAP_PANEL_SHADOW}
    };
    static struct linesegment line2[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_PANEL_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_PANEL_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X2, 3, -3, CMAP_PANEL_NORMAL},
        {LINESEGMENT_TYPE_RELX2X2, -3, -2, CMAP_PANEL_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX2X2, -2, 0, CMAP_PANEL_SHADOW}
    };
    static struct rowsegment rows[5] = {
        {ROWSEGMENT_TYPE_RELY0Y0, 0, 2, line0, 1},
        {ROWSEGMENT_TYPE_RELY0Y0, 2, 3, line1, 3},
        {ROWSEGMENT_TYPE_RELY0Y2, 3, -3, line2, 5},
        {ROWSEGMENT_TYPE_RELY2Y2, -3, -2, line1, 3},
        {ROWSEGMENT_TYPE_RELY2Y2, -2, 0, line0, 1}
    };
    struct rowsegment *rs = findrowsegment(rows, 5, line, y, h);

    if (rs)
        blitrowsegment(display, x, w, rs, line, x0, x2, cmap);

}

void blit_frame(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {LINESEGMENT_TYPE_RELX0X2, 0, 0, CMAP_FRAME_SHADOW}
    };
    static struct linesegment line1[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_FRAME_SHADOW},
        {LINESEGMENT_TYPE_RELX0X2, 2, -2, CMAP_FRAME_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX2X2, -2, 0, CMAP_FRAME_SHADOW}
    };
    static struct linesegment line2[7] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_FRAME_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_FRAME_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 3, 5, CMAP_FRAME_SHADOW},
        {LINESEGMENT_TYPE_RELX0X2, 5, -5, CMAP_FRAME_NORMAL},
        {LINESEGMENT_TYPE_RELX2X2, -5, -3, CMAP_FRAME_SHADOW},
        {LINESEGMENT_TYPE_RELX2X2, -3, -2, CMAP_FRAME_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX2X2, -2, 0, CMAP_FRAME_SHADOW}
    };
    static struct rowsegment rows[5] = {
        {ROWSEGMENT_TYPE_RELY0Y0, 0, 2, line0, 1},
        {ROWSEGMENT_TYPE_RELY0Y0, 2, 3, line1, 3},
        {ROWSEGMENT_TYPE_RELY0Y2, 3, -3, line2, 7},
        {ROWSEGMENT_TYPE_RELY2Y2, -3, -2, line1, 3},
        {ROWSEGMENT_TYPE_RELY2Y2, -2, 0, line0, 1}
    };
    struct rowsegment *rs = findrowsegment(rows, 5, line, y, h);

    if (rs)
        blitrowsegment(display, x, w, rs, line, x0, x2, cmap);

}

void blit_pcx(struct blit_display *display, int line, char *source, int x, int y, int x0, int x2)
{

    unsigned char buffer[BUFFER_SIZE];
    int i;

    pool_pcxload(&pcxresource, source);
    pool_pcxreadline(&pcxresource, line, y, buffer);

    for (i = x0; i < x2; i++)
    {

        unsigned int off = buffer[i - x];
        unsigned char r = pcxresource.colormap[off * 3 + 0];
        unsigned char g = pcxresource.colormap[off * 3 + 1];
        unsigned char b = pcxresource.colormap[off * 3 + 2];

        linebuffer[i] = (0xFF000000 | r << 16 | g << 8 | b);

    }

}

void blit_initdisplay(struct blit_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp)
{

    display->framebuffer = framebuffer;
    display->size.w = w;
    display->size.h = h;
    display->bpp = bpp;

}

void blit(struct blit_display *display, struct blit_damage *damage, int line)
{

    buffer_copy((unsigned int *)display->framebuffer + (line * display->size.w) + damage->position0.x, linebuffer + damage->position0.x, (damage->position2.x - damage->position0.x) * display->bpp);

}

