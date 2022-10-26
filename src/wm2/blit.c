#include <fudge.h>
#include <abi.h>
#include <image.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "blit.h"

#define LINESEGMENT_TYPE_RELX0X0        1
#define LINESEGMENT_TYPE_RELX0X1        2
#define LINESEGMENT_TYPE_RELX1X1        3
#define ROWSEGMENT_TYPE_RELY0Y0         1
#define ROWSEGMENT_TYPE_RELY0Y1         2
#define ROWSEGMENT_TYPE_RELY1Y1         3
#define CMAP_PANEL_SHADOW               0
#define CMAP_PANEL_NORMAL               1
#define CMAP_PANEL_HIGHLIGHT            2
#define CMAP_FRAME_SHADOW               0
#define CMAP_FRAME_NORMAL               1
#define CMAP_FRAME_HIGHLIGHT            2

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

            case ROWSEGMENT_TYPE_RELY0Y1:
                if (util_intersects(lline, current->p0, h + current->p1))
                    return current;

                break;

            case ROWSEGMENT_TYPE_RELY1Y1:
                if (util_intersects(lline, h + current->p0, h + current->p1))
                    return current;

                break;

            }

        }

    }

    return 0;

}

void blit_line(struct blit_display *display, unsigned int color, int line, int x0, int x1)
{

    while (x0 < x1)
        linebuffer[x0++] = color;

}

void blit_alphaline(struct blit_display *display, unsigned int color, int line, int x0, int x1)
{

    unsigned char *fg = (unsigned char *)&color;

    while (x0 < x1)
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

void blit_cmap32line(struct blit_display *display, int x, void *idata, unsigned int iwidth, unsigned int *cmap, int lline)
{

    unsigned char *data = idata;
    unsigned int w = (x + iwidth >= display->size.w) ? display->size.w - x : iwidth;
    int i;

    for (i = 0; i < w; i++)
    {

        unsigned int soffset = (lline * iwidth + i);
        unsigned int toffset = x + i;

        if (data[soffset] != 0xFF)
            linebuffer[toffset] = cmap[data[soffset]];

    }

}

void blit_textnormal(struct blit_display *display, struct blit_font *font, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x1)
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

            if (util_intersects(rx, x0, x1) || util_intersects(rx + metricsdata.width, x0, x1))
            {

                unsigned char *data = font->bitmapdata + offset + lline * font->bitmapalign;
                int r0 = util_max(0, x0 - rx);
                int r1 = util_min(x1 - rx, metricsdata.width);
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

void blit_textinverted(struct blit_display *display, struct blit_font *font, unsigned int color, char *text, unsigned int length, int rx, int ry, int line, int x0, int x1)
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

            if (util_intersects(rx, x0, x1) || util_intersects(rx + metricsdata.width, x0, x1))
            {

                unsigned char *data = font->bitmapdata + offset + lline * font->bitmapalign;
                int r0 = util_max(0, x0 - rx);
                int r1 = util_min(x1 - rx, metricsdata.width);
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

static void blitrowsegment(struct blit_display *display, int x, int w, struct rowsegment *rs, int line, int x0, int x1, unsigned int *cmap)
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

        case LINESEGMENT_TYPE_RELX0X1:
            p0 = x + p->p0;
            p1 = x + w + p->p1;

            break;

        case LINESEGMENT_TYPE_RELX1X1:
            p0 = x + w + p->p0;
            p1 = x + w + p->p1;

            break;

        default:
            p0 = x;
            p1 = x + w;

            break;

        }

        p0 = util_max(p0, x0);
        p1 = util_min(p1, x1);

        blit_alphaline(display, cmap[p->color], line, p0, p1);

    }

}

void blit_panel(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x1, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 0, 0, CMAP_PANEL_SHADOW}
    };
    static struct linesegment line1[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_PANEL_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 2, -2, CMAP_PANEL_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_PANEL_SHADOW}
    };
    static struct linesegment line2[5] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_PANEL_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_PANEL_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X1, 3, -3, CMAP_PANEL_NORMAL},
        {LINESEGMENT_TYPE_RELX1X1, -3, -2, CMAP_PANEL_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_PANEL_SHADOW}
    };
    static struct rowsegment rows[5] = {
        {ROWSEGMENT_TYPE_RELY0Y0, 0, 2, line0, 1},
        {ROWSEGMENT_TYPE_RELY0Y0, 2, 3, line1, 3},
        {ROWSEGMENT_TYPE_RELY0Y1, 3, -3, line2, 5},
        {ROWSEGMENT_TYPE_RELY1Y1, -3, -2, line1, 3},
        {ROWSEGMENT_TYPE_RELY1Y1, -2, 0, line0, 1}
    };
    struct rowsegment *rs = findrowsegment(rows, 5, line, y, h);

    if (rs)
        blitrowsegment(display, x, w, rs, line, x0, x1, cmap);

}

void blit_frame(struct blit_display *display, int x, int y, int w, int h, int line, int x0, int x1, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {LINESEGMENT_TYPE_RELX0X1, 0, 0, CMAP_FRAME_SHADOW}
    };
    static struct linesegment line1[3] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_FRAME_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 2, -2, CMAP_FRAME_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_FRAME_SHADOW}
    };
    static struct linesegment line2[7] = {
        {LINESEGMENT_TYPE_RELX0X0, 0, 2, CMAP_FRAME_SHADOW},
        {LINESEGMENT_TYPE_RELX0X0, 2, 3, CMAP_FRAME_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX0X0, 3, 5, CMAP_FRAME_SHADOW},
        {LINESEGMENT_TYPE_RELX0X1, 5, -5, CMAP_FRAME_NORMAL},
        {LINESEGMENT_TYPE_RELX1X1, -5, -3, CMAP_FRAME_SHADOW},
        {LINESEGMENT_TYPE_RELX1X1, -3, -2, CMAP_FRAME_HIGHLIGHT},
        {LINESEGMENT_TYPE_RELX1X1, -2, 0, CMAP_FRAME_SHADOW}
    };
    static struct rowsegment rows[5] = {
        {ROWSEGMENT_TYPE_RELY0Y0, 0, 2, line0, 1},
        {ROWSEGMENT_TYPE_RELY0Y0, 2, 3, line1, 3},
        {ROWSEGMENT_TYPE_RELY0Y1, 3, -3, line2, 7},
        {ROWSEGMENT_TYPE_RELY1Y1, -3, -2, line1, 3},
        {ROWSEGMENT_TYPE_RELY1Y1, -2, 0, line0, 1}
    };
    struct rowsegment *rs = findrowsegment(rows, 5, line, y, h);

    if (rs)
        blitrowsegment(display, x, w, rs, line, x0, x1, cmap);

}

void blit_pcx(struct blit_display *display, int line, char *source, int x, int y, int x0, int x1)
{

    unsigned char buffer[BUFFER_SIZE];
    int i;

    pool_pcxload(&pcxresource, source);
    pool_pcxreadline(&pcxresource, line, y, buffer);

    for (i = x0; i < x1; i++)
    {

        unsigned int off = buffer[i - x];
        unsigned char r = pcxresource.colormap[off * 3 + 0];
        unsigned char g = pcxresource.colormap[off * 3 + 1];
        unsigned char b = pcxresource.colormap[off * 3 + 2];

        linebuffer[i] = (0xFF000000 | r << 16 | g << 8 | b);

    }

}

void blit(struct blit_display *display, int line)
{

    buffer_copy((unsigned int *)display->framebuffer + (line * display->size.w) + display->damage.position0.x, linebuffer + display->damage.position0.x, (display->damage.position1.x - display->damage.position0.x) * display->bpp);

}

