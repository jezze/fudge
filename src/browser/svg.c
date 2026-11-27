#include <fudge.h>
#include <compression.h>
#include <image.h>
#include "html.h"
#include "css.h"
#include "svg.h"

#define DEPTH                           16
#define PROPERTIES                      11
#define STYLES                          8
#define PAINT_NONE                      0
#define PAINT_COLOR                     1
#define PAINT_CURRENT                   2

struct paint
{

    unsigned int kind;
    unsigned int color;

};

struct state
{

    struct svg_matrix matrix;
    struct paint fill;
    struct paint stroke;
    int strokewidth;
    unsigned int color;
    unsigned int fillopacity;
    unsigned int strokeopacity;
    unsigned int opacity;
    unsigned int evenodd;
    unsigned int cap;
    unsigned int display;
    unsigned int visible;

};

struct render
{

    struct svg svg;
    struct html_document *document;
    struct css_sheet *sheet;
    unsigned int node;
    unsigned int depth;
    int viewwidth;
    int viewheight;
    char *styles[STYLES];
    unsigned int stylelengths[STYLES];
    unsigned int stylecount;

};

struct viewport
{

    struct svg_matrix matrix;
    int width;
    int height;

};

static char *properties[PROPERTIES] = {"fill", "stroke", "stroke-width", "fill-opacity", "stroke-opacity", "opacity", "fill-rule", "stroke-linecap", "color", "display", "visibility"};
static struct render raster;

static struct svg_number number(int value, unsigned int valid)
{

    struct svg_number result;

    result.value = value;
    result.valid = valid;

    return result;

}

static struct svg_number readlength(char *text, int base)
{

    struct svg_reader reader;
    struct svg_number result;

    if (!text)
        return number(0, 0);

    reader.text = text;
    reader.offset = 0;
    result = svg_readnumber(&reader);

    if (result.valid && text[reader.offset] == '%')
        result.value = svg_mulfix(base, result.value / 100);

    return result;

}

static int getlength(struct render *render, unsigned int node, char *name, int base, int fallback)
{

    struct svg_number result = readlength(html_getattribute(render->document, node, name), base);

    return (result.valid) ? result.value : fallback;

}

static unsigned int matchname(struct render *render, unsigned int node, char *name)
{

    return cstring_match(html_getstring(render->document, render->document->nodes[node].name), name);

}

static unsigned int findid(struct render *render, char *id, unsigned int length)
{

    unsigned int node;

    for (node = 1; node < render->document->nodecount; node++)
    {

        char *value = (render->document->nodes[node].type == HTML_NODE_ELEMENT) ? html_getattribute(render->document, node, "id") : 0;

        if (value && cstring_length(value) == length && buffer_match(value, id, length))
            return node;

    }

    return 0;

}

static unsigned int parsecolor(char *text, unsigned int length, unsigned int current)
{

    struct css_color color = css_parsecolor(text, length);

    if (!color.valid)
        return 0;

    return (color.value == CSS_CURRENTCOLOR) ? current : color.value;

}

static unsigned int findstopcolor(struct render *render, unsigned int node, unsigned int current);

static unsigned int gradientcolor(struct render *render, unsigned int node, unsigned int current, unsigned int depth)
{

    unsigned int red = 0;
    unsigned int green = 0;
    unsigned int blue = 0;
    unsigned int alpha = 0;
    unsigned int count = 0;
    unsigned int child;
    char *href;

    for (child = render->document->nodes[node].first; child; child = render->document->nodes[child].next)
    {

        if (render->document->nodes[child].type == HTML_NODE_ELEMENT && matchname(render, child, "stop"))
        {

            unsigned int color = findstopcolor(render, child, current);

            red += (color >> 16) & 0xFF;
            green += (color >> 8) & 0xFF;
            blue += color & 0xFF;
            alpha += color >> 24;
            count++;

        }

    }

    if (count)
        return ((alpha / count) << 24) | ((red / count) << 16) | ((green / count) << 8) | (blue / count);

    href = html_getattribute(render->document, node, "href");

    if (!href)
        href = html_getattribute(render->document, node, "xlink:href");

    if (href && href[0] == '#' && depth < 4)
    {

        unsigned int target = findid(render, href + 1, cstring_length(href + 1));

        if (target)
            return gradientcolor(render, target, current, depth + 1);

    }

    return 0;

}

static void parsepaint(struct render *render, struct state *state, struct paint *paint, char *text, unsigned int length)
{

    while (length && text[0] == ' ')
    {

        text++;
        length--;

    }

    while (length && text[length - 1] == ' ')
        length--;

    if (length == 7 && buffer_match(text, "inherit", 7))
        return;

    if (length == 4 && buffer_match(text, "none", 4))
    {

        paint->kind = PAINT_NONE;

        return;

    }

    /* custom properties come from the page's css, with the fallback after the comma when there is none */
    if (length > 6 && buffer_match(text, "var(", 4) && render->sheet && render->depth < DEPTH)
    {

        unsigned int close = buffer_findbyte(text, length, ')');
        unsigned int comma = buffer_findbyte(text, close, ',');
        char *custom = css_findcustom(render->sheet, render->document, render->node, text + 4, comma - 4);

        render->depth++;

        if (custom)
            parsepaint(render, state, paint, custom, cstring_length(custom));
        else if (comma < close)
            parsepaint(render, state, paint, text + comma + 1, close - comma - 1);

        render->depth--;

        return;

    }

    if (length > 5 && buffer_match(text, "url(#", 5))
    {

        unsigned int close = buffer_findbyte(text, length, ')');
        unsigned int target = (close < length) ? findid(render, text + 5, close - 5) : 0;

        if (target)
        {

            paint->kind = PAINT_COLOR;
            paint->color = gradientcolor(render, target, state->color, 0);

        }

        else if (close + 1 < length)
        {

            parsepaint(render, state, paint, text + close + 1, length - close - 1);

        }

        else
        {

            paint->kind = PAINT_NONE;

        }

        return;

    }

    {

        struct css_color color = css_parsecolor(text, length);

        if (color.valid)
        {

            paint->kind = (color.value == CSS_CURRENTCOLOR) ? PAINT_CURRENT : PAINT_COLOR;
            paint->color = color.value;

        }

    }

}

static unsigned int readopacity(char *text, unsigned int length)
{

    struct svg_reader reader;
    struct svg_number result;
    char buffer[32];

    if (length >= 32)
        return 255;

    buffer_copy(buffer, text, length);

    buffer[length] = '\0';
    reader.text = buffer;
    reader.offset = 0;
    result = svg_readnumber(&reader);

    if (!result.valid)
        return 255;

    if (buffer[reader.offset] == '%')
        result.value /= 100;

    return (result.value <= 0) ? 0 : (result.value >= SVG_ONE) ? 255 : (unsigned int)(result.value * 255 / SVG_ONE);

}

static unsigned int matchvalue(char *text, unsigned int length, char *word)
{

    return cstring_length(word) == length && buffer_match(text, word, length);

}

static void apply(struct render *render, struct state *state, char *name, unsigned int namelength, char *value, unsigned int length)
{

    while (length && (value[0] == ' ' || value[0] == '\n' || value[0] == '\t'))
    {

        value++;
        length--;

    }

    while (length && (value[length - 1] == ' ' || value[length - 1] == '\n' || value[length - 1] == '\t'))
        length--;

    if (matchvalue(name, namelength, "fill"))
    {

        parsepaint(render, state, &state->fill, value, length);

    }

    else if (matchvalue(name, namelength, "stroke"))
    {

        parsepaint(render, state, &state->stroke, value, length);

    }

    else if (matchvalue(name, namelength, "stroke-width"))
    {

        char buffer[32];
        struct svg_reader reader;
        struct svg_number width;

        if (length >= 32)
            return;

        buffer_copy(buffer, value, length);

        buffer[length] = '\0';
        reader.text = buffer;
        reader.offset = 0;
        width = svg_readnumber(&reader);

        if (width.valid && width.value >= 0)
            state->strokewidth = width.value;

    }

    else if (matchvalue(name, namelength, "fill-opacity"))
    {

        state->fillopacity = readopacity(value, length);

    }

    else if (matchvalue(name, namelength, "stroke-opacity"))
    {

        state->strokeopacity = readopacity(value, length);

    }

    else if (matchvalue(name, namelength, "opacity"))
    {

        state->opacity = state->opacity * readopacity(value, length) / 255;

    }

    else if (matchvalue(name, namelength, "fill-rule"))
    {

        state->evenodd = matchvalue(value, length, "evenodd");

    }

    else if (matchvalue(name, namelength, "stroke-linecap"))
    {

        state->cap = (matchvalue(value, length, "round")) ? SVG_CAP_ROUND : (matchvalue(value, length, "square")) ? SVG_CAP_SQUARE : SVG_CAP_BUTT;

    }

    else if (matchvalue(name, namelength, "color"))
    {

        unsigned int color = parsecolor(value, length, state->color);

        if (color)
            state->color = color;

    }

    else if (matchvalue(name, namelength, "display"))
    {

        state->display = !matchvalue(value, length, "none");

    }

    else if (matchvalue(name, namelength, "visibility"))
    {

        state->visible = matchvalue(value, length, "visible");

    }

}

static void applystyle(struct render *render, struct state *state, char *text, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < length)
    {

        unsigned int end = offset + buffer_findbyte(text + offset, length - offset, ';');
        unsigned int colon = offset + buffer_findbyte(text + offset, end - offset, ':');
        unsigned int namestart = offset;
        unsigned int nameend = colon;

        while (namestart < nameend && (text[namestart] == ' ' || text[namestart] == '\n' || text[namestart] == '\t'))
            namestart++;

        while (nameend > namestart && (text[nameend - 1] == ' ' || text[nameend - 1] == '\n' || text[nameend - 1] == '\t'))
            nameend--;

        if (colon < end)
            apply(render, state, text + namestart, nameend - namestart, text + colon + 1, end - colon - 1);

        offset = end + 1;

    }

}

static unsigned int isnamechar(char c)
{

    return cstring_isalpha(c) || cstring_isdigit(c) || c == '-' || c == '_';

}

static unsigned int hasclass(char *selector, unsigned int length, char *name, unsigned int namelength)
{

    unsigned int i;

    for (i = 0; i + namelength < length; i++)
    {

        if (selector[i] == '.' && buffer_match(selector + i + 1, name, namelength) && (i + 1 + namelength == length || !isnamechar(selector[i + 1 + namelength])))
            return 1;

    }

    return 0;

}

static void applyclass(struct render *render, struct state *state, char *name, unsigned int namelength)
{

    unsigned int i;

    for (i = 0; i < render->stylecount; i++)
    {

        char *text = render->styles[i];
        unsigned int length = render->stylelengths[i];
        unsigned int offset = 0;

        while (offset < length)
        {

            unsigned int open = offset + buffer_findbyte(text + offset, length - offset, '{');
            unsigned int close = (open < length) ? open + buffer_findbyte(text + open, length - open, '}') : length;

            if (open >= length)
                break;

            if (hasclass(text + offset, open - offset, name, namelength))
                applystyle(render, state, text + open + 1, close - open - 1);

            offset = close + 1;

        }

    }

}

static void applyclasses(struct render *render, struct state *state, char *classes)
{

    unsigned int offset = 0;

    while (classes[offset])
    {

        unsigned int start;

        while (classes[offset] == ' ')
            offset++;

        start = offset;

        while (classes[offset] && classes[offset] != ' ')
            offset++;

        if (offset > start)
            applyclass(render, state, classes + start, offset - start);

    }

}

static void findstyles(struct render *render, unsigned int root)
{

    unsigned int node;

    render->stylecount = 0;

    for (node = html_next(render->document, root, root); node && render->stylecount < STYLES; node = html_next(render->document, node, root))
    {

        struct html_node *element = &render->document->nodes[node];

        if (element->type == HTML_NODE_ELEMENT && element->tag == HTML_TAG_STYLE && element->first)
        {

            struct html_node *text = &render->document->nodes[element->first];

            render->styles[render->stylecount] = html_getstring(render->document, text->text);
            render->stylelengths[render->stylecount] = text->length;
            render->stylecount++;

        }

    }

}

static void applyattributes(struct render *render, struct state *state, unsigned int node)
{

    char *value;
    unsigned int i;

    render->node = node;

    for (i = 0; i < PROPERTIES; i++)
    {

        value = html_getattribute(render->document, node, properties[i]);

        if (value)
            apply(render, state, properties[i], cstring_length(properties[i]), value, cstring_length(value));

    }

    value = html_getattribute(render->document, node, "class");

    if (value && render->stylecount)
        applyclasses(render, state, value);

    value = html_getattribute(render->document, node, "style");

    if (value)
        applystyle(render, state, value, cstring_length(value));

    value = html_getattribute(render->document, node, "transform");

    if (value)
        svg_parsetransform(&state->matrix, value);

}

static unsigned int findstopcolor(struct render *render, unsigned int node, unsigned int current)
{

    char *color = html_getattribute(render->document, node, "stop-color");
    char *style = html_getattribute(render->document, node, "style");
    unsigned int value = (color) ? parsecolor(color, cstring_length(color), current) : 0xFF000000;

    if (style)
    {

        unsigned int offset = 0;

        while (style[offset])
        {

            unsigned int end = offset + buffer_findbyte(style + offset, cstring_length(style + offset), ';');
            unsigned int colon = offset + buffer_findbyte(style + offset, end - offset, ':');
            unsigned int start = offset;

            while (style[start] == ' ')
                start++;

            if (colon < end && colon - start == 10 && buffer_match(style + start, "stop-color", 10))
            {

                unsigned int parsed;

                start = colon + 1;

                while (start < end && style[start] == ' ')
                    start++;

                parsed = parsecolor(style + start, end - start, current);

                if (parsed)
                    value = parsed;

            }

            offset = (style[end]) ? end + 1 : end;

        }

    }

    return value;

}

static void buildshape(struct render *render, struct state *state, unsigned int node)
{

    int width = render->viewwidth;
    int height = render->viewheight;

    if (matchname(render, node, "path"))
    {

        char *d = html_getattribute(render->document, node, "d");

        if (d)
            svg_parsepath(&render->svg, &state->matrix, d);

    }

    else if (matchname(render, node, "rect"))
    {

        int x = getlength(render, node, "x", width, 0);
        int y = getlength(render, node, "y", height, 0);
        int w = getlength(render, node, "width", width, 0);
        int h = getlength(render, node, "height", height, 0);
        int rx = getlength(render, node, "rx", width, -1);
        int ry = getlength(render, node, "ry", height, -1);

        if (w <= 0 || h <= 0)
            return;

        rx = (rx < 0) ? ((ry < 0) ? 0 : ry) : rx;
        ry = (ry < 0) ? rx : ry;
        rx = (rx > w / 2) ? w / 2 : rx;
        ry = (ry > h / 2) ? h / 2 : ry;

        svg_moveto(&render->svg, &state->matrix, x + rx, y);
        svg_lineto(&render->svg, &state->matrix, x + w - rx, y);

        if (rx && ry)
            svg_ellipse(&render->svg, &state->matrix, x + w - rx, y + ry, rx, ry, SVG_TURN * 3 / 4, SVG_TURN / 4, SVG_ONE, 0);

        svg_lineto(&render->svg, &state->matrix, x + w, y + h - ry);

        if (rx && ry)
            svg_ellipse(&render->svg, &state->matrix, x + w - rx, y + h - ry, rx, ry, 0, SVG_TURN / 4, SVG_ONE, 0);

        svg_lineto(&render->svg, &state->matrix, x + rx, y + h);

        if (rx && ry)
            svg_ellipse(&render->svg, &state->matrix, x + rx, y + h - ry, rx, ry, SVG_TURN / 4, SVG_TURN / 4, SVG_ONE, 0);

        svg_lineto(&render->svg, &state->matrix, x, y + ry);

        if (rx && ry)
            svg_ellipse(&render->svg, &state->matrix, x + rx, y + ry, rx, ry, SVG_TURN / 2, SVG_TURN / 4, SVG_ONE, 0);

        svg_closepath(&render->svg);

    }

    else if (matchname(render, node, "circle") || matchname(render, node, "ellipse"))
    {

        int cx = getlength(render, node, "cx", width, 0);
        int cy = getlength(render, node, "cy", height, 0);
        int r = getlength(render, node, "r", width, 0);
        int rx = (matchname(render, node, "circle")) ? r : getlength(render, node, "rx", width, 0);
        int ry = (matchname(render, node, "circle")) ? r : getlength(render, node, "ry", height, 0);

        if (rx <= 0 || ry <= 0)
            return;

        svg_moveto(&render->svg, &state->matrix, cx + rx, cy);
        svg_ellipse(&render->svg, &state->matrix, cx, cy, rx, ry, 0, SVG_TURN, SVG_ONE, 0);
        svg_closepath(&render->svg);

    }

    else if (matchname(render, node, "line"))
    {

        svg_moveto(&render->svg, &state->matrix, getlength(render, node, "x1", width, 0), getlength(render, node, "y1", height, 0));
        svg_lineto(&render->svg, &state->matrix, getlength(render, node, "x2", width, 0), getlength(render, node, "y2", height, 0));

    }

    else if (matchname(render, node, "polyline") || matchname(render, node, "polygon"))
    {

        char *points = html_getattribute(render->document, node, "points");

        if (points)
            svg_parsepoints(&render->svg, &state->matrix, points, matchname(render, node, "polygon"));

    }

}

static unsigned int resolve(struct paint *paint, unsigned int current)
{

    return (paint->kind == PAINT_CURRENT) ? current : paint->color;

}

static void drawshape(struct render *render, struct state *state)
{

    if (!state->visible)
        return;

    if (state->fill.kind != PAINT_NONE)
    {

        unsigned int color = resolve(&state->fill, state->color);

        svg_fill(&render->svg, color, (color >> 24) * state->fillopacity / 255 * state->opacity / 255, state->evenodd);

    }

    if (state->stroke.kind != PAINT_NONE && state->strokewidth > 0)
    {

        unsigned int color = resolve(&state->stroke, state->color);

        svg_stroke(&render->svg, &state->matrix, state->strokewidth, state->cap, color, (color >> 24) * state->strokeopacity / 255 * state->opacity / 255);

    }

}

static struct viewport fitviewbox(char *viewbox, char *aspect, int width, int height)
{

    unsigned int aspectlength = (aspect) ? cstring_length(aspect) : 0;
    struct viewport viewport;
    struct svg_reader reader;
    int values[4];
    int sx;
    int sy;

    svg_setmatrix(&viewport.matrix, SVG_ONE, 0, 0, SVG_ONE, 0, 0);

    viewport.width = width;
    viewport.height = height;

    if (!viewbox)
        return viewport;

    reader.text = viewbox;
    reader.offset = 0;

    if (!svg_readpoints(&reader, values, 4) || values[2] <= 0 || values[3] <= 0)
        return viewport;

    sx = svg_divfix(width, values[2]);
    sy = svg_divfix(height, values[3]);

    if (!(aspect && cstring_match(aspect, "none")))
    {

        unsigned int slice = aspectlength >= 5 && buffer_match(aspect + aspectlength - 5, "slice", 5);
        int scale = (slice) ? ((sx > sy) ? sx : sy) : ((sx < sy) ? sx : sy);

        sx = scale;
        sy = scale;

    }

    svg_setmatrix(&viewport.matrix, sx, 0, 0, sy, (width - svg_mulfix(values[2], sx)) / 2 - svg_mulfix(values[0], sx), (height - svg_mulfix(values[3], sy)) / 2 - svg_mulfix(values[1], sy));

    viewport.width = values[2];
    viewport.height = values[3];

    return viewport;

}

static void walk(struct render *render, unsigned int node, struct state *parent, unsigned int depth);

static void walkchildren(struct render *render, unsigned int node, struct state *state, unsigned int depth)
{

    unsigned int child;

    for (child = render->document->nodes[node].first; child; child = render->document->nodes[child].next)
    {

        if (render->document->nodes[child].type == HTML_NODE_ELEMENT)
            walk(render, child, state, depth + 1);

    }

}

static unsigned int isskipped(struct render *render, unsigned int node)
{

    char *name = html_getstring(render->document, render->document->nodes[node].name);

    return cstring_match(name, "defs") || cstring_match(name, "clippath") || cstring_match(name, "mask") || cstring_match(name, "symbol") || cstring_match(name, "lineargradient") || cstring_match(name, "radialgradient") || cstring_match(name, "pattern") || cstring_match(name, "marker") || cstring_match(name, "filter") || cstring_match(name, "style") || cstring_match(name, "script") || cstring_match(name, "title") || cstring_match(name, "desc") || cstring_match(name, "metadata") || cstring_match(name, "text") || cstring_match(name, "foreignobject") || cstring_match(name, "image");

}

static void walkuse(struct render *render, unsigned int node, struct state *state, unsigned int depth)
{

    char *href = html_getattribute(render->document, node, "href");
    unsigned int target;
    struct svg_matrix shift;

    if (!href)
        href = html_getattribute(render->document, node, "xlink:href");

    if (!href || href[0] != '#')
        return;

    target = findid(render, href + 1, cstring_length(href + 1));

    if (!target || target == node)
        return;

    svg_setmatrix(&shift, SVG_ONE, 0, 0, SVG_ONE, getlength(render, node, "x", render->viewwidth, 0), getlength(render, node, "y", render->viewheight, 0));
    svg_multiply(&state->matrix, &shift);

    /* a symbol fits its own viewbox into the use, which fills the viewport unless it says otherwise */
    if (matchname(render, target, "symbol"))
    {

        int viewwidth = render->viewwidth;
        int viewheight = render->viewheight;
        struct viewport viewport = fitviewbox(html_getattribute(render->document, target, "viewbox"), html_getattribute(render->document, target, "preserveaspectratio"), getlength(render, node, "width", viewwidth, viewwidth), getlength(render, node, "height", viewheight, viewheight));

        svg_multiply(&state->matrix, &viewport.matrix);

        render->viewwidth = viewport.width;
        render->viewheight = viewport.height;

        applyattributes(render, state, target);
        walkchildren(render, target, state, depth + 1);

        render->viewwidth = viewwidth;
        render->viewheight = viewheight;

    }

    else
    {

        walk(render, target, state, depth + 1);

    }

}

static void walk(struct render *render, unsigned int node, struct state *parent, unsigned int depth)
{

    struct state state;

    if (depth > DEPTH || isskipped(render, node))
        return;

    buffer_copy(&state, parent, sizeof (struct state));
    applyattributes(render, &state, node);

    if (!state.display)
        return;

    if (matchname(render, node, "g") || matchname(render, node, "a") || matchname(render, node, "svg"))
    {

        if (matchname(render, node, "svg"))
        {

            struct svg_matrix shift;

            svg_setmatrix(&shift, SVG_ONE, 0, 0, SVG_ONE, getlength(render, node, "x", render->viewwidth, 0), getlength(render, node, "y", render->viewheight, 0));
            svg_multiply(&state.matrix, &shift);

        }

        walkchildren(render, node, &state, depth);

    }

    else if (matchname(render, node, "switch"))
    {

        unsigned int child;

        for (child = render->document->nodes[node].first; child; child = render->document->nodes[child].next)
        {

            if (render->document->nodes[child].type == HTML_NODE_ELEMENT && !isskipped(render, child) && !html_getattribute(render->document, child, "requiredextensions"))
            {

                walk(render, child, &state, depth + 1);

                break;

            }

        }

    }

    else if (matchname(render, node, "use"))
    {

        walkuse(render, node, &state, depth);

    }

    else
    {

        svg_reset(&render->svg);

        buildshape(render, &state, node);
        drawshape(render, &state);

    }

}

unsigned int svg_findroot(struct html_document *document)
{

    unsigned int node;

    for (node = 1; node < document->nodecount; node++)
    {

        if (document->nodes[node].type == HTML_NODE_ELEMENT && document->nodes[node].tag == HTML_TAG_SVG)
            return node;

    }

    return 0;

}

static struct svg_number readsize(char *text, unsigned int fontsize)
{

    struct svg_reader reader;
    struct svg_number result;

    if (!text)
        return number(0, 0);

    reader.text = text;
    reader.offset = 0;
    result = svg_readnumber(&reader);

    if (!result.valid || result.value <= 0 || text[reader.offset] == '%')
        return number(0, 0);

    if (text[reader.offset] == 'e' && text[reader.offset + 1] == 'm')
        result.value *= (int)fontsize;

    return number((result.value + SVG_ONE / 2) >> 16, 1);

}

struct svg_size svg_measure(struct html_document *document, unsigned int root, unsigned int fontsize)
{

    struct svg_size size;
    struct svg_number width = readsize(html_getattribute(document, root, "width"), fontsize);
    struct svg_number height = readsize(html_getattribute(document, root, "height"), fontsize);
    char *viewbox = html_getattribute(document, root, "viewbox");
    int ratiowidth = 0;
    int ratioheight = 0;

    if (viewbox)
    {

        struct svg_reader reader;
        int values[4];

        reader.text = viewbox;
        reader.offset = 0;

        if (svg_readpoints(&reader, values, 4) && values[2] > 0 && values[3] > 0)
        {

            ratiowidth = values[2];
            ratioheight = values[3];

        }

    }

    if (!ratiowidth && width.valid && height.valid)
    {

        ratiowidth = width.value;
        ratioheight = height.value;

    }

    while (ratiowidth > 0x7FFF || ratioheight > 0x7FFF)
    {

        ratiowidth >>= 1;
        ratioheight >>= 1;

    }

    size.width = (width.valid) ? (unsigned int)width.value : 0;
    size.height = (height.valid) ? (unsigned int)height.value : 0;
    size.ratiowidth = (ratiowidth > 0 && ratioheight > 0) ? (unsigned int)ratiowidth : 0;
    size.ratioheight = (ratiowidth > 0 && ratioheight > 0) ? (unsigned int)ratioheight : 0;

    return size;

}

void svg_render(struct svg_canvas *canvas, struct html_document *document, struct css_sheet *sheet, unsigned int root, unsigned int color, unsigned int fill)
{

    struct state state;
    int width = (int)canvas->width << 16;
    int height = (int)canvas->height << 16;
    struct viewport viewport = fitviewbox(html_getattribute(document, root, "viewbox"), html_getattribute(document, root, "preserveaspectratio"), width, height);

    if (!canvas->width || canvas->width > SVG_WIDTH || !canvas->height)
        return;

    raster.svg.pixels = canvas->pixels;
    raster.svg.width = canvas->width;
    raster.svg.height = canvas->height;
    raster.document = document;
    raster.sheet = sheet;
    raster.depth = 0;
    raster.viewwidth = viewport.width;
    raster.viewheight = viewport.height;

    findstyles(&raster, root);

    buffer_clear(&state, sizeof (struct state));
    buffer_copy(&state.matrix, &viewport.matrix, sizeof (struct svg_matrix));

    state.fill.kind = PAINT_COLOR;
    state.fill.color = 0xFF000000;
    state.stroke.kind = PAINT_NONE;
    state.strokewidth = SVG_ONE;
    state.color = color;
    state.fillopacity = 255;
    state.strokeopacity = 255;
    state.opacity = 255;
    state.display = 1;
    state.visible = 1;

    applyattributes(&raster, &state, root);

    if (fill)
    {

        state.fill.kind = (fill == CSS_CURRENTCOLOR) ? PAINT_CURRENT : ((fill >> 24) ? PAINT_COLOR : PAINT_NONE);
        state.fill.color = fill;

    }

    if (state.display)
        walkchildren(&raster, root, &state, 0);

}

