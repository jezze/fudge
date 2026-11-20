#include <fudge.h>
#include "html.h"
#include "css.h"
#include "font.h"
#include "layout.h"
#include "paint.h"

#define WHITE                           0xFFFFFFFF
#define PLACEHOLDER                     0xFFC0C0C0
#define SINES                           91
#define LIMIT                           30000

static int sines[SINES] = {
    0, 17, 35, 52, 70, 87, 105, 122, 139, 156, 174, 191, 208, 225, 242, 259, 276, 292, 309, 326,
    342, 358, 375, 391, 407, 423, 438, 454, 469, 485, 500, 515, 530, 545, 559, 574, 588, 602, 616, 629,
    643, 656, 669, 682, 695, 707, 719, 731, 743, 755, 766, 777, 788, 799, 809, 819, 829, 839, 848, 857,
    866, 875, 883, 891, 899, 906, 914, 921, 927, 934, 940, 946, 951, 956, 961, 966, 970, 974, 978, 982,
    985, 988, 990, 993, 995, 996, 998, 999, 999, 1000, 1000
};

static unsigned int blend(unsigned int under, unsigned int color)
{

    unsigned int alpha = color >> 24;
    unsigned int inverse = 255 - alpha;
    unsigned int r = (((color >> 16) & 0xFF) * alpha + ((under >> 16) & 0xFF) * inverse) / 255;
    unsigned int g = (((color >> 8) & 0xFF) * alpha + ((under >> 8) & 0xFF) * inverse) / 255;
    unsigned int b = ((color & 0xFF) * alpha + (under & 0xFF) * inverse) / 255;

    return 0xFF000000 | (r << 16) | (g << 8) | b;

}

static void fill(struct paint_canvas *canvas, int x, int y, int w, int h, unsigned int color)
{

    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = (x + w > (int)canvas->width) ? (int)canvas->width : x + w;
    int y1 = (y + h > (int)canvas->height) ? (int)canvas->height : y + h;
    unsigned int alpha = color >> 24;
    int i;
    int j;

    if (!alpha)
        return;

    for (j = y0; j < y1; j++)
    {

        unsigned int *row = canvas->pixels + j * canvas->width;

        for (i = x0; i < x1; i++)
            row[i] = (alpha == 0xFF) ? color : blend(row[i], color);

    }

}

static int sine(int angle)
{

    angle %= 360;

    if (angle < 0)
        angle += 360;

    if (angle <= 90)
        return sines[angle];

    if (angle <= 180)
        return sines[180 - angle];

    if (angle <= 270)
        return -sines[angle - 180];

    return -sines[360 - angle];

}

static int root(int value)
{

    int result = 0;
    int bit = 1 << 30;

    if (value <= 0)
        return 0;

    while (bit > value)
        bit >>= 2;

    while (bit)
    {

        if (value >= result + bit)
        {

            value -= result + bit;
            result = (result >> 1) + bit;

        }

        else
        {

            result >>= 1;

        }

        bit >>= 2;

    }

    return result;

}

static unsigned int mixchannel(unsigned int a, unsigned int b, int shift, int t, int range)
{

    int ca = (a >> shift) & 0xFF;
    int cb = (b >> shift) & 0xFF;

    return (unsigned int)(ca + (cb - ca) * t / range) << shift;

}

static unsigned int stopcolor(struct css_gradient *gradient, int t)
{

    unsigned int i;

    if (t <= gradient->stops[0].position)
        return gradient->stops[0].color;

    for (i = 1; i < gradient->count; i++)
    {

        struct css_stop *a = &gradient->stops[i - 1];
        struct css_stop *b = &gradient->stops[i];

        if (t <= b->position)
        {

            int range = b->position - a->position;
            unsigned int ca = (a->color >> 24) ? a->color : (b->color & 0xFFFFFF);
            unsigned int cb = (b->color >> 24) ? b->color : (a->color & 0xFFFFFF);

            if (range <= 0)
                return b->color;

            t -= a->position;

            return mixchannel(ca, cb, 24, t, range) | mixchannel(ca, cb, 16, t, range) | mixchannel(ca, cb, 8, t, range) | mixchannel(ca, cb, 0, t, range);

        }

    }

    return gradient->stops[gradient->count - 1].color;

}

static void paintgradient(struct paint_canvas *canvas, struct css_gradient *gradient, struct layout_box *box)
{

    int x0 = (box->x < 0) ? 0 : box->x;
    int y0 = (box->y < 0) ? 0 : box->y;
    int x1 = (box->x + box->w > (int)canvas->width) ? (int)canvas->width : box->x + box->w;
    int y1 = (box->y + box->h > (int)canvas->height) ? (int)canvas->height : box->y + box->h;
    int sinus = sine(gradient->angle);
    int cosinus = sine(gradient->angle + 90);
    int length = (box->w * ((sinus < 0) ? -sinus : sinus) + box->h * ((cosinus < 0) ? -cosinus : cosinus)) / 1000;
    int cx = box->x + box->w * gradient->x / 1000;
    int cy = box->y + box->h * gradient->y / 1000;
    int rx = box->w * gradient->width / 1000;
    int ry = box->h * gradient->height / 1000;
    int i;
    int j;

    if (box->w <= 0 || box->h <= 0)
        return;

    if (!gradient->radial)
    {

        cx = box->x + box->w / 2;
        cy = box->y + box->h / 2;

    }

    for (j = y0; j < y1; j++)
    {

        unsigned int *row = canvas->pixels + j * canvas->width;

        for (i = x0; i < x1; i++)
        {

            unsigned int color;
            int t;

            if (gradient->radial)
            {

                int dx = (rx > 0) ? (i - cx) * 1000 / rx : 0;
                int dy = (ry > 0) ? (j - cy) * 1000 / ry : 0;

                dx = (dx > LIMIT) ? LIMIT : (dx < -LIMIT) ? -LIMIT : dx;
                dy = (dy > LIMIT) ? LIMIT : (dy < -LIMIT) ? -LIMIT : dy;

                t = root(dx * dx + dy * dy);

            }

            else
            {

                t = (length > 0) ? ((i - cx) * sinus - (j - cy) * cosinus) / length + 500 : 500;

            }

            color = stopcolor(gradient, t);

            if ((color >> 24) == 0xFF)
                row[i] = color;
            else if (color >> 24)
                row[i] = blend(row[i], color);

        }

    }

}

static void paintbox(struct paint_canvas *canvas, struct layout *layout, struct layout_box *box, struct css_style *style)
{

    fill(canvas, box->x, box->y, box->w, box->h, style->background);

    if (style->gradient)
        paintgradient(canvas, &layout->gradients[style->gradient - 1], box);

    fill(canvas, box->x, box->y, box->w, style->borderwidth[CSS_TOP], style->bordercolor[CSS_TOP]);
    fill(canvas, box->x, box->y + box->h - style->borderwidth[CSS_BOTTOM], box->w, style->borderwidth[CSS_BOTTOM], style->bordercolor[CSS_BOTTOM]);
    fill(canvas, box->x, box->y, style->borderwidth[CSS_LEFT], box->h, style->bordercolor[CSS_LEFT]);
    fill(canvas, box->x + box->w - style->borderwidth[CSS_RIGHT], box->y, style->borderwidth[CSS_RIGHT], box->h, style->bordercolor[CSS_RIGHT]);

}

static int padding(struct css_length *length)
{

    return (length->unit == CSS_UNIT_PX) ? length->value : 0;

}

static void paintplaceholder(struct paint_canvas *canvas, struct layout_box *box)
{

    fill(canvas, box->x, box->y, box->w, 1, PLACEHOLDER);
    fill(canvas, box->x, box->y + box->h - 1, box->w, 1, PLACEHOLDER);
    fill(canvas, box->x, box->y, 1, box->h, PLACEHOLDER);
    fill(canvas, box->x + box->w - 1, box->y, 1, box->h, PLACEHOLDER);

}

static void paintpicture(struct paint_canvas *canvas, struct layout_picture *picture, int x, int y, int w, int h)
{

    unsigned int *pixels = canvas->heap + picture->offset / 4;
    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = (x + w > (int)canvas->width) ? (int)canvas->width : x + w;
    int y1 = (y + h > (int)canvas->height) ? (int)canvas->height : y + h;
    int i;
    int j;

    if (w <= 0 || h <= 0)
        return;

    for (j = y0; j < y1; j++)
    {

        unsigned int *row = canvas->pixels + j * canvas->width;
        unsigned int *source = pixels + (unsigned int)(j - y) * picture->height / (unsigned int)h * picture->width;

        for (i = x0; i < x1; i++)
        {

            unsigned int color = source[(unsigned int)(i - x) * picture->width / (unsigned int)w];
            unsigned int alpha = color >> 24;

            if (alpha == 0xFF)
                row[i] = color;
            else if (alpha)
                row[i] = blend(row[i], color);

        }

    }

}

static int paintglyph(struct paint_canvas *canvas, struct font_face *face, unsigned int codepoint, int x, int y, unsigned int scale, unsigned int color)
{

    struct font_glyph *glyph = font_getglyph(face, codepoint);
    int top = y + (face->height - glyph->height) * scale;
    unsigned int row;
    unsigned int column;

    if (x + glyph->width * (int)scale <= 0 || x >= (int)canvas->width)
        return glyph->width * scale;

    for (row = 0; row < glyph->height; row++)
    {

        unsigned char *bits = glyph->rows + row * FONT_ROWBYTES;

        for (column = 0; column < glyph->width; column++)
        {

            if (bits[column >> 3] & (0x80 >> (column & 7)))
                fill(canvas, x + column * scale, top + row * scale, scale, scale, color);

        }

    }

    return glyph->width * scale;

}

static void painttext(struct paint_canvas *canvas, struct layout *layout, struct layout_fragment *fragment, unsigned int color)
{

    struct font_face *face = &layout->faces[fragment->face];
    char *text = (fragment->type == LAYOUT_FRAGMENT_TEXT) ? layout->document->text + fragment->text : layout->text + fragment->text;
    int x = fragment->x;
    unsigned int space = 0;
    unsigned int i = 0;

    if (fragment->y + fragment->h <= 0 || fragment->y >= (int)canvas->height)
        return;

    while (i < fragment->length)
    {

        unsigned int codepoint;

        if (fragment->collapse && (text[i] == ' ' || text[i] == '\n' || text[i] == '\t' || text[i] == '\r' || text[i] == '\f'))
        {

            if (!space)
                x += paintglyph(canvas, face, ' ', x, fragment->y, fragment->scale, color);

            space = 1;
            i++;

            continue;

        }

        space = 0;
        i += utf8_read(text + i, fragment->length - i, &codepoint);
        x += paintglyph(canvas, face, codepoint, x, fragment->y, fragment->scale, color);

    }

}

static void paintmarker(struct paint_canvas *canvas, struct layout_fragment *fragment, unsigned int color)
{

    int size = (fragment->w > 2) ? fragment->w - 2 : 1;
    int top = fragment->y + fragment->ascent - fragment->h / 4 - size / 2;
    int thickness = (size > 6) ? 2 : 1;

    if (fragment->text == CSS_LIST_CIRCLE)
    {

        fill(canvas, fragment->x, top, size, thickness, color);
        fill(canvas, fragment->x, top + size - thickness, size, thickness, color);
        fill(canvas, fragment->x, top, thickness, size, color);
        fill(canvas, fragment->x + size - thickness, top, thickness, size, color);

    }

    else
    {

        fill(canvas, fragment->x, top, size, size, color);

    }

}

static void paintfragment(struct paint_canvas *canvas, struct layout *layout, struct layout_fragment *fragment)
{

    struct html_document *document = layout->document;
    unsigned int owner = (fragment->type == LAYOUT_FRAGMENT_TEXT) ? document->nodes[fragment->node].parent : fragment->node;
    struct css_style *style = &layout->styles[owner];
    unsigned int node;

    if (style->hidden)
        return;

    if (fragment->type == LAYOUT_FRAGMENT_TEXT)
    {

        for (node = owner; node && document->nodes[node].type == HTML_NODE_ELEMENT && layout->styles[node].display == CSS_DISPLAY_INLINE; node = document->nodes[node].parent)
            fill(canvas, fragment->x, fragment->y, fragment->w, fragment->h, layout->styles[node].background);

    }

    if (fragment->type == LAYOUT_FRAGMENT_MARKER)
    {

        paintmarker(canvas, fragment, style->color);

        return;

    }

    painttext(canvas, layout, fragment, style->color);

    if (style->decoration & CSS_DECORATION_UNDERLINE)
        fill(canvas, fragment->x, fragment->y + fragment->ascent + fragment->scale, fragment->w, fragment->scale, style->color);

    if (style->decoration & CSS_DECORATION_LINETHROUGH)
        fill(canvas, fragment->x, fragment->y + fragment->h / 2, fragment->w, fragment->scale, style->color);

}

unsigned int paint_getbackground(struct layout *layout)
{

    struct html_document *document = layout->document;
    unsigned int color = 0;

    if (document->html)
        color = layout->styles[document->html].background;

    if (!(color >> 24) && document->body)
        color = layout->styles[document->body].background;

    return (color >> 24) ? blend(WHITE, color) : WHITE;

}

void paint_document(struct paint_canvas *canvas, struct layout *layout)
{

    struct html_document *document = layout->document;
    unsigned int node = document->html;
    unsigned int i;

    fill(canvas, 0, 0, canvas->width, canvas->height, paint_getbackground(layout));

    while (node)
    {

        struct layout_box *box = &layout->boxes[node];
        struct css_style *style = &layout->styles[node];

        if (document->nodes[node].type == HTML_NODE_ELEMENT && box->placed && !style->hidden)
        {

            if (style->display != CSS_DISPLAY_INLINE)
                paintbox(canvas, layout, box, style);

            if (document->nodes[node].tag == HTML_TAG_IMG)
            {

                if (layout->picture[node] && canvas->heap)
                    paintpicture(canvas, &layout->pictures[layout->picture[node] - 1], box->x + style->borderwidth[CSS_LEFT] + padding(&style->padding[CSS_LEFT]), box->y + style->borderwidth[CSS_TOP] + padding(&style->padding[CSS_TOP]), box->w - style->borderwidth[CSS_LEFT] - style->borderwidth[CSS_RIGHT] - padding(&style->padding[CSS_LEFT]) - padding(&style->padding[CSS_RIGHT]), box->h - style->borderwidth[CSS_TOP] - style->borderwidth[CSS_BOTTOM] - padding(&style->padding[CSS_TOP]) - padding(&style->padding[CSS_BOTTOM]));
                else
                    paintplaceholder(canvas, box);

            }

        }

        node = html_next(document, node, document->html);

    }

    for (i = 0; i < layout->fragmentcount; i++)
        paintfragment(canvas, layout, &layout->fragments[i]);

}
