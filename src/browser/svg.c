#include <fudge.h>
#include "html.h"
#include "css.h"
#include "svg.h"

#define ONE                             0x10000
#define TURN                            0x10000
#define POINTS                          16384
#define SUBPATHS                        1024
#define EDGES                           16384
#define CROSSINGS                       1024
#define MAXWIDTH                        4096
#define SAMPLES                         4
#define DEPTH                           16
#define CURVESTEPS                      32
#define ARCSTEPS                        64
#define ROUNDSTEPS                      16
#define ATANS                           16
#define PROPERTIES                      11
#define STYLES                          8
#define PAINT_NONE                      0
#define PAINT_COLOR                     1
#define PAINT_CURRENT                   2
#define CAP_BUTT                        0
#define CAP_ROUND                       1
#define CAP_SQUARE                      2

struct matrix
{

    int a;
    int b;
    int c;
    int d;
    int e;
    int f;

};

struct paint
{

    unsigned int kind;
    unsigned int color;

};

struct state
{

    struct matrix matrix;
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

struct point
{

    int x;
    int y;

};

struct subpath
{

    unsigned int start;
    unsigned int count;
    unsigned int closed;

};

struct edge
{

    int x0;
    int y0;
    int y1;
    int slope;
    int winding;

};

struct reader
{

    char *text;
    unsigned int offset;

};

struct number
{

    int value;
    unsigned int valid;

};

struct pen
{

    int x;
    int y;
    int startx;
    int starty;
    int controlx;
    int controly;
    unsigned int curve;
    unsigned int needmove;

};

struct render
{

    struct svg_canvas *canvas;
    struct html_document *document;
    struct point points[POINTS];
    struct subpath subpaths[SUBPATHS];
    struct edge edges[EDGES];
    int coverage[MAXWIDTH];
    int crossings[CROSSINGS];
    int windings[CROSSINGS];
    unsigned int pointcount;
    unsigned int subpathcount;
    unsigned int edgecount;
    int viewwidth;
    int viewheight;
    char *styles[STYLES];
    unsigned int stylelengths[STYLES];
    unsigned int stylecount;

};

static int atans[ATANS] = {8192, 4836, 2555, 1297, 651, 326, 163, 81, 41, 20, 10, 5, 3, 1, 1, 0};
static char *properties[PROPERTIES] = {"fill", "stroke", "stroke-width", "fill-opacity", "stroke-opacity", "opacity", "fill-rule", "stroke-linecap", "color", "display", "visibility"};
static struct render raster;

static unsigned int magnitude(int value)
{

    return (value < 0) ? (unsigned int)-value : (unsigned int)value;

}

static int mulfix(int a, int b)
{

    unsigned int ua = magnitude(a);
    unsigned int ub = magnitude(b);
    unsigned int result = ((ua >> 16) * (ub >> 16) << 16) + (ua >> 16) * (ub & 0xFFFF) + (ua & 0xFFFF) * (ub >> 16) + ((ua & 0xFFFF) * (ub & 0xFFFF) >> 16);

    return ((a < 0) != (b < 0)) ? -(int)result : (int)result;

}

static int divfix(int a, int b)
{

    unsigned int ua = magnitude(a);
    unsigned int ub = magnitude(b);
    unsigned int quotient;
    unsigned int remainder;
    unsigned int i;

    if (!ub)
        return 0;

    quotient = ua / ub;
    remainder = ua % ub;

    if (quotient >= 0x8000)
        return ((a < 0) != (b < 0)) ? -0x7FFFFFFF : 0x7FFFFFFF;

    for (i = 0; i < 16; i++)
    {

        remainder <<= 1;
        quotient <<= 1;

        if (remainder >= ub)
        {

            remainder -= ub;
            quotient |= 1;

        }

    }

    return ((a < 0) != (b < 0)) ? -(int)quotient : (int)quotient;

}

static unsigned int isqrt(unsigned int value)
{

    unsigned int root = 0;
    unsigned int bit = 1 << 30;

    while (bit > value)
        bit >>= 2;

    while (bit)
    {

        if (value >= root + bit)
        {

            value -= root + bit;
            root = (root >> 1) + bit;

        }

        else
        {

            root >>= 1;

        }

        bit >>= 2;

    }

    return root;

}

static int sqrtfix(int value)
{

    if (value <= 0)
        return 0;

    return (value < 0x8000) ? (int)isqrt((unsigned int)value << 16) : (int)isqrt(value) << 8;

}

static struct point rotation(int angle)
{

    struct point result;
    unsigned int quadrant = (angle & (TURN - 1)) >> 14;
    int z = angle & 0x3FFF;
    int x = 39797;
    int y = 0;
    unsigned int i;

    for (i = 0; i < ATANS; i++)
    {

        int next;

        if (z >= 0)
        {

            next = x - (y >> i);
            y += x >> i;
            z -= atans[i];

        }

        else
        {

            next = x + (y >> i);
            y -= x >> i;
            z += atans[i];

        }

        x = next;

    }

    result.x = (quadrant == 0) ? x : (quadrant == 1) ? -y : (quadrant == 2) ? -x : y;
    result.y = (quadrant == 0) ? y : (quadrant == 1) ? x : (quadrant == 2) ? -y : -x;

    return result;

}

static int direction(int x, int y)
{

    int z = 0;
    unsigned int i;

    if (!x && !y)
        return 0;

    while (magnitude(x) > 0x10000000 || magnitude(y) > 0x10000000)
    {

        x >>= 1;
        y >>= 1;

    }

    if (x < 0)
    {

        x = -x;
        y = -y;
        z = TURN / 2;

    }

    for (i = 0; i < ATANS; i++)
    {

        int next;

        if (y > 0)
        {

            next = x + (y >> i);
            y -= x >> i;
            z += atans[i];

        }

        else
        {

            next = x - (y >> i);
            y += x >> i;
            z -= atans[i];

        }

        x = next;

    }

    return z & (TURN - 1);

}

static void setmatrix(struct matrix *matrix, int a, int b, int c, int d, int e, int f)
{

    matrix->a = a;
    matrix->b = b;
    matrix->c = c;
    matrix->d = d;
    matrix->e = e;
    matrix->f = f;

}

static void multiply(struct matrix *matrix, struct matrix *by)
{

    struct matrix result;

    result.a = mulfix(matrix->a, by->a) + mulfix(matrix->c, by->b);
    result.b = mulfix(matrix->b, by->a) + mulfix(matrix->d, by->b);
    result.c = mulfix(matrix->a, by->c) + mulfix(matrix->c, by->d);
    result.d = mulfix(matrix->b, by->c) + mulfix(matrix->d, by->d);
    result.e = mulfix(matrix->a, by->e) + mulfix(matrix->c, by->f) + matrix->e;
    result.f = mulfix(matrix->b, by->e) + mulfix(matrix->d, by->f) + matrix->f;

    buffer_copy(matrix, &result, sizeof (struct matrix));

}

static struct point project(struct matrix *matrix, int x, int y)
{

    struct point result;

    result.x = (mulfix(matrix->a, x) + mulfix(matrix->c, y) + matrix->e) >> 8;
    result.y = (mulfix(matrix->b, x) + mulfix(matrix->d, y) + matrix->f) >> 8;

    return result;

}

static void skipseparators(struct reader *reader)
{

    char c = reader->text[reader->offset];

    while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',')
        c = reader->text[++reader->offset];

}

static struct number number(int value, unsigned int valid)
{

    struct number result;

    result.value = value;
    result.valid = valid;

    return result;

}

static struct number readnumber(struct reader *reader)
{

    char *text = reader->text;
    unsigned int offset;
    unsigned int digits = 0;
    unsigned int negative = 0;
    int whole = 0;
    int fraction = 0;
    int scale = 1;
    int exponent = 0;
    int value;

    skipseparators(reader);

    offset = reader->offset;

    if (text[offset] == '+' || text[offset] == '-')
        negative = text[offset++] == '-';

    for (; cstring_isdigit(text[offset]); offset++, digits++)
        whole = (whole < 32767) ? whole * 10 + text[offset] - '0' : 32767;

    if (text[offset] == '.')
    {

        for (offset++; cstring_isdigit(text[offset]); offset++, digits++)
        {

            if (scale < 10000)
            {

                fraction = fraction * 10 + text[offset] - '0';
                scale *= 10;

            }

        }

    }

    if (!digits)
        return number(0, 0);

    if ((text[offset] == 'e' || text[offset] == 'E') && (cstring_isdigit(text[offset + 1]) || ((text[offset + 1] == '-' || text[offset + 1] == '+') && cstring_isdigit(text[offset + 2]))))
    {

        unsigned int below = text[offset + 1] == '-';

        offset += (cstring_isdigit(text[offset + 1])) ? 1 : 2;

        for (; cstring_isdigit(text[offset]); offset++)
            exponent = (exponent < 100) ? exponent * 10 + text[offset] - '0' : 100;

        exponent = (below) ? -exponent : exponent;

    }

    value = ((whole > 32767) ? 32767 : whole) * ONE + fraction * ONE / scale;

    for (; exponent > 0 && value < 0x7FFFFFFF / 10; exponent--)
        value *= 10;

    for (; exponent < 0; exponent++)
        value /= 10;

    reader->offset = offset;

    return number((negative) ? -value : value, 1);

}

static struct number readflag(struct reader *reader)
{

    char c;

    skipseparators(reader);

    c = reader->text[reader->offset];

    if (c != '0' && c != '1')
        return number(0, 0);

    reader->offset++;

    return number(c == '1', 1);

}

static struct number readlength(char *text, int base)
{

    struct reader reader;
    struct number result;

    if (!text)
        return number(0, 0);

    reader.text = text;
    reader.offset = 0;
    result = readnumber(&reader);

    if (result.valid && text[reader.offset] == '%')
        result.value = mulfix(base, result.value / 100);

    return result;

}

static int getlength(struct render *render, unsigned int node, char *name, int base, int fallback)
{

    struct number result = readlength(html_getattribute(render->document, node, name), base);

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

    struct reader reader;
    struct number result;
    char buffer[32];

    if (length >= 32)
        return 255;

    buffer_copy(buffer, text, length);

    buffer[length] = '\0';
    reader.text = buffer;
    reader.offset = 0;
    result = readnumber(&reader);

    if (!result.valid)
        return 255;

    if (buffer[reader.offset] == '%')
        result.value /= 100;

    return (result.value <= 0) ? 0 : (result.value >= ONE) ? 255 : (unsigned int)(result.value * 255 / ONE);

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
        struct reader reader;
        struct number width;

        if (length >= 32)
            return;

        buffer_copy(buffer, value, length);

        buffer[length] = '\0';
        reader.text = buffer;
        reader.offset = 0;
        width = readnumber(&reader);

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

        state->cap = (matchvalue(value, length, "round")) ? CAP_ROUND : (matchvalue(value, length, "square")) ? CAP_SQUARE : CAP_BUTT;

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

static void parsetransform(struct matrix *matrix, char *text)
{

    struct reader reader;

    reader.text = text;
    reader.offset = 0;

    while (text[reader.offset])
    {

        unsigned int start;
        unsigned int length;
        struct matrix by;
        int values[6];
        unsigned int count = 0;

        skipseparators(&reader);

        start = reader.offset;

        while (cstring_isalpha(text[reader.offset]))
            reader.offset++;

        length = reader.offset - start;

        skipseparators(&reader);

        if (!length || text[reader.offset] != '(')
            return;

        reader.offset++;

        while (count < 6)
        {

            struct number value = readnumber(&reader);

            if (!value.valid)
                break;

            values[count++] = value.value;

        }

        skipseparators(&reader);

        if (text[reader.offset] != ')')
            return;

        reader.offset++;

        setmatrix(&by, ONE, 0, 0, ONE, 0, 0);

        if (matchvalue(text + start, length, "matrix") && count == 6)
        {

            setmatrix(&by, values[0], values[1], values[2], values[3], values[4], values[5]);

        }

        else if (matchvalue(text + start, length, "translate") && count)
        {

            setmatrix(&by, ONE, 0, 0, ONE, values[0], (count > 1) ? values[1] : 0);

        }

        else if (matchvalue(text + start, length, "scale") && count)
        {

            setmatrix(&by, values[0], 0, 0, (count > 1) ? values[1] : values[0], 0, 0);

        }

        else if (matchvalue(text + start, length, "rotate") && count)
        {

            struct point unit = rotation(mulfix(values[0], 11930465) >> 16);

            if (count == 3)
            {

                struct matrix shift;

                setmatrix(&shift, ONE, 0, 0, ONE, values[1], values[2]);
                multiply(matrix, &shift);

            }

            setmatrix(&by, unit.x, unit.y, -unit.y, unit.x, 0, 0);

            if (count == 3)
            {

                multiply(matrix, &by);
                setmatrix(&by, ONE, 0, 0, ONE, -values[1], -values[2]);

            }

        }

        else if ((matchvalue(text + start, length, "skewX") || matchvalue(text + start, length, "skewY")) && count)
        {

            struct point unit = rotation(mulfix(values[0], 11930465) >> 16);
            int tangent = divfix(unit.y, unit.x);

            if (text[start + 4] == 'X')
                by.c = tangent;
            else
                by.b = tangent;

        }

        multiply(matrix, &by);

    }

}

static void applyattributes(struct render *render, struct state *state, unsigned int node)
{

    char *value;
    unsigned int i;

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
        parsetransform(&state->matrix, value);

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

static void addpoint(struct render *render, struct point point)
{

    struct subpath *subpath;

    if (!render->subpathcount || render->pointcount >= POINTS)
        return;

    subpath = &render->subpaths[render->subpathcount - 1];

    if (subpath->count && render->points[render->pointcount - 1].x == point.x && render->points[render->pointcount - 1].y == point.y)
        return;

    render->points[render->pointcount].x = point.x;
    render->points[render->pointcount].y = point.y;
    render->pointcount++;
    subpath->count++;

}

static void moveto(struct render *render, struct state *state, int x, int y)
{

    struct subpath *subpath;

    if (render->subpathcount && render->subpaths[render->subpathcount - 1].count < 2)
    {

        render->pointcount -= render->subpaths[render->subpathcount - 1].count;
        render->subpathcount--;

    }

    if (render->subpathcount >= SUBPATHS)
        return;

    subpath = &render->subpaths[render->subpathcount++];
    subpath->start = render->pointcount;
    subpath->count = 0;
    subpath->closed = 0;

    addpoint(render, project(&state->matrix, x, y));

}

static void lineto(struct render *render, struct state *state, int x, int y)
{

    if (!render->subpathcount)
        moveto(render, state, x, y);
    else
        addpoint(render, project(&state->matrix, x, y));

}

static void closepath(struct render *render)
{

    if (render->subpathcount)
        render->subpaths[render->subpathcount - 1].closed = 1;

}

static int lerp(int a, int b, int step, int steps)
{

    return a + (b - a) * step / steps;

}

static void cubic(struct render *render, struct state *state, struct pen *pen, int x1, int y1, int x2, int y2, int x3, int y3)
{

    struct point p0 = project(&state->matrix, pen->x, pen->y);
    struct point p1 = project(&state->matrix, x1, y1);
    struct point p2 = project(&state->matrix, x2, y2);
    struct point p3 = project(&state->matrix, x3, y3);
    unsigned int length = (magnitude(p1.x - p0.x) + magnitude(p1.y - p0.y) + magnitude(p2.x - p1.x) + magnitude(p2.y - p1.y) + magnitude(p3.x - p2.x) + magnitude(p3.y - p2.y)) >> 8;
    int steps = (length / 3 < 2) ? 2 : (length / 3 > CURVESTEPS) ? CURVESTEPS : (int)length / 3;
    int i;

    for (i = 1; i <= steps; i++)
    {

        struct point point;
        int abx = lerp(p0.x, p1.x, i, steps);
        int aby = lerp(p0.y, p1.y, i, steps);
        int bcx = lerp(p1.x, p2.x, i, steps);
        int bcy = lerp(p1.y, p2.y, i, steps);
        int cdx = lerp(p2.x, p3.x, i, steps);
        int cdy = lerp(p2.y, p3.y, i, steps);
        int abcx = lerp(abx, bcx, i, steps);
        int abcy = lerp(aby, bcy, i, steps);
        int bcdx = lerp(bcx, cdx, i, steps);
        int bcdy = lerp(bcy, cdy, i, steps);

        point.x = lerp(abcx, bcdx, i, steps);
        point.y = lerp(abcy, bcdy, i, steps);

        addpoint(render, point);

    }

    pen->x = x3;
    pen->y = y3;

}

static void ellipse(struct render *render, struct state *state, int cx, int cy, int rx, int ry, int from, int sweep, int cosine, int sine)
{

    int steps = (int)(magnitude(sweep) * ARCSTEPS / TURN) + 1;
    int i;

    for (i = 1; i <= steps; i++)
    {

        struct point unit = rotation(from + sweep * i / steps);
        int px = mulfix(rx, unit.x);
        int py = mulfix(ry, unit.y);

        lineto(render, state, cx + mulfix(cosine, px) - mulfix(sine, py), cy + mulfix(sine, px) + mulfix(cosine, py));

    }

}

static void arc(struct render *render, struct state *state, struct pen *pen, int rx, int ry, int angle, unsigned int large, unsigned int sweep, int x, int y)
{

    struct point axis = rotation(mulfix(angle, 11930465) >> 16);
    int dx = (pen->x - x) / 2;
    int dy = (pen->y - y) / 2;
    int x1 = mulfix(axis.x, dx) + mulfix(axis.y, dy);
    int y1 = mulfix(axis.x, dy) - mulfix(axis.y, dx);
    int ux;
    int uy;
    int lambda;
    int factor;
    int cx;
    int cy;
    int from;
    int delta;

    rx = (int)magnitude(rx);
    ry = (int)magnitude(ry);

    if (!rx || !ry || (pen->x == x && pen->y == y))
    {

        lineto(render, state, x, y);

        pen->x = x;
        pen->y = y;

        return;

    }

    ux = divfix(x1, rx);
    uy = divfix(y1, ry);
    lambda = mulfix(ux, ux) + mulfix(uy, uy);

    if (lambda > ONE)
    {

        int scale = sqrtfix(lambda);

        rx = mulfix(rx, scale);
        ry = mulfix(ry, scale);
        ux = divfix(ux, scale);
        uy = divfix(uy, scale);
        lambda = mulfix(ux, ux) + mulfix(uy, uy);

    }

    factor = (lambda > 0 && lambda < ONE) ? sqrtfix(divfix(ONE - lambda, lambda)) : 0;

    if (large == sweep)
        factor = -factor;

    cx = mulfix(factor, uy);
    cy = -mulfix(factor, ux);
    from = direction(ux - cx, uy - cy);
    delta = (direction(-ux - cx, -uy - cy) - from) & (TURN - 1);

    if (!sweep && delta)
        delta -= TURN;

    {

        int tx = mulfix(rx, cx);
        int ty = mulfix(ry, cy);
        int centerx = mulfix(axis.x, tx) - mulfix(axis.y, ty) + (pen->x + x) / 2;
        int centery = mulfix(axis.y, tx) + mulfix(axis.x, ty) + (pen->y + y) / 2;

        ellipse(render, state, centerx, centery, rx, ry, from, delta, axis.x, axis.y);

    }

    lineto(render, state, x, y);

    pen->x = x;
    pen->y = y;

}

static unsigned int readpoints(struct reader *reader, int *values, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct number value = readnumber(reader);

        if (!value.valid)
            return 0;

        values[i] = value.value;

    }

    return 1;

}

static unsigned int pathcommand(struct render *render, struct state *state, struct reader *reader, struct pen *pen, char command)
{

    unsigned int relative = command >= 'a';
    char upper = (relative) ? command - 32 : command;
    int ox = (relative) ? pen->x : 0;
    int oy = (relative) ? pen->y : 0;
    int values[7];
    unsigned int curve = 0;

    if (upper != 'M' && upper != 'Z' && pen->needmove)
    {

        moveto(render, state, pen->startx, pen->starty);

        pen->needmove = 0;

    }

    switch (upper)
    {

    case 'M':
        if (!readpoints(reader, values, 2))
            return 0;

        pen->x = pen->startx = values[0] + ox;
        pen->y = pen->starty = values[1] + oy;
        pen->needmove = 0;

        moveto(render, state, pen->x, pen->y);

        break;

    case 'L':
        if (!readpoints(reader, values, 2))
            return 0;

        pen->x = values[0] + ox;
        pen->y = values[1] + oy;

        lineto(render, state, pen->x, pen->y);

        break;

    case 'H':
        if (!readpoints(reader, values, 1))
            return 0;

        pen->x = values[0] + ox;

        lineto(render, state, pen->x, pen->y);

        break;

    case 'V':
        if (!readpoints(reader, values, 1))
            return 0;

        pen->y = values[0] + oy;

        lineto(render, state, pen->x, pen->y);

        break;

    case 'C':
        if (!readpoints(reader, values, 6))
            return 0;

        pen->controlx = values[2] + ox;
        pen->controly = values[3] + oy;
        curve = 1;

        cubic(render, state, pen, values[0] + ox, values[1] + oy, pen->controlx, pen->controly, values[4] + ox, values[5] + oy);

        break;

    case 'S':
        if (!readpoints(reader, values, 4))
            return 0;

        values[4] = (pen->curve == 1) ? 2 * pen->x - pen->controlx : pen->x;
        values[5] = (pen->curve == 1) ? 2 * pen->y - pen->controly : pen->y;
        pen->controlx = values[0] + ox;
        pen->controly = values[1] + oy;
        curve = 1;

        cubic(render, state, pen, values[4], values[5], pen->controlx, pen->controly, values[2] + ox, values[3] + oy);

        break;

    case 'Q':
    case 'T':
        if (upper == 'Q' && !readpoints(reader, values, 4))
            return 0;

        if (upper == 'T')
        {

            if (!readpoints(reader, values + 2, 2))
                return 0;

            values[0] = (pen->curve == 2) ? 2 * pen->x - pen->controlx - ox : pen->x - ox;
            values[1] = (pen->curve == 2) ? 2 * pen->y - pen->controly - oy : pen->y - oy;

        }

        pen->controlx = values[0] + ox;
        pen->controly = values[1] + oy;
        curve = 2;

        cubic(render, state, pen, pen->x + (pen->controlx - pen->x) * 2 / 3, pen->y + (pen->controly - pen->y) * 2 / 3, values[2] + ox + (pen->controlx - values[2] - ox) * 2 / 3, values[3] + oy + (pen->controly - values[3] - oy) * 2 / 3, values[2] + ox, values[3] + oy);

        break;

    case 'A':
        {

            struct number large;
            struct number sweep;

            if (!readpoints(reader, values, 3))
                return 0;

            large = readflag(reader);
            sweep = readflag(reader);

            if (!large.valid || !sweep.valid || !readpoints(reader, values + 3, 2))
                return 0;

            arc(render, state, pen, values[0], values[1], values[2], large.value, sweep.value, values[3] + ox, values[4] + oy);

        }

        break;

    case 'Z':
        closepath(render);

        pen->x = pen->startx;
        pen->y = pen->starty;
        pen->needmove = 1;

        break;

    default:
        return 0;

    }

    pen->curve = curve;

    return 1;

}

static void parsepath(struct render *render, struct state *state, char *text)
{

    struct reader reader;
    struct pen pen;
    char command = 0;

    reader.text = text;
    reader.offset = 0;

    buffer_clear(&pen, sizeof (struct pen));

    while (1)
    {

        char c;

        skipseparators(&reader);

        c = text[reader.offset];

        if (!c)
            break;

        if (cstring_isalpha(c))
        {

            command = c;
            reader.offset++;

        }

        else if (!command || command == 'Z' || command == 'z')
        {

            break;

        }

        if (!pathcommand(render, state, &reader, &pen, command))
            break;

        if (command == 'M')
            command = 'L';
        else if (command == 'm')
            command = 'l';

    }

}

static void parsepoints(struct render *render, struct state *state, char *text, unsigned int closed)
{

    struct reader reader;
    unsigned int first = 1;

    reader.text = text;
    reader.offset = 0;

    while (1)
    {

        int values[2];

        if (!readpoints(&reader, values, 2))
            break;

        if (first)
            moveto(render, state, values[0], values[1]);
        else
            lineto(render, state, values[0], values[1]);

        first = 0;

    }

    if (closed)
        closepath(render);

}

static void buildshape(struct render *render, struct state *state, unsigned int node)
{

    int width = render->viewwidth;
    int height = render->viewheight;

    if (matchname(render, node, "path"))
    {

        char *d = html_getattribute(render->document, node, "d");

        if (d)
            parsepath(render, state, d);

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

        moveto(render, state, x + rx, y);
        lineto(render, state, x + w - rx, y);

        if (rx && ry)
            ellipse(render, state, x + w - rx, y + ry, rx, ry, TURN * 3 / 4, TURN / 4, ONE, 0);

        lineto(render, state, x + w, y + h - ry);

        if (rx && ry)
            ellipse(render, state, x + w - rx, y + h - ry, rx, ry, 0, TURN / 4, ONE, 0);

        lineto(render, state, x + rx, y + h);

        if (rx && ry)
            ellipse(render, state, x + rx, y + h - ry, rx, ry, TURN / 4, TURN / 4, ONE, 0);

        lineto(render, state, x, y + ry);

        if (rx && ry)
            ellipse(render, state, x + rx, y + ry, rx, ry, TURN / 2, TURN / 4, ONE, 0);

        closepath(render);

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

        moveto(render, state, cx + rx, cy);
        ellipse(render, state, cx, cy, rx, ry, 0, TURN, ONE, 0);
        closepath(render);

    }

    else if (matchname(render, node, "line"))
    {

        moveto(render, state, getlength(render, node, "x1", width, 0), getlength(render, node, "y1", height, 0));
        lineto(render, state, getlength(render, node, "x2", width, 0), getlength(render, node, "y2", height, 0));

    }

    else if (matchname(render, node, "polyline") || matchname(render, node, "polygon"))
    {

        char *points = html_getattribute(render->document, node, "points");

        if (points)
            parsepoints(render, state, points, matchname(render, node, "polygon"));

    }

}

static void addedge(struct render *render, struct point a, struct point b)
{

    struct edge *edge;

    if (a.y == b.y || render->edgecount >= EDGES)
        return;

    edge = &render->edges[render->edgecount++];

    if (a.y < b.y)
    {

        edge->winding = 1;
        edge->x0 = a.x;
        edge->y0 = a.y;
        edge->y1 = b.y;
        edge->slope = ((b.x - a.x) << 8) / (b.y - a.y);

    }

    else
    {

        edge->winding = -1;
        edge->x0 = b.x;
        edge->y0 = b.y;
        edge->y1 = a.y;
        edge->slope = ((a.x - b.x) << 8) / (a.y - b.y);

    }

}

static void fillsubpaths(struct render *render)
{

    unsigned int i;

    for (i = 0; i < render->subpathcount; i++)
    {

        struct subpath *subpath = &render->subpaths[i];
        unsigned int j;

        if (subpath->count < 2)
            continue;

        for (j = 0; j < subpath->count; j++)
            addedge(render, render->points[subpath->start + j], render->points[subpath->start + (j + 1) % subpath->count]);

    }

}

static void addround(struct render *render, struct point center, int radius)
{

    struct point previous;
    unsigned int i;

    previous.x = center.x + radius;
    previous.y = center.y;

    for (i = 1; i <= ROUNDSTEPS; i++)
    {

        struct point unit = rotation(-(int)(i * TURN / ROUNDSTEPS));
        struct point next;

        next.x = center.x + mulfix(unit.x, radius);
        next.y = center.y + mulfix(unit.y, radius);

        addedge(render, previous, next);

        previous.x = next.x;
        previous.y = next.y;

    }

}

static void addsegment(struct render *render, struct point a, struct point b, int radius, unsigned int startcap, unsigned int endcap)
{

    int dx = b.x - a.x;
    int dy = b.y - a.y;
    unsigned int shift = 0;
    unsigned int length;
    struct point corners[4];
    int nx;
    int ny;
    int ux;
    int uy;

    while ((magnitude(dx) >> shift) > 0x7FFF || (magnitude(dy) >> shift) > 0x7FFF)
        shift++;

    length = isqrt((unsigned int)((dx >> shift) * (dx >> shift)) + (unsigned int)((dy >> shift) * (dy >> shift)));

    if (!length)
        return;

    ux = (dx >> shift) * 16384 / (int)length;
    uy = (dy >> shift) * 16384 / (int)length;
    nx = -uy * radius >> 14;
    ny = ux * radius >> 14;

    if (startcap)
    {

        a.x -= ux * radius >> 14;
        a.y -= uy * radius >> 14;

    }

    if (endcap)
    {

        b.x += ux * radius >> 14;
        b.y += uy * radius >> 14;

    }

    corners[0].x = a.x + nx;
    corners[0].y = a.y + ny;
    corners[1].x = b.x + nx;
    corners[1].y = b.y + ny;
    corners[2].x = b.x - nx;
    corners[2].y = b.y - ny;
    corners[3].x = a.x - nx;
    corners[3].y = a.y - ny;

    addedge(render, corners[0], corners[1]);
    addedge(render, corners[1], corners[2]);
    addedge(render, corners[2], corners[3]);
    addedge(render, corners[3], corners[0]);

}

static void strokesubpaths(struct render *render, int radius, unsigned int cap)
{

    unsigned int i;

    for (i = 0; i < render->subpathcount; i++)
    {

        struct subpath *subpath = &render->subpaths[i];
        unsigned int segments = (subpath->closed && subpath->count > 2) ? subpath->count : subpath->count - 1;
        unsigned int j;

        if (subpath->count < 2)
            continue;

        for (j = 0; j < segments; j++)
        {

            unsigned int square = cap == CAP_SQUARE && !subpath->closed;

            addsegment(render, render->points[subpath->start + j], render->points[subpath->start + (j + 1) % subpath->count], radius, square && j == 0, square && j + 1 == segments);

        }

        for (j = 0; j < subpath->count; j++)
        {

            unsigned int end = !subpath->closed && (j == 0 || j + 1 == subpath->count);

            if (!end || cap == CAP_ROUND)
                addround(render, render->points[subpath->start + j], radius);

        }

    }

}

static unsigned int blend(unsigned int under, unsigned int color, unsigned int alpha)
{

    unsigned int rest = (under >> 24) * (255 - alpha) / 255;
    unsigned int total = alpha + rest;
    unsigned int red;
    unsigned int green;
    unsigned int blue;

    if (!total)
        return 0;

    red = (((color >> 16) & 0xFF) * alpha + ((under >> 16) & 0xFF) * rest) / total;
    green = (((color >> 8) & 0xFF) * alpha + ((under >> 8) & 0xFF) * rest) / total;
    blue = ((color & 0xFF) * alpha + (under & 0xFF) * rest) / total;

    return (total << 24) | (red << 16) | (green << 8) | blue;

}

static void cover(struct render *render, int from, int to)
{

    int limit = (int)render->canvas->width << 8;
    int first;
    int last;
    int i;

    from = (from < 0) ? 0 : from;
    to = (to > limit) ? limit : to;

    if (from >= to)
        return;

    first = from >> 8;
    last = to >> 8;

    if (first == last)
    {

        render->coverage[first] += (to - from) / SAMPLES;

        return;

    }

    render->coverage[first] += (256 - (from & 0xFF)) / SAMPLES;

    for (i = first + 1; i < last; i++)
        render->coverage[i] += 256 / SAMPLES;

    if (last < (int)render->canvas->width)
        render->coverage[last] += (to & 0xFF) / SAMPLES;

}

static void scan(struct render *render, unsigned int color, unsigned int alpha, unsigned int evenodd)
{

    struct svg_canvas *canvas = render->canvas;
    int top = 0x7FFFFFFF;
    int bottom = 0;
    int y;
    unsigned int i;

    if (!render->edgecount || !alpha)
        return;

    for (i = 0; i < render->edgecount; i++)
    {

        top = (render->edges[i].y0 < top) ? render->edges[i].y0 : top;
        bottom = (render->edges[i].y1 > bottom) ? render->edges[i].y1 : bottom;

    }

    top = (top < 0) ? 0 : top >> 8;
    bottom = ((bottom + 255) >> 8 > (int)canvas->height) ? (int)canvas->height : (bottom + 255) >> 8;

    for (y = top; y < bottom; y++)
    {

        unsigned int *row = canvas->pixels + y * canvas->width;
        int sample;
        unsigned int x;

        for (sample = 0; sample < SAMPLES; sample++)
        {

            int sy = (y << 8) + sample * (256 / SAMPLES) + 128 / SAMPLES;
            unsigned int count = 0;
            int winding = 0;

            for (i = 0; i < render->edgecount && count < CROSSINGS; i++)
            {

                struct edge *edge = &render->edges[i];

                if (edge->y0 <= sy && sy < edge->y1)
                {

                    int crossing = edge->x0 + ((sy - edge->y0) * edge->slope >> 8);
                    unsigned int j = count++;

                    while (j && render->crossings[j - 1] > crossing)
                    {

                        render->crossings[j] = render->crossings[j - 1];
                        render->windings[j] = render->windings[j - 1];
                        j--;

                    }

                    render->crossings[j] = crossing;
                    render->windings[j] = edge->winding;

                }

            }

            for (i = 0; i + 1 < count; i++)
            {

                winding += render->windings[i];

                if ((evenodd) ? (winding & 1) : winding != 0)
                    cover(render, render->crossings[i], render->crossings[i + 1]);

            }

        }

        for (x = 0; x < canvas->width; x++)
        {

            if (render->coverage[x])
            {

                unsigned int amount = (render->coverage[x] > 256) ? 256 : render->coverage[x];

                row[x] = blend(row[x], color, amount * alpha / 256);
                render->coverage[x] = 0;

            }

        }

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

        render->edgecount = 0;

        fillsubpaths(render);
        scan(render, color, (color >> 24) * state->fillopacity / 255 * state->opacity / 255, state->evenodd);

    }

    if (state->stroke.kind != PAINT_NONE && state->strokewidth > 0)
    {

        unsigned int color = resolve(&state->stroke, state->color);
        int scale = sqrtfix((int)magnitude(mulfix(state->matrix.a, state->matrix.d) - mulfix(state->matrix.b, state->matrix.c)));
        int radius = mulfix(state->strokewidth, scale) >> 9;
        unsigned int alpha = (color >> 24) * state->strokeopacity / 255 * state->opacity / 255;

        if (radius < 64)
        {

            alpha = alpha * (unsigned int)radius / 64;
            radius = 64;

        }

        render->edgecount = 0;

        strokesubpaths(render, radius, state->cap);
        scan(render, color, alpha, 0);

    }

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
    struct matrix shift;

    if (!href)
        href = html_getattribute(render->document, node, "xlink:href");

    if (!href || href[0] != '#')
        return;

    target = findid(render, href + 1, cstring_length(href + 1));

    if (!target || target == node)
        return;

    setmatrix(&shift, ONE, 0, 0, ONE, getlength(render, node, "x", render->viewwidth, 0), getlength(render, node, "y", render->viewheight, 0));
    multiply(&state->matrix, &shift);

    if (matchname(render, target, "symbol"))
    {

        applyattributes(render, state, target);
        walkchildren(render, target, state, depth + 1);

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

            struct matrix shift;

            setmatrix(&shift, ONE, 0, 0, ONE, getlength(render, node, "x", render->viewwidth, 0), getlength(render, node, "y", render->viewheight, 0));
            multiply(&state.matrix, &shift);

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

        render->pointcount = 0;
        render->subpathcount = 0;

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

static struct number readsize(char *text, unsigned int fontsize)
{

    struct reader reader;
    struct number result;

    if (!text)
        return number(0, 0);

    reader.text = text;
    reader.offset = 0;
    result = readnumber(&reader);

    if (!result.valid || result.value <= 0 || text[reader.offset] == '%')
        return number(0, 0);

    if (text[reader.offset] == 'e' && text[reader.offset + 1] == 'm')
        result.value *= (int)fontsize;

    return number((result.value + ONE / 2) >> 16, 1);

}

struct svg_size svg_measure(struct html_document *document, unsigned int root, unsigned int fontsize)
{

    struct svg_size size;
    struct number width = readsize(html_getattribute(document, root, "width"), fontsize);
    struct number height = readsize(html_getattribute(document, root, "height"), fontsize);
    char *viewbox = html_getattribute(document, root, "viewbox");
    int ratiowidth = 0;
    int ratioheight = 0;

    if (viewbox)
    {

        struct reader reader;
        int values[4];

        reader.text = viewbox;
        reader.offset = 0;

        if (readpoints(&reader, values, 4) && values[2] > 0 && values[3] > 0)
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

void svg_render(struct svg_canvas *canvas, struct html_document *document, unsigned int root, unsigned int color, unsigned int fill)
{

    struct state state;
    char *viewbox = html_getattribute(document, root, "viewbox");
    char *aspect = html_getattribute(document, root, "preserveaspectratio");
    unsigned int aspectlength = (aspect) ? cstring_length(aspect) : 0;
    int width = (int)canvas->width << 16;
    int height = (int)canvas->height << 16;

    if (!canvas->width || canvas->width > MAXWIDTH || !canvas->height)
        return;

    raster.canvas = canvas;
    raster.document = document;
    raster.viewwidth = width;
    raster.viewheight = height;

    findstyles(&raster, root);

    buffer_clear(&state, sizeof (struct state));
    setmatrix(&state.matrix, ONE, 0, 0, ONE, 0, 0);

    state.fill.kind = PAINT_COLOR;
    state.fill.color = 0xFF000000;
    state.stroke.kind = PAINT_NONE;
    state.strokewidth = ONE;
    state.color = color;
    state.fillopacity = 255;
    state.strokeopacity = 255;
    state.opacity = 255;
    state.display = 1;
    state.visible = 1;

    if (viewbox)
    {

        struct reader reader;
        int values[4];

        reader.text = viewbox;
        reader.offset = 0;

        if (readpoints(&reader, values, 4) && values[2] > 0 && values[3] > 0)
        {

            int sx = divfix(width, values[2]);
            int sy = divfix(height, values[3]);

            if (!(aspect && cstring_match(aspect, "none")))
            {

                unsigned int slice = aspectlength >= 5 && buffer_match(aspect + aspectlength - 5, "slice", 5);
                int scale = (slice) ? ((sx > sy) ? sx : sy) : ((sx < sy) ? sx : sy);

                sx = scale;
                sy = scale;

            }

            setmatrix(&state.matrix, sx, 0, 0, sy, (width - mulfix(values[2], sx)) / 2 - mulfix(values[0], sx), (height - mulfix(values[3], sy)) / 2 - mulfix(values[1], sy));

            raster.viewwidth = values[2];
            raster.viewheight = values[3];

        }

    }

    applyattributes(&raster, &state, root);

    if (fill)
    {

        state.fill.kind = (fill == CSS_CURRENTCOLOR) ? PAINT_CURRENT : ((fill >> 24) ? PAINT_COLOR : PAINT_NONE);
        state.fill.color = fill;

    }

    if (state.display)
        walkchildren(&raster, root, &state, 0);

}
