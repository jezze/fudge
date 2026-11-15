#include "toffee.h"

struct parser
{

    char *text;
    unsigned int count;
    unsigned int offset;

};

struct token
{

    unsigned int start;
    unsigned int length;

};

struct number
{

    unsigned int value;
    unsigned int valid;

};

static unsigned int isgap(char c)
{

    return c == ' ' || c == '\t' || c == '\r';

}

static void skipblanks(struct parser *parser)
{

    while (parser->offset < parser->count && isgap(parser->text[parser->offset]))
        parser->offset++;

}

static struct token nexttoken(struct parser *parser)
{

    struct token token;

    skipblanks(parser);

    token.start = parser->offset;

    while (parser->offset < parser->count && parser->text[parser->offset] != '\n' && !isgap(parser->text[parser->offset]))
        parser->offset++;

    token.length = parser->offset - token.start;

    return token;

}

static unsigned int endline(struct parser *parser)
{

    skipblanks(parser);

    if (parser->offset == parser->count)
        return 1;

    if (parser->text[parser->offset] != '\n')
        return 0;

    parser->offset++;

    return 1;

}

static unsigned int matchword(struct parser *parser, struct token token, char *word, unsigned int length)
{

    unsigned int i;

    if (token.length != length)
        return 0;

    for (i = 0; i < length; i++)
    {

        if (parser->text[token.start + i] != word[i])
            return 0;

    }

    return 1;

}

static unsigned int getdigit(char c, unsigned int base)
{

    if (c >= '0' && c <= '9')
        return c - '0';

    if (base == 16 && c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    if (base == 16 && c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    return base;

}

static struct number readnumber(struct parser *parser, struct token token, unsigned int base, unsigned int max)
{

    struct number number;
    unsigned int i;

    number.value = 0;
    number.valid = token.length > 0 && token.length <= 8;

    for (i = 0; number.valid && i < token.length; i++)
    {

        unsigned int digit = getdigit(parser->text[token.start + i], base);

        if (digit >= base)
            number.valid = 0;
        else
            number.value = number.value * base + digit;

    }

    if (number.value > max)
        number.valid = 0;

    return number;

}

static unsigned int readpoint(struct toffee *toffee, struct parser *parser, unsigned int move)
{

    struct number x = readnumber(parser, nexttoken(parser), 10, toffee->width);
    struct number y = readnumber(parser, nexttoken(parser), 10, toffee->height);
    struct toffee_point *point;

    if (!x.valid || !y.valid || toffee->npoints >= TOFFEE_POINTS)
        return 0;

    point = &toffee->points[toffee->npoints];
    point->x = x.value;
    point->y = y.value;
    point->move = move;
    toffee->npoints++;

    return 1;

}

static unsigned int readcolor(struct toffee_shape *shape, struct parser *parser, struct token token)
{

    struct number number;

    if (token.length == 2 && parser->text[token.start] == '@')
    {

        token.start++;
        token.length--;

        number = readnumber(parser, token, 10, TOFFEE_SLOTS - 1);
        shape->color = 0;
        shape->slot = number.value;

    }

    else
    {

        number = readnumber(parser, token, 16, 0xFFFFFFFF);
        number.valid = number.valid && token.length == 8;
        shape->color = number.value;
        shape->slot = TOFFEE_SLOTS;

    }

    return number.valid;

}

static unsigned int readshape(struct toffee *toffee, struct parser *parser, struct token token)
{

    struct toffee_shape *shape;
    unsigned int open = 0;

    if (toffee->nshapes >= TOFFEE_SHAPES)
        return 0;

    shape = &toffee->shapes[toffee->nshapes];
    shape->start = toffee->npoints;

    if (!readcolor(shape, parser, token))
        return 0;

    for (token = nexttoken(parser); token.length; token = nexttoken(parser))
    {

        if (matchword(parser, token, "M", 1))
        {

            if (!readpoint(toffee, parser, 1))
                return 0;

            open = 1;

        }

        else if (open && matchword(parser, token, "L", 1))
        {

            if (!readpoint(toffee, parser, 0))
                return 0;

        }

        else if (open && matchword(parser, token, "Z", 1))
        {

            open = 0;

        }

        else
        {

            return 0;

        }

    }

    shape->count = toffee->npoints - shape->start;
    toffee->nshapes++;

    return shape->count && endline(parser);

}

unsigned int toffee_parse(struct toffee *toffee, char *text, unsigned int count)
{

    struct parser parser;
    struct number width;
    struct number height;

    parser.text = text;
    parser.count = count;
    parser.offset = 0;
    toffee->width = 0;
    toffee->height = 0;
    toffee->nshapes = 0;
    toffee->npoints = 0;

    if (!matchword(&parser, nexttoken(&parser), "toffee", 6))
        return 0;

    width = readnumber(&parser, nexttoken(&parser), 10, TOFFEE_GRID);
    height = readnumber(&parser, nexttoken(&parser), 10, TOFFEE_GRID);

    if (!width.valid || !height.valid || !width.value || !height.value || !endline(&parser))
        return 0;

    toffee->width = width.value;
    toffee->height = height.value;

    while (parser.offset < parser.count)
    {

        struct token token = nexttoken(&parser);

        if (token.length)
        {

            if (!readshape(toffee, &parser, token))
                return 0;

        }

        else if (!endline(&parser))
        {

            return 0;

        }

    }

    return 1;

}

static void addcrossing(struct toffee_line *scan, int x)
{

    unsigned int i;

    if (scan->ncrossings >= TOFFEE_CROSSINGS)
        return;

    for (i = scan->ncrossings; i > 0 && scan->crossings[i - 1] > x; i--)
        scan->crossings[i] = scan->crossings[i - 1];

    scan->crossings[i] = x;
    scan->ncrossings++;

}

static int scale(int value, unsigned int size, unsigned int grid)
{

    return value * (int)size * TOFFEE_SUBPIXELS / (int)grid;

}

static void crossedge(struct toffee *toffee, struct toffee_line *scan, struct toffee_point *p0, struct toffee_point *p1, unsigned int width, unsigned int height, int sy)
{

    int xa = scale(p0->x, width, toffee->width);
    int ya = scale(p0->y, height, toffee->height);
    int xb = scale(p1->x, width, toffee->width);
    int yb = scale(p1->y, height, toffee->height);
    int dx;
    unsigned int offset;

    if (ya > yb)
    {

        int t = xa;

        xa = xb;
        xb = t;
        t = ya;
        ya = yb;
        yb = t;

    }

    if (sy < ya || sy >= yb)
        return;

    dx = xb - xa;
    offset = (unsigned int)(sy - ya) * (unsigned int)((dx < 0) ? -dx : dx) / (unsigned int)(yb - ya);

    addcrossing(scan, (dx < 0) ? xa - (int)offset : xa + (int)offset);

}

void toffee_scanline(struct toffee *toffee, struct toffee_line *scan, unsigned int index, unsigned int width, unsigned int height, unsigned int line)
{

    struct toffee_shape *shape = &toffee->shapes[index];
    unsigned int end = shape->start + shape->count;
    unsigned int first = shape->start;
    int sy = (int)line * TOFFEE_SUBPIXELS + TOFFEE_SUBPIXELS / 2;
    unsigned int next;
    unsigned int i;

    scan->ncrossings = 0;
    scan->count = 0;

    for (i = shape->start; i < end; i++)
    {

        if (toffee->points[i].move)
            first = i;

        next = (i + 1 < end && !toffee->points[i + 1].move) ? i + 1 : first;

        crossedge(toffee, scan, &toffee->points[i], &toffee->points[next], width, height, sy);

    }

    for (i = 0; i + 1 < scan->ncrossings; i += 2)
    {

        struct toffee_span *span = &scan->spans[scan->count];

        span->x0 = (scan->crossings[i] + TOFFEE_SUBPIXELS / 2 - 1) / TOFFEE_SUBPIXELS;
        span->x2 = (scan->crossings[i + 1] + TOFFEE_SUBPIXELS / 2 - 1) / TOFFEE_SUBPIXELS;

        if (span->x2 > (int)width)
            span->x2 = width;

        if (span->x0 < span->x2)
            scan->count++;

    }

}

