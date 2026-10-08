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
#define CMAP_SHADOW                     0
#define CMAP_NORMAL                     1
#define CMAP_DARK                       2
#define CMAP_LIGHT                      3
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

/* rows of characters: '#' is the icon colour, S N D L are shadow, normal, dark and light, anything else is transparent */
static void blitbitmap(struct blit_display *display, char **rows, unsigned int count, int bx, int by, int line, int x0, int x2, unsigned int *cmap)
{

    char *row;
    int x;

    if (line < by || line >= by + (int)count)
        return;

    for (row = rows[line - by], x = bx; *row && x < x2; row++, x++)
    {

        if (x < x0)
            continue;

        switch (*row)
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
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        unsigned int index = text[i];
        struct text_atlas *atlas = &font->atlas[index];
        unsigned int lline = (line - ry) % font->lineheight - (font->lineheight - atlas->height) / 2;

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

void blit_iconarrowup(struct blit_display *display, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    static char *rows[6] = {
        "     ##",
        "    ####",
        "   ######",
        "  ########",
        " ##########",
        "############"
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

void blit_icondropdown(struct blit_display *display, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    static char *rows[14] = {
        "    ##",
        "   ####",
        "  ######",
        " ########",
        "##########",
        "",
        "",
        "",
        "",
        "##########",
        " ########",
        "  ######",
        "   ####",
        "    ##"
    };

    blitbitmap(display, rows, 14, region->position.x + (int)(region->size.w / 2) - 5, region->position.y + (int)(region->size.h / 2) - 7, line, x0, x2, cmap);

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

/* the arrow points with its top left corner, the text bar with its middle, the hand with its finger */
struct util_region blit_getmouseregion(unsigned int type, int mx, int my)
{

    switch (type)
    {

    case BLIT_MOUSE_TEXT:
        return util_region(mx - 3, my - 10, 7, 24);

    case BLIT_MOUSE_HAND:
        return util_region(mx - 8, my, 23, 24);

    }

    return util_region(mx, my, 18, 24);

}

void blit_mouse(struct blit_display *display, unsigned int type, int mx, int my, int line, int x0, int x2, unsigned int *cmap)
{

    static char *arrow[24] = {
        "SSS",
        "SLSS",
        "SLLSS",
        "SLNLSS",
        "SLNNLSS",
        "SLNNNLSS",
        "SLNNNNLSS",
        "SLNNNNNLSS",
        "SLNNNNNNLSS",
        "SLNNNNNNNLSS",
        "SLNNNNNNNNLSS",
        "SLNNNNNNNNNLSS",
        "SLNNNNNNNNNNLSS",
        "SLNNNNNNNNNNNLSS",
        "SLNNNNNNNNNNNNLSS",
        "SLNNNNNNNNNNNNNLSS",
        "SLNNNNNLLLLLLLLLLS",
        "SLNNNNLSSSSSSSSSSS",
        "SLNNNLSS",
        "SLNNLSS",
        "SLNLSS",
        "SLLSS",
        "SLSS",
        "SSS"
    };
    static char *text[24] = {
        "SSS    SSS",
        "SLLSSSSLLS",
        "SLLLLLLLLS",
        "SLNNNNNNLS",
        "SLLLNNLLLS",
        "SSSLNNLSSS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "  SLNNLS",
        "SSSLNNLSSS",
        "SLLLNNLLLS",
        "SLNNNNNNLS",
        "SLLLLLLLLS",
        "SLLSSSSLLS",
        "SSS    SSS"
    };
    static char *hand[24] = {
        "      SSSS",
        "     SSLLSS",
        "     SLNNLS",
        "     SLNNLS",
        "     SLNNLS",
        "     SLNNLS",
        "     SLNNLS",
        "     SLNNLS",
        "     SLNNLSSSSSSSSSSSSS",
        "     SLNNLLLSLLSLLSLLSS",
        " SS  SLNNLNNLNNLNNLNNLS",
        "SLLS SLNNLNNLNNLNNLNNLS",
        "SLNLSLNNNLNNLNNLNNLNNLS",
        "SLNNLNNNNNNNNNNNNNNNNLS",
        "SLNNNNNNNNNNNNNNNNNNNLS",
        "SLNNNNNNNNNNNNNNNNNNNLS",
        " SLNNNNNNNNNNNNNNNNNNLS",
        "  SLNNNNNNNNNNNNNNNNNLS",
        "   SLNNNNNNNNNNNNNNNNLS",
        "    SLNNNNNNNNNNNNNNNLS",
        "     SLNNNNNNNNNNNNNNLS",
        "      SLNNNNNNNNNNNNLS",
        "       SLLLLLLLLLLLLLS",
        "       SSSSSSSSSSSSSSS"
    };
    struct util_region region = blit_getmouseregion(type, mx, my);

    switch (type)
    {

    case BLIT_MOUSE_TEXT:
        blitbitmap(display, text, 24, region.position.x, region.position.y, line, x0, x2, cmap);

        break;

    case BLIT_MOUSE_HAND:
        blitbitmap(display, hand, 24, region.position.x, region.position.y, line, x0, x2, cmap);

        break;

    default:
        blitbitmap(display, arrow, 24, region.position.x, region.position.y, line, x0, x2, cmap);

        break;

    }

}

void blit_frame(struct blit_display *display, struct util_region *region, int line, int x0, int x2, unsigned int *cmap)
{

    static struct linesegment line0[1] = {
        {REL0, REL2, 0, 0, CMAP_SHADOW}
    };
    static struct linesegment line1[3] = {
        {REL0, REL0, 0, 2, CMAP_SHADOW},
        {REL0, REL2, 2, -2, CMAP_LIGHT},
        {REL2, REL2, -2, 0, CMAP_SHADOW}
    };
    static struct linesegment line2[5] = {
        {REL0, REL0, 0, 2, CMAP_SHADOW},
        {REL0, REL0, 2, 3, CMAP_LIGHT},
        {REL0, REL2, 3, -3, CMAP_DARK},
        {REL2, REL2, -3, -2, CMAP_LIGHT},
        {REL2, REL2, -2, 0, CMAP_SHADOW}
    };
    static struct linesegment line3[7] = {
        {REL0, REL0, 0, 2, CMAP_SHADOW},
        {REL0, REL0, 2, 3, CMAP_LIGHT},
        {REL0, REL0, 3, 5, CMAP_DARK},
        {REL0, REL2, 5, -5, CMAP_NORMAL},
        {REL2, REL2, -5, -3, CMAP_DARK},
        {REL2, REL2, -3, -2, CMAP_LIGHT},
        {REL2, REL2, -2, 0, CMAP_SHADOW}
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

    blitsegment(display, rows, 7, line, region, x0, x2, cmap);

}

/* draws the image scaled to w by h (nearest pixel); what is around it is left as it is */
void blit_pcx(struct blit_display *display, struct pool_pcxresource *resource, int line, int x, int y, unsigned int w, unsigned int h, int x0, int x2)
{

    unsigned char *row;
    int i;

    if (!w || !h || !util_intersects(line, y, y + h))
        return;

    row = pool_pcxreadline(resource, (line - y) * resource->height / h);

    for (i = util_max(x0, x); i < util_min(x2, x + w); i++)
    {

        unsigned int off = row[(i - x) * resource->width / w] * 3;
        unsigned char r = resource->colormap[off + 0];
        unsigned char g = resource->colormap[off + 1];
        unsigned char b = resource->colormap[off + 2];

        display->linebuffer[i] = (0xFF000000 | r << 16 | g << 8 | b);

    }

}

void blit_initdisplay(struct blit_display *display, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp, unsigned int *linebuffer)
{

    display->framebuffer = framebuffer;
    display->bpp = bpp;
    display->linebuffer = linebuffer;
    display->region = util_region(0, 0, w, h);

}

void blit(struct blit_display *display, int line, int x0, int x2)
{

    buffer_copy((unsigned int *)display->framebuffer + (line * display->region.size.w) + x0, display->linebuffer + x0, (x2 - x0) * display->bpp);

}

