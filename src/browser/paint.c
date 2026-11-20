#include <fudge.h>
#include "html.h"
#include "css.h"
#include "font.h"
#include "layout.h"
#include "paint.h"

#define WHITE                           0xFFFFFFFF
#define PLACEHOLDER                     0xFFC0C0C0

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

static void paintbox(struct paint_canvas *canvas, struct layout_box *box, struct css_style *style)
{

    fill(canvas, box->x, box->y, box->w, box->h, style->background);
    fill(canvas, box->x, box->y, box->w, style->borderwidth[CSS_TOP], style->bordercolor[CSS_TOP]);
    fill(canvas, box->x, box->y + box->h - style->borderwidth[CSS_BOTTOM], box->w, style->borderwidth[CSS_BOTTOM], style->bordercolor[CSS_BOTTOM]);
    fill(canvas, box->x, box->y, style->borderwidth[CSS_LEFT], box->h, style->bordercolor[CSS_LEFT]);
    fill(canvas, box->x + box->w - style->borderwidth[CSS_RIGHT], box->y, style->borderwidth[CSS_RIGHT], box->h, style->bordercolor[CSS_RIGHT]);

}

static void paintplaceholder(struct paint_canvas *canvas, struct layout_box *box)
{

    fill(canvas, box->x, box->y, box->w, 1, PLACEHOLDER);
    fill(canvas, box->x, box->y + box->h - 1, box->w, 1, PLACEHOLDER);
    fill(canvas, box->x, box->y, 1, box->h, PLACEHOLDER);
    fill(canvas, box->x + box->w - 1, box->y, 1, box->h, PLACEHOLDER);

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
                paintbox(canvas, box, style);

            if (document->nodes[node].tag == HTML_TAG_IMG)
                paintplaceholder(canvas, box);

        }

        node = html_next(document, node, document->html);

    }

    for (i = 0; i < layout->fragmentcount; i++)
        paintfragment(canvas, layout, &layout->fragments[i]);

}
