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
#define INLINES                         16

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

    int x0 = (x < canvas->clipleft) ? canvas->clipleft : x;
    int y0 = (y < canvas->cliptop) ? canvas->cliptop : y;
    int x1 = (x + w > canvas->clipright) ? canvas->clipright : x + w;
    int y1 = (y + h > canvas->clipbottom) ? canvas->clipbottom : y + h;
    unsigned int alpha = color >> 24;
    int i;
    int j;

    if (!alpha)
        return;

    for (j = y0; j < y1; j++)
    {

        unsigned int *row = canvas->pixels + (j - canvas->top) * canvas->width;

        for (i = x0; i < x1; i++)
            row[i] = (alpha == 0xFF) ? color : blend(row[i], color);

    }

}

static int root(int value);

static int cornerinset(int row, int height, int radius)
{

    int d;

    if (radius <= 0)
        return 0;

    if (row >= height - radius)
        row = height - 1 - row;

    if (row >= radius || row < 0)
        return 0;

    d = radius * 2 - row * 2 - 1;

    return radius - root(radius * radius * 4 - d * d) / 2;

}

static int getradius(struct css_style *style, struct layout_box *box)
{

    int limit = ((box->w < box->h) ? box->w : box->h) / 2;
    int radius = (style->radius.unit == CSS_UNIT_PERCENT) ? limit * 2 * (style->radius.value / 10) / 10000 : style->radius.value;

    return (radius > limit) ? limit : (radius < 0) ? 0 : radius;

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

static void paintgradient(struct paint_canvas *canvas, struct css_gradient *gradient, struct layout_box *box, int radius)
{

    int x0 = (box->x < canvas->clipleft) ? canvas->clipleft : box->x;
    int y0 = (box->y < canvas->cliptop) ? canvas->cliptop : box->y;
    int x1 = (box->x + box->w > canvas->clipright) ? canvas->clipright : box->x + box->w;
    int y1 = (box->y + box->h > canvas->clipbottom) ? canvas->clipbottom : box->y + box->h;
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

        unsigned int *row = canvas->pixels + (j - canvas->top) * canvas->width;
        int inset = cornerinset(j - box->y, box->h, radius);
        int start = (box->x + inset > x0) ? box->x + inset : x0;
        int end = (box->x + box->w - inset < x1) ? box->x + box->w - inset : x1;

        for (i = start; i < end; i++)
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

    int radius = getradius(style, box);
    int bt = style->borderwidth[CSS_TOP];
    int bb = style->borderwidth[CSS_BOTTOM];
    int bl = style->borderwidth[CSS_LEFT];
    int br = style->borderwidth[CSS_RIGHT];
    int j;

    if (!radius)
    {

        fill(canvas, box->x, box->y, box->w, box->h, style->background);

        if (style->gradient)
            paintgradient(canvas, &layout->gradients[style->gradient - 1], box, 0);

        fill(canvas, box->x, box->y, box->w, bt, style->bordercolor[CSS_TOP]);
        fill(canvas, box->x, box->y + box->h - bb, box->w, bb, style->bordercolor[CSS_BOTTOM]);
        fill(canvas, box->x, box->y, bl, box->h, style->bordercolor[CSS_LEFT]);
        fill(canvas, box->x + box->w - br, box->y, br, box->h, style->bordercolor[CSS_RIGHT]);

        return;

    }

    for (j = 0; j < box->h; j++)
    {

        int inset = cornerinset(j, box->h, radius);

        fill(canvas, box->x + inset, box->y + j, box->w - inset * 2, 1, style->background);

    }

    if (style->gradient)
        paintgradient(canvas, &layout->gradients[style->gradient - 1], box, radius);

    if (bt || bb || bl || br)
    {

        int innerheight = box->h - bt - bb;
        int innerwidth = box->w - bl - br;
        int innerradius = radius - ((bt > bl) ? bt : bl);

        for (j = 0; j < box->h; j++)
        {

            int inset = cornerinset(j, box->h, radius);
            int y = box->y + j;

            if (j < bt || j >= box->h - bb || innerwidth <= 0)
            {

                fill(canvas, box->x + inset, y, box->w - inset * 2, 1, style->bordercolor[(j < bt) ? CSS_TOP : CSS_BOTTOM]);

            }

            else
            {

                int innerinset = cornerinset(j - bt, innerheight, innerradius);

                fill(canvas, box->x + inset, y, bl + innerinset - inset, 1, style->bordercolor[CSS_LEFT]);
                fill(canvas, box->x + box->w - br - innerinset, y, br + innerinset - inset, 1, style->bordercolor[CSS_RIGHT]);

            }

        }

    }

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
    int x0 = (x < canvas->clipleft) ? canvas->clipleft : x;
    int y0 = (y < canvas->cliptop) ? canvas->cliptop : y;
    int x1 = (x + w > canvas->clipright) ? canvas->clipright : x + w;
    int y1 = (y + h > canvas->clipbottom) ? canvas->clipbottom : y + h;
    int i;
    int j;

    if (w <= 0 || h <= 0)
        return;

    for (j = y0; j < y1; j++)
    {

        unsigned int *row = canvas->pixels + (j - canvas->top) * canvas->width;
        unsigned int *source = pixels + (unsigned int)(j - y) * picture->rows / (unsigned int)h * picture->columns;

        for (i = x0; i < x1; i++)
        {

            unsigned int color = source[(unsigned int)(i - x) * picture->columns / (unsigned int)w];
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

static unsigned int transform(unsigned int codepoint, unsigned int mode)
{

    unsigned int lower = (codepoint >= 'a' && codepoint <= 'z') || (codepoint >= 0xE0 && codepoint <= 0xFE && codepoint != 0xF7);
    unsigned int upper = (codepoint >= 'A' && codepoint <= 'Z') || (codepoint >= 0xC0 && codepoint <= 0xDE && codepoint != 0xD7);

    if (mode == CSS_TRANSFORM_UPPERCASE && lower)
        return codepoint - 32;

    if (mode == CSS_TRANSFORM_LOWERCASE && upper)
        return codepoint + 32;

    return codepoint;

}

static void painttext(struct paint_canvas *canvas, struct layout *layout, struct layout_fragment *fragment, unsigned int color, unsigned int mode)
{

    struct font_face *face = &layout->faces[fragment->face];
    char *text = (fragment->type == LAYOUT_FRAGMENT_TEXT) ? layout->document->text + fragment->text : layout->text + fragment->text;
    int x = fragment->x;
    unsigned int space = 0;
    unsigned int i = 0;

    if (fragment->y + fragment->h <= canvas->top || fragment->y >= canvas->top + (int)canvas->height)
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
        x += paintglyph(canvas, face, transform(codepoint, mode), x, fragment->y, fragment->scale, color);

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

static unsigned int isinside(struct html_document *document, unsigned int node, unsigned int ancestor)
{

    while (node && node != ancestor)
        node = document->nodes[node].parent;

    return node == ancestor;

}

static void paintinline(struct paint_canvas *canvas, struct layout *layout, unsigned int index, unsigned int node)
{

    struct layout_fragment *fragment = &layout->fragments[index];
    struct css_style *style = &layout->styles[node];
    struct html_document *document = layout->document;
    unsigned int first = !index || !isinside(document, layout->fragments[index - 1].node, node);
    unsigned int last = index + 1 >= layout->fragmentcount || !isinside(document, layout->fragments[index + 1].node, node);
    int bt = style->borderwidth[CSS_TOP];
    int bb = style->borderwidth[CSS_BOTTOM];
    int bl = (first) ? style->borderwidth[CSS_LEFT] : 0;
    int br = (last) ? style->borderwidth[CSS_RIGHT] : 0;
    int x = fragment->x - ((first) ? bl + padding(&style->padding[CSS_LEFT]) : 0);
    int y = fragment->y - bt - padding(&style->padding[CSS_TOP]);
    int w = fragment->x + fragment->w + ((last) ? br + padding(&style->padding[CSS_RIGHT]) : 0) - x;
    int h = fragment->y + fragment->h + bb + padding(&style->padding[CSS_BOTTOM]) - y;

    fill(canvas, x, y, w, h, style->background);
    fill(canvas, x, y, w, bt, style->bordercolor[CSS_TOP]);
    fill(canvas, x, y + h - bb, w, bb, style->bordercolor[CSS_BOTTOM]);
    fill(canvas, x, y, bl, h, style->bordercolor[CSS_LEFT]);
    fill(canvas, x + w - br, y, br, h, style->bordercolor[CSS_RIGHT]);

}

static void paintinlines(struct paint_canvas *canvas, struct layout *layout, unsigned int index)
{

    struct html_document *document = layout->document;
    struct layout_fragment *fragment = &layout->fragments[index];
    unsigned int inlines[INLINES];
    unsigned int count = 0;
    unsigned int node;

    if (fragment->type != LAYOUT_FRAGMENT_TEXT)
        return;

    for (node = document->nodes[fragment->node].parent; node && count < INLINES && document->nodes[node].type == HTML_NODE_ELEMENT && layout->styles[node].display == CSS_DISPLAY_INLINE; node = document->nodes[node].parent)
        inlines[count++] = node;

    while (count--)
    {

        if (!layout->styles[inlines[count]].hidden)
            paintinline(canvas, layout, index, inlines[count]);

    }

}

static void paintfragment(struct paint_canvas *canvas, struct layout *layout, struct layout_fragment *fragment)
{

    struct html_document *document = layout->document;
    unsigned int owner = (fragment->type == LAYOUT_FRAGMENT_TEXT) ? document->nodes[fragment->node].parent : fragment->node;
    struct css_style *style = &layout->styles[owner];

    if (style->hidden)
        return;

    if (fragment->type == LAYOUT_FRAGMENT_MARKER)
    {

        paintmarker(canvas, fragment, style->color);

        return;

    }

    painttext(canvas, layout, fragment, style->color, style->transform);

    if (style->decoration & CSS_DECORATION_UNDERLINE)
        fill(canvas, fragment->x, fragment->y + fragment->ascent + fragment->scale, fragment->w, fragment->scale, style->color);

    if (style->decoration & CSS_DECORATION_LINETHROUGH)
        fill(canvas, fragment->x, fragment->y + fragment->h / 2, fragment->w, fragment->scale, style->color);

}

static void setclip(struct paint_canvas *canvas, struct layout *layout, unsigned int node)
{

    struct html_document *document = layout->document;
    unsigned int self = node;

    canvas->clipleft = 0;
    canvas->cliptop = canvas->top;
    canvas->clipright = canvas->width;
    canvas->clipbottom = canvas->top + canvas->height;

    while (node)
    {

        struct layout_box *box = &layout->boxes[node];
        struct css_style *style = &layout->styles[node];
        unsigned int parent = document->nodes[node].parent;
        int left;
        int top;
        int right;
        int bottom;

        if (document->nodes[node].type != HTML_NODE_ELEMENT)
        {

            node = parent;

            continue;

        }

        /* a positioned box is only clipped by the boxes that contain it */
        if (style->position == CSS_POSITION_FIXED)
            parent = 0;

        if (style->position == CSS_POSITION_ABSOLUTE)
        {

            while (parent && layout->styles[parent].position == CSS_POSITION_STATIC)
                parent = document->nodes[parent].parent;

        }

        if (node == self || !style->overflow || !box->placed || node == document->html || node == document->body)
        {

            node = parent;

            continue;

        }

        left = box->x + style->borderwidth[CSS_LEFT];
        top = box->y + style->borderwidth[CSS_TOP];
        right = box->x + box->w - style->borderwidth[CSS_RIGHT];
        bottom = box->y + box->h - style->borderwidth[CSS_BOTTOM];

        if (canvas->clipleft < left)
            canvas->clipleft = left;

        if (canvas->cliptop < top)
            canvas->cliptop = top;

        if (canvas->clipright > right)
            canvas->clipright = right;

        if (canvas->clipbottom > bottom)
            canvas->clipbottom = bottom;

        node = parent;

    }

}

struct paint_area paint_getcontent(struct layout *layout, unsigned int node)
{

    struct layout_box *box = &layout->boxes[node];
    struct css_style *style = &layout->styles[node];
    struct paint_area area;

    area.x = box->x + style->borderwidth[CSS_LEFT] + padding(&style->padding[CSS_LEFT]);
    area.y = box->y + style->borderwidth[CSS_TOP] + padding(&style->padding[CSS_TOP]);
    area.w = box->w - style->borderwidth[CSS_LEFT] - style->borderwidth[CSS_RIGHT] - padding(&style->padding[CSS_LEFT]) - padding(&style->padding[CSS_RIGHT]);
    area.h = box->h - style->borderwidth[CSS_TOP] - style->borderwidth[CSS_BOTTOM] - padding(&style->padding[CSS_TOP]) - padding(&style->padding[CSS_BOTTOM]);

    return area;

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

static void paintboxes(struct paint_canvas *canvas, struct layout *layout, unsigned int layer)
{

    struct html_document *document = layout->document;
    unsigned int root = (layout->layers[layer]) ? layout->layers[layer] : document->html;
    unsigned int node = root;

    while (node)
    {

        struct layout_box *box = &layout->boxes[node];
        struct css_style *style = &layout->styles[node];

        if (document->nodes[node].type == HTML_NODE_ELEMENT && layout->layer[node] == layer && box->placed && !style->hidden)
        {

            setclip(canvas, layout, node);

            if (style->display != CSS_DISPLAY_INLINE)
                paintbox(canvas, layout, box, style);

            if (document->nodes[node].tag == HTML_TAG_IMG || document->nodes[node].tag == HTML_TAG_SVG)
            {

                struct layout_picture *picture = (layout->picture[node]) ? &layout->pictures[layout->picture[node] - 1] : 0;
                struct paint_area area = paint_getcontent(layout, node);

                if (picture && picture->columns && canvas->heap)
                    paintpicture(canvas, picture, area.x, area.y, area.w, area.h);
                else if (document->nodes[node].tag == HTML_TAG_IMG)
                    paintplaceholder(canvas, box);

            }

        }

        node = html_next(document, node, root);

    }

}

void paint_document(struct paint_canvas *canvas, struct layout *layout)
{

    unsigned int i;

    setclip(canvas, layout, 0);
    fill(canvas, 0, canvas->top, canvas->width, canvas->height, paint_getbackground(layout));

    /* each layer is drawn whole, boxes then inline backgrounds then text, so a box on top covers the text below it */
    for (i = 0; i < layout->layercount; i++)
    {

        unsigned int j;

        paintboxes(canvas, layout, i);

        for (j = layout->layerstart[i]; j < layout->layerstart[i + 1]; j++)
        {

            setclip(canvas, layout, layout->fragments[layout->layerfragments[j]].node);
            paintinlines(canvas, layout, layout->layerfragments[j]);

        }

        for (j = layout->layerstart[i]; j < layout->layerstart[i + 1]; j++)
        {

            setclip(canvas, layout, layout->fragments[layout->layerfragments[j]].node);
            paintfragment(canvas, layout, &layout->fragments[layout->layerfragments[j]]);

        }

    }

}
