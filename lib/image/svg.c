#include <fudge.h>
#include "svg.h"

#define SAMPLES                         4
#define CURVESTEPS                      32
#define ARCSTEPS                        64
#define ROUNDSTEPS                      16
#define ATANS                           16

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

static int atans[ATANS] = {8192, 4836, 2555, 1297, 651, 326, 163, 81, 41, 20, 10, 5, 3, 1, 1, 0};

static unsigned int matchword(char *text, unsigned int length, char *word)
{

    return cstring_length(word) == length && buffer_match(text, word, length);

}

static unsigned int magnitude(int value)
{

    return (value < 0) ? (unsigned int)-value : (unsigned int)value;

}

int svg_mulfix(int a, int b)
{

    unsigned int ua = magnitude(a);
    unsigned int ub = magnitude(b);
    unsigned int result = ((ua >> 16) * (ub >> 16) << 16) + (ua >> 16) * (ub & 0xFFFF) + (ua & 0xFFFF) * (ub >> 16) + ((ua & 0xFFFF) * (ub & 0xFFFF) >> 16);

    return ((a < 0) != (b < 0)) ? -(int)result : (int)result;

}

int svg_divfix(int a, int b)
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

static struct svg_point rotation(int angle)
{

    struct svg_point result;
    unsigned int quadrant = (angle & (SVG_TURN - 1)) >> 14;
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
        z = SVG_TURN / 2;

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

    return z & (SVG_TURN - 1);

}

void svg_setmatrix(struct svg_matrix *matrix, int a, int b, int c, int d, int e, int f)
{

    matrix->a = a;
    matrix->b = b;
    matrix->c = c;
    matrix->d = d;
    matrix->e = e;
    matrix->f = f;

}

void svg_multiply(struct svg_matrix *matrix, struct svg_matrix *by)
{

    struct svg_matrix result;

    result.a = svg_mulfix(matrix->a, by->a) + svg_mulfix(matrix->c, by->b);
    result.b = svg_mulfix(matrix->b, by->a) + svg_mulfix(matrix->d, by->b);
    result.c = svg_mulfix(matrix->a, by->c) + svg_mulfix(matrix->c, by->d);
    result.d = svg_mulfix(matrix->b, by->c) + svg_mulfix(matrix->d, by->d);
    result.e = svg_mulfix(matrix->a, by->e) + svg_mulfix(matrix->c, by->f) + matrix->e;
    result.f = svg_mulfix(matrix->b, by->e) + svg_mulfix(matrix->d, by->f) + matrix->f;

    buffer_copy(matrix, &result, sizeof (struct svg_matrix));

}

static struct svg_point project(struct svg_matrix *matrix, int x, int y)
{

    struct svg_point result;

    result.x = (svg_mulfix(matrix->a, x) + svg_mulfix(matrix->c, y) + matrix->e) >> 8;
    result.y = (svg_mulfix(matrix->b, x) + svg_mulfix(matrix->d, y) + matrix->f) >> 8;

    return result;

}

static void skipseparators(struct svg_reader *reader)
{

    char c = reader->text[reader->offset];

    while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',')
        c = reader->text[++reader->offset];

}

static struct svg_number number(int value, unsigned int valid)
{

    struct svg_number result;

    result.value = value;
    result.valid = valid;

    return result;

}

struct svg_number svg_readnumber(struct svg_reader *reader)
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

    value = ((whole > 32767) ? 32767 : whole) * SVG_ONE + fraction * SVG_ONE / scale;

    for (; exponent > 0 && value < 0x7FFFFFFF / 10; exponent--)
        value *= 10;

    for (; exponent < 0; exponent++)
        value /= 10;

    reader->offset = offset;

    return number((negative) ? -value : value, 1);

}

static struct svg_number readflag(struct svg_reader *reader)
{

    char c;

    skipseparators(reader);

    c = reader->text[reader->offset];

    if (c != '0' && c != '1')
        return number(0, 0);

    reader->offset++;

    return number(c == '1', 1);

}

void svg_parsetransform(struct svg_matrix *matrix, char *text)
{

    struct svg_reader reader;

    reader.text = text;
    reader.offset = 0;

    while (text[reader.offset])
    {

        unsigned int start;
        unsigned int length;
        struct svg_matrix by;
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

            struct svg_number value = svg_readnumber(&reader);

            if (!value.valid)
                break;

            values[count++] = value.value;

        }

        skipseparators(&reader);

        if (text[reader.offset] != ')')
            return;

        reader.offset++;

        svg_setmatrix(&by, SVG_ONE, 0, 0, SVG_ONE, 0, 0);

        if (matchword(text + start, length, "matrix") && count == 6)
        {

            svg_setmatrix(&by, values[0], values[1], values[2], values[3], values[4], values[5]);

        }

        else if (matchword(text + start, length, "translate") && count)
        {

            svg_setmatrix(&by, SVG_ONE, 0, 0, SVG_ONE, values[0], (count > 1) ? values[1] : 0);

        }

        else if (matchword(text + start, length, "scale") && count)
        {

            svg_setmatrix(&by, values[0], 0, 0, (count > 1) ? values[1] : values[0], 0, 0);

        }

        else if (matchword(text + start, length, "rotate") && count)
        {

            struct svg_point unit = rotation(svg_mulfix(values[0], 11930465) >> 16);

            if (count == 3)
            {

                struct svg_matrix shift;

                svg_setmatrix(&shift, SVG_ONE, 0, 0, SVG_ONE, values[1], values[2]);
                svg_multiply(matrix, &shift);

            }

            svg_setmatrix(&by, unit.x, unit.y, -unit.y, unit.x, 0, 0);

            if (count == 3)
            {

                svg_multiply(matrix, &by);
                svg_setmatrix(&by, SVG_ONE, 0, 0, SVG_ONE, -values[1], -values[2]);

            }

        }

        else if ((matchword(text + start, length, "skewX") || matchword(text + start, length, "skewY")) && count)
        {

            struct svg_point unit = rotation(svg_mulfix(values[0], 11930465) >> 16);
            int tangent = svg_divfix(unit.y, unit.x);

            if (text[start + 4] == 'X')
                by.c = tangent;
            else
                by.b = tangent;

        }

        svg_multiply(matrix, &by);

    }

}

static void addpoint(struct svg *svg, struct svg_point point)
{

    struct svg_subpath *subpath;

    if (!svg->subpathcount || svg->pointcount >= SVG_POINTS)
        return;

    subpath = &svg->subpaths[svg->subpathcount - 1];

    if (subpath->count && svg->points[svg->pointcount - 1].x == point.x && svg->points[svg->pointcount - 1].y == point.y)
        return;

    svg->points[svg->pointcount].x = point.x;
    svg->points[svg->pointcount].y = point.y;
    svg->pointcount++;
    subpath->count++;

}

void svg_moveto(struct svg *svg, struct svg_matrix *matrix, int x, int y)
{

    struct svg_subpath *subpath;

    if (svg->subpathcount && svg->subpaths[svg->subpathcount - 1].count < 2)
    {

        svg->pointcount -= svg->subpaths[svg->subpathcount - 1].count;
        svg->subpathcount--;

    }

    if (svg->subpathcount >= SVG_SUBPATHS)
        return;

    subpath = &svg->subpaths[svg->subpathcount++];
    subpath->start = svg->pointcount;
    subpath->count = 0;
    subpath->closed = 0;

    addpoint(svg, project(matrix, x, y));

}

void svg_lineto(struct svg *svg, struct svg_matrix *matrix, int x, int y)
{

    if (!svg->subpathcount)
        svg_moveto(svg, matrix, x, y);
    else
        addpoint(svg, project(matrix, x, y));

}

void svg_closepath(struct svg *svg)
{

    if (svg->subpathcount)
        svg->subpaths[svg->subpathcount - 1].closed = 1;

}

static int lerp(int a, int b, int step, int steps)
{

    return a + (b - a) * step / steps;

}

static void cubic(struct svg *svg, struct svg_matrix *matrix, struct pen *pen, int x1, int y1, int x2, int y2, int x3, int y3)
{

    struct svg_point p0 = project(matrix, pen->x, pen->y);
    struct svg_point p1 = project(matrix, x1, y1);
    struct svg_point p2 = project(matrix, x2, y2);
    struct svg_point p3 = project(matrix, x3, y3);
    unsigned int length = (magnitude(p1.x - p0.x) + magnitude(p1.y - p0.y) + magnitude(p2.x - p1.x) + magnitude(p2.y - p1.y) + magnitude(p3.x - p2.x) + magnitude(p3.y - p2.y)) >> 8;
    int steps = (length / 3 < 2) ? 2 : (length / 3 > CURVESTEPS) ? CURVESTEPS : (int)length / 3;
    int i;

    for (i = 1; i <= steps; i++)
    {

        struct svg_point point;
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

        addpoint(svg, point);

    }

    pen->x = x3;
    pen->y = y3;

}

void svg_ellipse(struct svg *svg, struct svg_matrix *matrix, int cx, int cy, int rx, int ry, int from, int sweep, int cosine, int sine)
{

    int steps = (int)(magnitude(sweep) * ARCSTEPS / SVG_TURN) + 1;
    int i;

    for (i = 1; i <= steps; i++)
    {

        struct svg_point unit = rotation(from + sweep * i / steps);
        int px = svg_mulfix(rx, unit.x);
        int py = svg_mulfix(ry, unit.y);

        svg_lineto(svg, matrix, cx + svg_mulfix(cosine, px) - svg_mulfix(sine, py), cy + svg_mulfix(sine, px) + svg_mulfix(cosine, py));

    }

}

static void arc(struct svg *svg, struct svg_matrix *matrix, struct pen *pen, int rx, int ry, int angle, unsigned int large, unsigned int sweep, int x, int y)
{

    struct svg_point axis = rotation(svg_mulfix(angle, 11930465) >> 16);
    int dx = (pen->x - x) / 2;
    int dy = (pen->y - y) / 2;
    int x1 = svg_mulfix(axis.x, dx) + svg_mulfix(axis.y, dy);
    int y1 = svg_mulfix(axis.x, dy) - svg_mulfix(axis.y, dx);
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

        svg_lineto(svg, matrix, x, y);

        pen->x = x;
        pen->y = y;

        return;

    }

    ux = svg_divfix(x1, rx);
    uy = svg_divfix(y1, ry);
    lambda = svg_mulfix(ux, ux) + svg_mulfix(uy, uy);

    if (lambda > SVG_ONE)
    {

        int scale = sqrtfix(lambda);

        rx = svg_mulfix(rx, scale);
        ry = svg_mulfix(ry, scale);
        ux = svg_divfix(ux, scale);
        uy = svg_divfix(uy, scale);
        lambda = svg_mulfix(ux, ux) + svg_mulfix(uy, uy);

    }

    factor = (lambda > 0 && lambda < SVG_ONE) ? sqrtfix(svg_divfix(SVG_ONE - lambda, lambda)) : 0;

    if (large == sweep)
        factor = -factor;

    cx = svg_mulfix(factor, uy);
    cy = -svg_mulfix(factor, ux);
    from = direction(ux - cx, uy - cy);
    delta = (direction(-ux - cx, -uy - cy) - from) & (SVG_TURN - 1);

    if (!sweep && delta)
        delta -= SVG_TURN;

    {

        int tx = svg_mulfix(rx, cx);
        int ty = svg_mulfix(ry, cy);
        int centerx = svg_mulfix(axis.x, tx) - svg_mulfix(axis.y, ty) + (pen->x + x) / 2;
        int centery = svg_mulfix(axis.y, tx) + svg_mulfix(axis.x, ty) + (pen->y + y) / 2;

        svg_ellipse(svg, matrix, centerx, centery, rx, ry, from, delta, axis.x, axis.y);

    }

    svg_lineto(svg, matrix, x, y);

    pen->x = x;
    pen->y = y;

}

unsigned int svg_readpoints(struct svg_reader *reader, int *values, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct svg_number value = svg_readnumber(reader);

        if (!value.valid)
            return 0;

        values[i] = value.value;

    }

    return 1;

}

static unsigned int pathcommand(struct svg *svg, struct svg_matrix *matrix, struct svg_reader *reader, struct pen *pen, char command)
{

    unsigned int relative = command >= 'a';
    char upper = (relative) ? command - 32 : command;
    int ox = (relative) ? pen->x : 0;
    int oy = (relative) ? pen->y : 0;
    int values[7];
    unsigned int curve = 0;

    if (upper != 'M' && upper != 'Z' && pen->needmove)
    {

        svg_moveto(svg, matrix, pen->startx, pen->starty);

        pen->needmove = 0;

    }

    switch (upper)
    {

    case 'M':
        if (!svg_readpoints(reader, values, 2))
            return 0;

        pen->x = pen->startx = values[0] + ox;
        pen->y = pen->starty = values[1] + oy;
        pen->needmove = 0;

        svg_moveto(svg, matrix, pen->x, pen->y);

        break;

    case 'L':
        if (!svg_readpoints(reader, values, 2))
            return 0;

        pen->x = values[0] + ox;
        pen->y = values[1] + oy;

        svg_lineto(svg, matrix, pen->x, pen->y);

        break;

    case 'H':
        if (!svg_readpoints(reader, values, 1))
            return 0;

        pen->x = values[0] + ox;

        svg_lineto(svg, matrix, pen->x, pen->y);

        break;

    case 'V':
        if (!svg_readpoints(reader, values, 1))
            return 0;

        pen->y = values[0] + oy;

        svg_lineto(svg, matrix, pen->x, pen->y);

        break;

    case 'C':
        if (!svg_readpoints(reader, values, 6))
            return 0;

        pen->controlx = values[2] + ox;
        pen->controly = values[3] + oy;
        curve = 1;

        cubic(svg, matrix, pen, values[0] + ox, values[1] + oy, pen->controlx, pen->controly, values[4] + ox, values[5] + oy);

        break;

    case 'S':
        if (!svg_readpoints(reader, values, 4))
            return 0;

        values[4] = (pen->curve == 1) ? 2 * pen->x - pen->controlx : pen->x;
        values[5] = (pen->curve == 1) ? 2 * pen->y - pen->controly : pen->y;
        pen->controlx = values[0] + ox;
        pen->controly = values[1] + oy;
        curve = 1;

        cubic(svg, matrix, pen, values[4], values[5], pen->controlx, pen->controly, values[2] + ox, values[3] + oy);

        break;

    case 'Q':
    case 'T':
        if (upper == 'Q' && !svg_readpoints(reader, values, 4))
            return 0;

        if (upper == 'T')
        {

            if (!svg_readpoints(reader, values + 2, 2))
                return 0;

            values[0] = (pen->curve == 2) ? 2 * pen->x - pen->controlx - ox : pen->x - ox;
            values[1] = (pen->curve == 2) ? 2 * pen->y - pen->controly - oy : pen->y - oy;

        }

        pen->controlx = values[0] + ox;
        pen->controly = values[1] + oy;
        curve = 2;

        cubic(svg, matrix, pen, pen->x + (pen->controlx - pen->x) * 2 / 3, pen->y + (pen->controly - pen->y) * 2 / 3, values[2] + ox + (pen->controlx - values[2] - ox) * 2 / 3, values[3] + oy + (pen->controly - values[3] - oy) * 2 / 3, values[2] + ox, values[3] + oy);

        break;

    case 'A':
        {

            struct svg_number large;
            struct svg_number sweep;

            if (!svg_readpoints(reader, values, 3))
                return 0;

            large = readflag(reader);
            sweep = readflag(reader);

            if (!large.valid || !sweep.valid || !svg_readpoints(reader, values + 3, 2))
                return 0;

            arc(svg, matrix, pen, values[0], values[1], values[2], large.value, sweep.value, values[3] + ox, values[4] + oy);

        }

        break;

    case 'Z':
        svg_closepath(svg);

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

void svg_parsepath(struct svg *svg, struct svg_matrix *matrix, char *text)
{

    struct svg_reader reader;
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

        if (!pathcommand(svg, matrix, &reader, &pen, command))
            break;

        if (command == 'M')
            command = 'L';
        else if (command == 'm')
            command = 'l';

    }

}

void svg_parsepoints(struct svg *svg, struct svg_matrix *matrix, char *text, unsigned int closed)
{

    struct svg_reader reader;
    unsigned int first = 1;

    reader.text = text;
    reader.offset = 0;

    while (1)
    {

        int values[2];

        if (!svg_readpoints(&reader, values, 2))
            break;

        if (first)
            svg_moveto(svg, matrix, values[0], values[1]);
        else
            svg_lineto(svg, matrix, values[0], values[1]);

        first = 0;

    }

    if (closed)
        svg_closepath(svg);

}

static void addedge(struct svg *svg, struct svg_point a, struct svg_point b)
{

    struct svg_edge *edge;

    if (a.y == b.y || svg->edgecount >= SVG_EDGES)
        return;

    edge = &svg->edges[svg->edgecount++];

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

static void fillsubpaths(struct svg *svg)
{

    unsigned int i;

    for (i = 0; i < svg->subpathcount; i++)
    {

        struct svg_subpath *subpath = &svg->subpaths[i];
        unsigned int j;

        if (subpath->count < 2)
            continue;

        for (j = 0; j < subpath->count; j++)
            addedge(svg, svg->points[subpath->start + j], svg->points[subpath->start + (j + 1) % subpath->count]);

    }

}

static void addround(struct svg *svg, struct svg_point center, int radius)
{

    struct svg_point previous;
    unsigned int i;

    previous.x = center.x + radius;
    previous.y = center.y;

    for (i = 1; i <= ROUNDSTEPS; i++)
    {

        struct svg_point unit = rotation(-(int)(i * SVG_TURN / ROUNDSTEPS));
        struct svg_point next;

        next.x = center.x + svg_mulfix(unit.x, radius);
        next.y = center.y + svg_mulfix(unit.y, radius);

        addedge(svg, previous, next);

        previous.x = next.x;
        previous.y = next.y;

    }

}

static void addsegment(struct svg *svg, struct svg_point a, struct svg_point b, int radius, unsigned int startcap, unsigned int endcap)
{

    int dx = b.x - a.x;
    int dy = b.y - a.y;
    unsigned int shift = 0;
    unsigned int length;
    struct svg_point corners[4];
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

    addedge(svg, corners[0], corners[1]);
    addedge(svg, corners[1], corners[2]);
    addedge(svg, corners[2], corners[3]);
    addedge(svg, corners[3], corners[0]);

}

static void strokesubpaths(struct svg *svg, int radius, unsigned int cap)
{

    unsigned int i;

    for (i = 0; i < svg->subpathcount; i++)
    {

        struct svg_subpath *subpath = &svg->subpaths[i];
        unsigned int segments = (subpath->closed && subpath->count > 2) ? subpath->count : subpath->count - 1;
        unsigned int j;

        if (subpath->count < 2)
            continue;

        for (j = 0; j < segments; j++)
        {

            unsigned int square = cap == SVG_CAP_SQUARE && !subpath->closed;

            addsegment(svg, svg->points[subpath->start + j], svg->points[subpath->start + (j + 1) % subpath->count], radius, square && j == 0, square && j + 1 == segments);

        }

        for (j = 0; j < subpath->count; j++)
        {

            unsigned int end = !subpath->closed && (j == 0 || j + 1 == subpath->count);

            if (!end || cap == SVG_CAP_ROUND)
                addround(svg, svg->points[subpath->start + j], radius);

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

static void cover(struct svg *svg, int from, int to)
{

    int limit = (int)svg->width << 8;
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

        svg->coverage[first] += (to - from) / SAMPLES;

        return;

    }

    svg->coverage[first] += (256 - (from & 0xFF)) / SAMPLES;

    for (i = first + 1; i < last; i++)
        svg->coverage[i] += 256 / SAMPLES;

    if (last < (int)svg->width)
        svg->coverage[last] += (to & 0xFF) / SAMPLES;

}

static void scan(struct svg *svg, unsigned int color, unsigned int alpha, unsigned int evenodd)
{

    int top = 0x7FFFFFFF;
    int bottom = 0;
    int y;
    unsigned int i;

    if (!svg->edgecount || !alpha)
        return;

    for (i = 0; i < svg->edgecount; i++)
    {

        top = (svg->edges[i].y0 < top) ? svg->edges[i].y0 : top;
        bottom = (svg->edges[i].y1 > bottom) ? svg->edges[i].y1 : bottom;

    }

    top = (top < 0) ? 0 : top >> 8;
    bottom = ((bottom + 255) >> 8 > (int)svg->height) ? (int)svg->height : (bottom + 255) >> 8;

    for (y = top; y < bottom; y++)
    {

        unsigned int *row = svg->pixels + y * svg->width;
        int sample;
        unsigned int x;

        for (sample = 0; sample < SAMPLES; sample++)
        {

            int sy = (y << 8) + sample * (256 / SAMPLES) + 128 / SAMPLES;
            unsigned int count = 0;
            int winding = 0;

            for (i = 0; i < svg->edgecount && count < SVG_CROSSINGS; i++)
            {

                struct svg_edge *edge = &svg->edges[i];

                if (edge->y0 <= sy && sy < edge->y1)
                {

                    int crossing = edge->x0 + ((sy - edge->y0) * edge->slope >> 8);
                    unsigned int j = count++;

                    while (j && svg->crossings[j - 1] > crossing)
                    {

                        svg->crossings[j] = svg->crossings[j - 1];
                        svg->windings[j] = svg->windings[j - 1];
                        j--;

                    }

                    svg->crossings[j] = crossing;
                    svg->windings[j] = edge->winding;

                }

            }

            for (i = 0; i + 1 < count; i++)
            {

                winding += svg->windings[i];

                if ((evenodd) ? (winding & 1) : winding != 0)
                    cover(svg, svg->crossings[i], svg->crossings[i + 1]);

            }

        }

        for (x = 0; x < svg->width; x++)
        {

            if (svg->coverage[x])
            {

                unsigned int amount = (svg->coverage[x] > 256) ? 256 : svg->coverage[x];

                row[x] = blend(row[x], color, amount * alpha / 256);
                svg->coverage[x] = 0;

            }

        }

    }

}

void svg_reset(struct svg *svg)
{

    svg->pointcount = 0;
    svg->subpathcount = 0;

}

void svg_fill(struct svg *svg, unsigned int color, unsigned int alpha, unsigned int evenodd)
{

    svg->edgecount = 0;

    fillsubpaths(svg);
    scan(svg, color, alpha, evenodd);

}

void svg_stroke(struct svg *svg, struct svg_matrix *matrix, int width, unsigned int cap, unsigned int color, unsigned int alpha)
{

    int scale = sqrtfix((int)magnitude(svg_mulfix(matrix->a, matrix->d) - svg_mulfix(matrix->b, matrix->c)));
    int radius = svg_mulfix(width, scale) >> 9;

    /* a line thinner than a quarter pixel is drawn that wide and fainter */
    if (radius < 64)
    {

        alpha = alpha * (unsigned int)((radius > 0) ? radius : 0) / 64;
        radius = 64;

    }

    svg->edgecount = 0;

    strokesubpaths(svg, radius, cap);
    scan(svg, color, alpha, 0);

}

