#include <fudge.h>
#include "html.h"
#include "css.h"
#include <font.h>
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

static unsigned int fade(unsigned int color, unsigned int opacity)
{

    return (opacity == 0xFF) ? color : ((((color >> 24) * opacity / 0xFF) << 24) | (color & 0xFFFFFF));

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

static int max(int a, int b)
{

    return (a > b) ? a : b;

}

struct corners
{

    int rx[CSS_CORNERS];
    int ry[CSS_CORNERS];

};

struct insets
{

    int left;
    int right;

};

static int arcinset(int distance, int rx, int ry)
{

    int d;

    if (rx <= 0 || ry <= 0 || distance >= ry || distance < 0)
        return 0;

    d = ry * 2 - distance * 2 - 1;

    return rx - rx * root(ry * ry * 4 - d * d) / (ry * 2);

}

static struct insets rowinsets(struct corners *corners, int row, int height)
{

    struct insets insets;

    insets.left = max(arcinset(row, corners->rx[CSS_TOPLEFT], corners->ry[CSS_TOPLEFT]), arcinset(height - 1 - row, corners->rx[CSS_BOTTOMLEFT], corners->ry[CSS_BOTTOMLEFT]));
    insets.right = max(arcinset(row, corners->rx[CSS_TOPRIGHT], corners->ry[CSS_TOPRIGHT]), arcinset(height - 1 - row, corners->rx[CSS_BOTTOMRIGHT], corners->ry[CSS_BOTTOMRIGHT]));

    return insets;

}

static int fitscale(int scale, int side, int a, int b)
{

    return (a + b > side && a + b > 0 && side * 1000 / (a + b) < scale) ? side * 1000 / (a + b) : scale;

}

static struct corners getcorners(struct css_style *style, struct layout_box *box)
{

    struct corners corners;
    int scale = 1000;
    unsigned int i;

    /* a percentage is of the width across and of the height down, so half of each makes an ellipse */
    for (i = 0; i < CSS_CORNERS; i++)
    {

        struct css_length *length = &style->radius[i];

        corners.rx[i] = (length->unit == CSS_UNIT_PERCENT) ? box->w * (length->value / 10) / 10000 : (length->unit == CSS_UNIT_PX) ? length->value : 0;
        corners.ry[i] = (length->unit == CSS_UNIT_PERCENT) ? box->h * (length->value / 10) / 10000 : corners.rx[i];
        corners.rx[i] = max(corners.rx[i], 0);
        corners.ry[i] = max(corners.ry[i], 0);

    }

    /* corners that would overlap along a side all shrink by the same factor */
    scale = fitscale(scale, box->w, corners.rx[CSS_TOPLEFT], corners.rx[CSS_TOPRIGHT]);
    scale = fitscale(scale, box->w, corners.rx[CSS_BOTTOMLEFT], corners.rx[CSS_BOTTOMRIGHT]);
    scale = fitscale(scale, box->h, corners.ry[CSS_TOPLEFT], corners.ry[CSS_BOTTOMLEFT]);
    scale = fitscale(scale, box->h, corners.ry[CSS_TOPRIGHT], corners.ry[CSS_BOTTOMRIGHT]);

    for (i = 0; i < CSS_CORNERS; i++)
    {

        corners.rx[i] = corners.rx[i] * scale / 1000;
        corners.ry[i] = corners.ry[i] * scale / 1000;

    }

    return corners;

}

static struct corners growcorners(struct corners *corners, int amount)
{

    struct corners grown;
    unsigned int i;

    /* a corner with no rounding stays sharp when the box it follows grows or shrinks */
    for (i = 0; i < CSS_CORNERS; i++)
    {

        grown.rx[i] = (corners->rx[i]) ? max(corners->rx[i] + amount, 0) : 0;
        grown.ry[i] = (corners->ry[i]) ? max(corners->ry[i] + amount, 0) : 0;

    }

    return grown;

}

static unsigned int isrounded(struct corners *corners)
{

    return corners->rx[0] || corners->rx[1] || corners->rx[2] || corners->rx[3];

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

struct shade
{

    struct css_gradient *gradient;
    unsigned int opacity;
    int sinus;
    int cosinus;
    int length;
    int cx;
    int cy;
    int rx;
    int ry;

};

static void initshade(struct shade *shade, struct css_gradient *gradient, struct layout_box *box, unsigned int opacity)
{

    shade->gradient = gradient;
    shade->opacity = opacity;
    shade->sinus = sine(gradient->angle);
    shade->cosinus = sine(gradient->angle + 90);
    shade->length = (box->w * ((shade->sinus < 0) ? -shade->sinus : shade->sinus) + box->h * ((shade->cosinus < 0) ? -shade->cosinus : shade->cosinus)) / 1000;
    shade->cx = (gradient->radial) ? box->x + box->w * gradient->x / 1000 : box->x + box->w / 2;
    shade->cy = (gradient->radial) ? box->y + box->h * gradient->y / 1000 : box->y + box->h / 2;
    shade->rx = box->w * gradient->width / 1000;
    shade->ry = box->h * gradient->height / 1000;

}

static unsigned int shadeat(struct shade *shade, int i, int j)
{

    int t;

    if (shade->gradient->radial)
    {

        int dx = (shade->rx > 0) ? (i - shade->cx) * 1000 / shade->rx : 0;
        int dy = (shade->ry > 0) ? (j - shade->cy) * 1000 / shade->ry : 0;

        dx = (dx > LIMIT) ? LIMIT : (dx < -LIMIT) ? -LIMIT : dx;
        dy = (dy > LIMIT) ? LIMIT : (dy < -LIMIT) ? -LIMIT : dy;

        t = root(dx * dx + dy * dy);

    }

    else
    {

        t = (shade->length > 0) ? ((i - shade->cx) * shade->sinus - (j - shade->cy) * shade->cosinus) / shade->length + 500 : 500;

    }

    return fade(stopcolor(shade->gradient, t), shade->opacity);

}

static void paintgradient(struct paint_canvas *canvas, struct css_gradient *gradient, struct layout_box *box, struct corners *corners, unsigned int opacity)
{

    int x0 = (box->x < canvas->clipleft) ? canvas->clipleft : box->x;
    int y0 = (box->y < canvas->cliptop) ? canvas->cliptop : box->y;
    int x1 = (box->x + box->w > canvas->clipright) ? canvas->clipright : box->x + box->w;
    int y1 = (box->y + box->h > canvas->clipbottom) ? canvas->clipbottom : box->y + box->h;
    struct shade shade;
    int i;
    int j;

    if (box->w <= 0 || box->h <= 0)
        return;

    initshade(&shade, gradient, box, opacity);

    for (j = y0; j < y1; j++)
    {

        unsigned int *row = canvas->pixels + (j - canvas->top) * canvas->width;
        struct insets insets = rowinsets(corners, j - box->y, box->h);
        int start = (box->x + insets.left > x0) ? box->x + insets.left : x0;
        int end = (box->x + box->w - insets.right < x1) ? box->x + box->w - insets.right : x1;

        for (i = start; i < end; i++)
        {

            unsigned int color = shadeat(&shade, i, j);

            if ((color >> 24) == 0xFF)
                row[i] = color;
            else if (color >> 24)
                row[i] = blend(row[i], color);

        }

    }

}

static unsigned int ismitred(struct css_style *style)
{

    unsigned int side;

    for (side = 0; side < CSS_SIDES; side++)
    {

        unsigned int next = (side + 1) % CSS_SIDES;

        if (style->borderwidth[side] && style->borderwidth[next] && style->bordercolor[side] != style->bordercolor[next])
            return 1;

    }

    return 0;

}

static void paintmitred(struct paint_canvas *canvas, struct layout_box *box, struct css_style *style)
{

    int bt = style->borderwidth[CSS_TOP];
    int bb = style->borderwidth[CSS_BOTTOM];
    int bl = style->borderwidth[CSS_LEFT];
    int br = style->borderwidth[CSS_RIGHT];
    unsigned int top = fade(style->bordercolor[CSS_TOP], style->opacity);
    unsigned int bottom = fade(style->bordercolor[CSS_BOTTOM], style->opacity);
    unsigned int left = fade(style->bordercolor[CSS_LEFT], style->opacity);
    unsigned int right = fade(style->bordercolor[CSS_RIGHT], style->opacity);
    int j;

    /* corners are split along the line from the outer to the inner corner, which is what draws css triangles */
    for (j = 0; j < box->h; j++)
    {

        int y = box->y + j;

        if (j < bt || j >= box->h - bb)
        {

            unsigned int band = (j < bt);
            int depth = (band) ? j * 2 + 1 : (box->h - 1 - j) * 2 + 1;
            int width = (band) ? bt * 2 : bb * 2;
            int xl = bl * depth / width;
            int xr = br * depth / width;

            fill(canvas, box->x, y, xl, 1, left);
            fill(canvas, box->x + xl, y, box->w - xl - xr, 1, (band) ? top : bottom);
            fill(canvas, box->x + box->w - xr, y, xr, 1, right);

        }

        else
        {

            fill(canvas, box->x, y, bl, 1, left);
            fill(canvas, box->x + box->w - br, y, br, 1, right);

        }

    }

}

static int shadowramp(int depth, int blur)
{

    int value;

    if (!blur)
        return (depth >= 0) ? 0xFF : 0;

    value = (depth * 2 + 1 + blur * 2) * 0xFF / (blur * 4);

    return (value < 0) ? 0 : (value > 0xFF) ? 0xFF : value;

}

static void paintshadow(struct paint_canvas *canvas, struct layout_box *box, struct css_style *style)
{

    unsigned int color = fade(style->shadow, style->opacity);
    int blur = style->shadowblur;
    struct corners corners = getcorners(style, box);
    struct corners outer = growcorners(&corners, style->shadowspread);
    int left = box->x + style->shadowx - style->shadowspread;
    int top = box->y + style->shadowy - style->shadowspread;
    int right = box->x + box->w + style->shadowx + style->shadowspread;
    int bottom = box->y + box->h + style->shadowy + style->shadowspread;
    int x0 = (left - blur < canvas->clipleft) ? canvas->clipleft : left - blur;
    int y0 = (top - blur < canvas->cliptop) ? canvas->cliptop : top - blur;
    int x1 = (right + blur > canvas->clipright) ? canvas->clipright : right + blur;
    int y1 = (bottom + blur > canvas->clipbottom) ? canvas->clipbottom : bottom + blur;
    int i;
    int j;

    if (!(color >> 24) || right <= left || bottom <= top)
        return;

    /* the blur is a ramp across twice its radius on each edge, and the box itself covers the inside */
    for (j = y0; j < y1; j++)
    {

        unsigned int *row = canvas->pixels + (j - canvas->top) * canvas->width;
        int vertical = shadowramp(j - top, blur) * shadowramp(bottom - 1 - j, blur) / 0xFF;
        unsigned int inside = j >= box->y && j < box->y + box->h;
        struct insets inset = rowinsets(&corners, j - box->y, box->h);
        int edge = (j < top) ? 0 : (j >= bottom) ? bottom - top - 1 : j - top;
        struct insets round = rowinsets(&outer, edge, bottom - top);

        if (!vertical)
            continue;

        for (i = x0; i < x1; i++)
        {

            unsigned int alpha;

            if (inside && i >= box->x + inset.left && i < box->x + box->w - inset.right)
                continue;

            alpha = (color >> 24) * (vertical * shadowramp(i - left - round.left, blur) / 0xFF * shadowramp(right - round.right - 1 - i, blur) / 0xFF) / 0xFF;

            if (alpha)
                row[i] = blend(row[i], (alpha << 24) | (color & 0xFFFFFF));

        }

    }

}

static void paintbox(struct paint_canvas *canvas, struct layout *layout, struct layout_box *box, struct css_style *style, unsigned int backdrop)
{

    struct corners corners = getcorners(style, box);
    int bt = style->borderwidth[CSS_TOP];
    int bb = style->borderwidth[CSS_BOTTOM];
    int bl = style->borderwidth[CSS_LEFT];
    int br = style->borderwidth[CSS_RIGHT];
    unsigned int background = style->background;
    int j;

    if (style->textclip || !backdrop)
        background = 0;

    if (!isrounded(&corners))
    {

        fill(canvas, box->x, box->y, box->w, box->h, fade(background, style->opacity));

        if (style->gradient && !style->textclip && backdrop)
            paintgradient(canvas, &layout->gradients[style->gradient - 1], box, &corners, style->opacity);

        if (ismitred(style))
        {

            paintmitred(canvas, box, style);

            return;

        }

        fill(canvas, box->x, box->y, box->w, bt, fade(style->bordercolor[CSS_TOP], style->opacity));
        fill(canvas, box->x, box->y + box->h - bb, box->w, bb, fade(style->bordercolor[CSS_BOTTOM], style->opacity));
        fill(canvas, box->x, box->y, bl, box->h, fade(style->bordercolor[CSS_LEFT], style->opacity));
        fill(canvas, box->x + box->w - br, box->y, br, box->h, fade(style->bordercolor[CSS_RIGHT], style->opacity));

        return;

    }

    for (j = 0; j < box->h; j++)
    {

        struct insets insets = rowinsets(&corners, j, box->h);

        fill(canvas, box->x + insets.left, box->y + j, box->w - insets.left - insets.right, 1, fade(background, style->opacity));

    }

    if (style->gradient && !style->textclip && backdrop)
        paintgradient(canvas, &layout->gradients[style->gradient - 1], box, &corners, style->opacity);

    if (bt || bb || bl || br)
    {

        struct corners inner;
        int innerheight = box->h - bt - bb;
        int innerwidth = box->w - bl - br;

        /* an inner corner is its outer one less the border on each side, so unequal borders make it oval */
        inner.rx[CSS_TOPLEFT] = max(corners.rx[CSS_TOPLEFT] - bl, 0);
        inner.ry[CSS_TOPLEFT] = max(corners.ry[CSS_TOPLEFT] - bt, 0);
        inner.rx[CSS_TOPRIGHT] = max(corners.rx[CSS_TOPRIGHT] - br, 0);
        inner.ry[CSS_TOPRIGHT] = max(corners.ry[CSS_TOPRIGHT] - bt, 0);
        inner.rx[CSS_BOTTOMRIGHT] = max(corners.rx[CSS_BOTTOMRIGHT] - br, 0);
        inner.ry[CSS_BOTTOMRIGHT] = max(corners.ry[CSS_BOTTOMRIGHT] - bb, 0);
        inner.rx[CSS_BOTTOMLEFT] = max(corners.rx[CSS_BOTTOMLEFT] - bl, 0);
        inner.ry[CSS_BOTTOMLEFT] = max(corners.ry[CSS_BOTTOMLEFT] - bb, 0);

        for (j = 0; j < box->h; j++)
        {

            struct insets outer = rowinsets(&corners, j, box->h);
            int y = box->y + j;

            if (j < bt || j >= box->h - bb || innerwidth <= 0)
            {

                fill(canvas, box->x + outer.left, y, box->w - outer.left - outer.right, 1, fade(style->bordercolor[(j < bt) ? CSS_TOP : CSS_BOTTOM], style->opacity));

            }

            else
            {

                struct insets insets = rowinsets(&inner, j - bt, innerheight);

                fill(canvas, box->x + outer.left, y, bl + insets.left - outer.left, 1, fade(style->bordercolor[CSS_LEFT], style->opacity));
                fill(canvas, box->x + box->w - br - insets.right, y, br + insets.right - outer.right, 1, fade(style->bordercolor[CSS_RIGHT], style->opacity));

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

static void paintpicture(struct paint_canvas *canvas, struct layout_picture *picture, int x, int y, int w, int h, unsigned int opacity)
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

            unsigned int color = fade(source[(unsigned int)(i - x) * picture->columns / (unsigned int)w], opacity);
            unsigned int alpha = color >> 24;

            if (alpha == 0xFF)
                row[i] = color;
            else if (alpha)
                row[i] = blend(row[i], color);

        }

    }

}

static void paintglyph(struct paint_canvas *canvas, struct font_set *fonts, struct font_glyph *glyph, int x, int y, unsigned int color, struct shade *shade)
{

    unsigned char *coverage = font_getcoverage(fonts, glyph);
    int left = x + glyph->left;
    int top = y + glyph->top;
    int x0 = (left < canvas->clipleft) ? canvas->clipleft : left;
    int y0 = (top < canvas->cliptop) ? canvas->cliptop : top;
    int x1 = (left + (int)glyph->width > canvas->clipright) ? canvas->clipright : left + (int)glyph->width;
    int y1 = (top + (int)glyph->height > canvas->clipbottom) ? canvas->clipbottom : top + (int)glyph->height;
    int i;
    int j;

    for (j = y0; j < y1; j++)
    {

        unsigned int *row = canvas->pixels + (j - canvas->top) * canvas->width;
        unsigned char *source = coverage + (j - top) * glyph->width - left;

        for (i = x0; i < x1; i++)
        {

            unsigned int ink = (shade) ? shadeat(shade, i, j) : color;
            unsigned int alpha = (ink >> 24) * source[i] / 255;

            if (alpha)
                row[i] = blend(row[i], (alpha << 24) | (ink & 0xFFFFFF));

        }

    }

}

static void painttext(struct paint_canvas *canvas, struct layout *layout, struct layout_fragment *fragment, struct css_style *style, unsigned int color, struct shade *shade)
{

    struct font_choice choice;
    char *text = (fragment->type == LAYOUT_FRAGMENT_TEXT) ? layout->document->text + fragment->text : layout->text + fragment->text;
    int baseline = fragment->y + fragment->ascent;
    int pen = fragment->x * 64;
    unsigned int previous = 0;
    unsigned int space = 0;
    unsigned int i = 0;

    if (fragment->y + fragment->h <= canvas->top || fragment->y >= canvas->top + (int)canvas->height)
        return;

    choice.face = fragment->face;
    choice.size = fragment->size;

    /* glyphs advance in 1/64 pixels from the left of the piece, the same sums layout measured */
    while (i < fragment->length)
    {

        unsigned int codepoint;
        struct font_glyph *glyph;

        if (fragment->collapse && (text[i] == ' ' || text[i] == '\n' || text[i] == '\t' || text[i] == '\r' || text[i] == '\f'))
        {

            if (!space)
                pen += font_getglyph(layout->fonts, choice, ' ')->advance + style->letterspacing * 64;

            previous = 0;
            space = 1;
            i++;

            continue;

        }

        space = 0;
        i += utf8_read(text + i, fragment->length - i, &codepoint);
        glyph = font_render(layout->fonts, choice, font_setcase(codepoint, fragment->casing));
        pen += font_getkerning(layout->fonts, choice, previous, glyph->id);

        paintglyph(canvas, layout->fonts, glyph, (pen + 32) / 64, baseline, color, shade);

        pen += glyph->advance + style->letterspacing * 64;
        previous = (codepoint == ' ') ? 0 : glyph->id;

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

    fill(canvas, x, y, w, h, (style->textclip) ? 0 : fade(style->background, style->opacity));
    fill(canvas, x, y, w, bt, fade(style->bordercolor[CSS_TOP], style->opacity));
    fill(canvas, x, y + h - bb, w, bb, fade(style->bordercolor[CSS_BOTTOM], style->opacity));
    fill(canvas, x, y, bl, h, fade(style->bordercolor[CSS_LEFT], style->opacity));
    fill(canvas, x + w - br, y, br, h, fade(style->bordercolor[CSS_RIGHT], style->opacity));

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
    unsigned int color = fade(style->color, style->opacity);
    int thickness = (fragment->size + 8) / 16;
    unsigned int clip = owner;
    struct shade shade;

    if (style->hidden)
        return;

    /* text clipped to a background is drawn in that background */
    while (clip && document->nodes[clip].type == HTML_NODE_ELEMENT && !layout->styles[clip].textclip && layout->styles[clip].display == CSS_DISPLAY_INLINE)
        clip = document->nodes[clip].parent;

    if (clip && layout->styles[clip].textclip)
    {

        struct css_style *clipstyle = &layout->styles[clip];

        if (clipstyle->gradient && layout->boxes[clip].placed && fragment->type == LAYOUT_FRAGMENT_TEXT)
        {

            initshade(&shade, &layout->gradients[clipstyle->gradient - 1], &layout->boxes[clip], style->opacity);
            painttext(canvas, layout, fragment, style, color, &shade);

            return;

        }

        color = fade(clipstyle->background, style->opacity);

    }

    if (fragment->type == LAYOUT_FRAGMENT_MARKER)
    {

        paintmarker(canvas, fragment, color);

        return;

    }

    painttext(canvas, layout, fragment, style, color, 0);

    if (style->decoration & CSS_DECORATION_UNDERLINE)
        fill(canvas, fragment->x, fragment->y + fragment->ascent + thickness, fragment->w, thickness, color);

    if (style->decoration & CSS_DECORATION_LINETHROUGH)
        fill(canvas, fragment->x, fragment->y + fragment->ascent - fragment->size * 3 / 10, fragment->w, thickness, color);

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

static unsigned int hasbackground(struct css_style *style)
{

    return (style->background >> 24) || (style->gradient && !style->textclip);

}

static unsigned int canvasnode(struct layout *layout)
{

    struct html_document *document = layout->document;

    /* the root takes its background from the body when it has none of its own */
    if (document->html && hasbackground(&layout->styles[document->html]))
        return document->html;

    if (document->body && hasbackground(&layout->styles[document->body]))
        return document->body;

    return 0;

}

unsigned int paint_getbackground(struct layout *layout)
{

    unsigned int node = canvasnode(layout);
    unsigned int color = (node) ? layout->styles[node].background : 0;

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

            if (style->display != CSS_DISPLAY_INLINE && style->shadow)
                paintshadow(canvas, box, style);

            if (style->display != CSS_DISPLAY_INLINE)
                paintbox(canvas, layout, box, style, node != canvasnode(layout));

            if (document->nodes[node].tag == HTML_TAG_IMG || document->nodes[node].tag == HTML_TAG_SVG)
            {

                struct layout_picture *picture = (layout->picture[node]) ? &layout->pictures[layout->picture[node] - 1] : 0;
                struct paint_area area = paint_getcontent(layout, node);

                if (picture && picture->columns && canvas->heap)
                    paintpicture(canvas, picture, area.x, area.y, area.w, area.h, style->opacity);
                else if (document->nodes[node].tag == HTML_TAG_IMG)
                    paintplaceholder(canvas, box);

            }

        }

        node = html_next(document, node, root);

    }

}

void paint_document(struct paint_canvas *canvas, struct layout *layout)
{

    unsigned int node = canvasnode(layout);
    unsigned int i;

    setclip(canvas, layout, 0);
    fill(canvas, 0, canvas->top, canvas->width, canvas->height, paint_getbackground(layout));

    /* the canvas background covers at least the screen, also below a short page */
    if (node && layout->styles[node].gradient && layout->document->html)
    {

        struct layout_box *html = &layout->boxes[layout->document->html];
        struct layout_box area;
        struct corners square;

        buffer_clear(&square, sizeof (struct corners));

        area.x = html->x;
        area.y = html->y;
        area.w = html->w;
        area.h = (html->h > layout->viewheight) ? html->h : layout->viewheight;

        paintgradient(canvas, &layout->gradients[layout->styles[node].gradient - 1], &area, &square, layout->styles[node].opacity);

    }

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

