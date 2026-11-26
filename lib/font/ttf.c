#include "ttf.h"

#define FLAG_ONCURVE                    0x01
#define FLAG_XSHORT                     0x02
#define FLAG_YSHORT                     0x04
#define FLAG_REPEAT                     0x08
#define FLAG_XSAME                      0x10
#define FLAG_YSAME                      0x20
#define COMPONENT_WORDS                 0x0001
#define COMPONENT_XY                    0x0002
#define COMPONENT_SCALE                 0x0008
#define COMPONENT_MORE                  0x0020
#define COMPONENT_XYSCALE               0x0040
#define COMPONENT_TWOBYTWO              0x0080

struct transform
{

    int a;
    int b;
    int c;
    int d;
    int dx;
    int dy;

};

struct pen
{

    struct ttf_raster *raster;
    int x;
    int y;

};

static unsigned int read8(struct ttf_font *font, unsigned int offset)
{

    return (offset < font->size) ? font->data[offset] : 0;

}

static unsigned int read16(struct ttf_font *font, unsigned int offset)
{

    return (read8(font, offset) << 8) | read8(font, offset + 1);

}

static unsigned int read32(struct ttf_font *font, unsigned int offset)
{

    return (read16(font, offset) << 16) | read16(font, offset + 2);

}

static int readshort(struct ttf_font *font, unsigned int offset)
{

    unsigned int value = read16(font, offset);

    return (value & 0x8000) ? (int)value - 0x10000 : (int)value;

}

static unsigned int findtable(struct ttf_font *font, char *tag)
{

    unsigned int count = read16(font, 4);
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int record = 12 + i * 16;

        if (read8(font, record) == (unsigned char)tag[0] && read8(font, record + 1) == (unsigned char)tag[1] && read8(font, record + 2) == (unsigned char)tag[2] && read8(font, record + 3) == (unsigned char)tag[3])
            return read32(font, record + 8);

    }

    return 0;

}

static void findcmap(struct ttf_font *font, unsigned int cmap)
{

    unsigned int count = read16(font, cmap + 2);
    unsigned int i;

    /* full unicode tables are preferred over the basic plane ones */
    for (i = 0; i < count; i++)
    {

        unsigned int record = cmap + 4 + i * 8;
        unsigned int platform = read16(font, record);
        unsigned int subtable = cmap + read32(font, record + 4);
        unsigned int format = read16(font, subtable);

        if (platform != 0 && platform != 3)
            continue;

        if (format == 12 || (format == 4 && font->cmapformat != 12))
        {

            font->cmap = subtable;
            font->cmapformat = format;

        }

    }

}

static void findkern(struct ttf_font *font, unsigned int kern)
{

    font->kern = 0;
    font->kernpairs = 0;

    /* only the first subtable is used, and only when it holds horizontal pairs to look up */
    if (!kern || read16(font, kern) != 0 || !read16(font, kern + 2) || read16(font, kern + 8) != 0x0001)
        return;

    font->kern = kern + 18;
    font->kernpairs = read16(font, kern + 10);

}

unsigned int ttf_init(struct ttf_font *font, char *data, unsigned int size)
{

    unsigned int head;
    unsigned int hhea;
    unsigned int maxp;
    unsigned int cmap;

    font->data = (unsigned char *)data;
    font->size = size;
    font->cmap = 0;
    font->cmapformat = 0;

    head = findtable(font, "head");
    hhea = findtable(font, "hhea");
    maxp = findtable(font, "maxp");
    cmap = findtable(font, "cmap");
    font->loca = findtable(font, "loca");
    font->glyf = findtable(font, "glyf");
    font->hmtx = findtable(font, "hmtx");

    if (!head || !hhea || !maxp || !cmap || !font->loca || !font->glyf || !font->hmtx)
        return 0;

    font->unitsperem = read16(font, head + 18);
    font->longloca = read16(font, head + 50);
    font->ascender = readshort(font, hhea + 4);
    font->descender = readshort(font, hhea + 6);
    font->linegap = readshort(font, hhea + 8);
    font->hmetrics = read16(font, hhea + 34);
    font->glyphs = read16(font, maxp + 4);
    font->boxwidth = readshort(font, head + 40) - readshort(font, head + 36);
    font->averagewidth = (findtable(font, "OS/2")) ? readshort(font, findtable(font, "OS/2") + 2) : font->boxwidth / 2;

    findcmap(font, cmap);
    findkern(font, findtable(font, "kern"));

    return font->cmap && font->unitsperem && font->hmetrics;

}

static unsigned int findglyph4(struct ttf_font *font, unsigned int codepoint)
{

    unsigned int cmap = font->cmap;
    unsigned int segments = read16(font, cmap + 6);
    unsigned int i;

    if (codepoint > 0xFFFF)
        return 0;

    for (i = 0; i < segments; i += 2)
    {

        unsigned int start;
        unsigned int delta;
        unsigned int range;
        unsigned int glyph;

        if (codepoint > read16(font, cmap + 14 + i))
            continue;

        start = read16(font, cmap + 16 + segments + i);

        if (codepoint < start)
            return 0;

        delta = read16(font, cmap + 16 + segments * 2 + i);
        range = read16(font, cmap + 16 + segments * 3 + i);

        if (!range)
            return (codepoint + delta) & 0xFFFF;

        glyph = read16(font, cmap + 16 + segments * 3 + i + range + (codepoint - start) * 2);

        return (glyph) ? (glyph + delta) & 0xFFFF : 0;

    }

    return 0;

}

static unsigned int findglyph12(struct ttf_font *font, unsigned int codepoint)
{

    unsigned int cmap = font->cmap;
    unsigned int count = read32(font, cmap + 12);
    unsigned int low = 0;
    unsigned int high = count;

    while (low < high)
    {

        unsigned int middle = (low + high) / 2;
        unsigned int group = cmap + 16 + middle * 12;
        unsigned int start = read32(font, group);
        unsigned int end = read32(font, group + 4);

        if (codepoint < start)
            high = middle;
        else if (codepoint > end)
            low = middle + 1;
        else
            return read32(font, group + 8) + codepoint - start;

    }

    return 0;

}

unsigned int ttf_findglyph(struct ttf_font *font, unsigned int codepoint)
{

    unsigned int glyph = (font->cmapformat == 12) ? findglyph12(font, codepoint) : findglyph4(font, codepoint);

    return (glyph < font->glyphs) ? glyph : 0;

}

unsigned int ttf_getadvance(struct ttf_font *font, unsigned int glyph)
{

    unsigned int index = (glyph < font->hmetrics) ? glyph : font->hmetrics - 1;

    return read16(font, font->hmtx + index * 4);

}

int ttf_getkerning(struct ttf_font *font, unsigned int left, unsigned int right)
{

    unsigned int key = (left << 16) | right;
    unsigned int low = 0;
    unsigned int high = font->kernpairs;

    while (low < high)
    {

        unsigned int middle = (low + high) / 2;
        unsigned int pair = read32(font, font->kern + middle * 6);

        if (key < pair)
            high = middle;
        else if (key > pair)
            low = middle + 1;
        else
            return readshort(font, font->kern + middle * 6 + 4);

    }

    return 0;

}

static unsigned int findoutline(struct ttf_font *font, unsigned int glyph)
{

    if (font->longloca)
        return (read32(font, font->loca + glyph * 4) != read32(font, font->loca + glyph * 4 + 4)) ? font->glyf + read32(font, font->loca + glyph * 4) : 0;

    return (read16(font, font->loca + glyph * 2) != read16(font, font->loca + glyph * 2 + 2)) ? font->glyf + read16(font, font->loca + glyph * 2) * 2 : 0;

}

static struct ttf_point project(struct transform *transform, int x, int y, unsigned int on)
{

    struct ttf_point point;

    point.x = ((transform->a * x + transform->c * y) >> 14) + transform->dx;
    point.y = ((transform->b * x + transform->d * y) >> 14) + transform->dy;
    point.on = on;

    return point;

}

static unsigned int loadsimple(struct ttf_font *font, struct ttf_outline *outline, unsigned int offset, unsigned int contours, struct transform *transform)
{

    unsigned int base = outline->pointcount;
    unsigned int count = read16(font, offset + 10 + (contours - 1) * 2) + 1;
    unsigned int cursor = offset + 12 + contours * 2 + read16(font, offset + 10 + contours * 2);
    struct ttf_point *points = outline->points + base;
    int x = 0;
    int y = 0;
    unsigned int i;

    if (base + count > TTF_POINTS || outline->contourcount + contours > TTF_CONTOURS)
        return 0;

    for (i = 0; i < contours; i++)
        outline->ends[outline->contourcount + i] = base + read16(font, offset + 10 + i * 2);

    for (i = 0; i < count; )
    {

        unsigned int flag = read8(font, cursor++);
        unsigned int repeat = (flag & FLAG_REPEAT) ? read8(font, cursor++) : 0;

        /* the flags wait in the points until the coordinates are read */
        points[i++].on = flag;

        while (repeat-- && i < count)
            points[i++].on = flag;

    }

    for (i = 0; i < count; i++)
    {

        if (points[i].on & FLAG_XSHORT)
            x += (points[i].on & FLAG_XSAME) ? (int)read8(font, cursor++) : -(int)read8(font, cursor++);
        else if (!(points[i].on & FLAG_XSAME))
            x += readshort(font, (cursor += 2) - 2);

        points[i].x = x;

    }

    for (i = 0; i < count; i++)
    {

        if (points[i].on & FLAG_YSHORT)
            y += (points[i].on & FLAG_YSAME) ? (int)read8(font, cursor++) : -(int)read8(font, cursor++);
        else if (!(points[i].on & FLAG_YSAME))
            y += readshort(font, (cursor += 2) - 2);

        points[i] = project(transform, points[i].x, y, points[i].on & FLAG_ONCURVE);

    }

    outline->pointcount += count;
    outline->contourcount += contours;

    return 1;

}

static unsigned int loadglyph(struct ttf_font *font, struct ttf_outline *outline, unsigned int glyph, struct transform *transform, unsigned int depth);

static unsigned int loadcomposite(struct ttf_font *font, struct ttf_outline *outline, unsigned int offset, struct transform *transform, unsigned int depth)
{

    unsigned int cursor = offset + 10;
    unsigned int flags = COMPONENT_MORE;

    while (flags & COMPONENT_MORE)
    {

        struct transform local;
        struct transform combined;
        unsigned int glyph;
        int dx;
        int dy;

        flags = read16(font, cursor);
        glyph = read16(font, cursor + 2);
        cursor += 4;

        if (flags & COMPONENT_WORDS)
        {

            dx = readshort(font, cursor);
            dy = readshort(font, cursor + 2);
            cursor += 4;

        }

        else
        {

            dx = (read8(font, cursor) & 0x80) ? (int)read8(font, cursor) - 0x100 : (int)read8(font, cursor);
            dy = (read8(font, cursor + 1) & 0x80) ? (int)read8(font, cursor + 1) - 0x100 : (int)read8(font, cursor + 1);
            cursor += 2;

        }

        /* components anchored by point numbers are rare and placed without an offset */
        local.dx = (flags & COMPONENT_XY) ? dx : 0;
        local.dy = (flags & COMPONENT_XY) ? dy : 0;
        local.a = 0x4000;
        local.b = 0;
        local.c = 0;
        local.d = 0x4000;

        if (flags & COMPONENT_SCALE)
        {

            local.a = readshort(font, cursor);
            local.d = local.a;
            cursor += 2;

        }

        else if (flags & COMPONENT_XYSCALE)
        {

            local.a = readshort(font, cursor);
            local.d = readshort(font, cursor + 2);
            cursor += 4;

        }

        else if (flags & COMPONENT_TWOBYTWO)
        {

            local.a = readshort(font, cursor);
            local.b = readshort(font, cursor + 2);
            local.c = readshort(font, cursor + 4);
            local.d = readshort(font, cursor + 6);
            cursor += 8;

        }

        combined.a = (transform->a * local.a + transform->c * local.b) >> 14;
        combined.b = (transform->b * local.a + transform->d * local.b) >> 14;
        combined.c = (transform->a * local.c + transform->c * local.d) >> 14;
        combined.d = (transform->b * local.c + transform->d * local.d) >> 14;
        combined.dx = ((transform->a * local.dx + transform->c * local.dy) >> 14) + transform->dx;
        combined.dy = ((transform->b * local.dx + transform->d * local.dy) >> 14) + transform->dy;

        if (!loadglyph(font, outline, glyph, &combined, depth + 1))
            return 0;

    }

    return 1;

}

static unsigned int loadglyph(struct ttf_font *font, struct ttf_outline *outline, unsigned int glyph, struct transform *transform, unsigned int depth)
{

    unsigned int offset;
    int contours;

    if (glyph >= font->glyphs || depth > TTF_DEPTH)
        return 0;

    offset = findoutline(font, glyph);

    if (!offset)
        return 1;

    contours = readshort(font, offset);

    if (contours > 0)
        return loadsimple(font, outline, offset, contours, transform);

    if (contours < 0)
        return loadcomposite(font, outline, offset, transform, depth);

    return 1;

}

unsigned int ttf_loadoutline(struct ttf_font *font, struct ttf_outline *outline, unsigned int glyph)
{

    struct transform identity;

    identity.a = 0x4000;
    identity.b = 0;
    identity.c = 0;
    identity.d = 0x4000;
    identity.dx = 0;
    identity.dy = 0;
    outline->pointcount = 0;
    outline->contourcount = 0;

    return loadglyph(font, outline, glyph, &identity, 0);

}

static int crossing(int x0, int y0, int x1, int y1, int y)
{

    int dy = y1 - y0;
    int dx = x1 - x0;

    return x0 + (y - y0) * (dx / dy) + (y - y0) * (dx % dy) / dy;

}

static void accumulate(struct ttf_raster *raster, int row, int xa, int xb, int d)
{

    int *cells = raster->cells + row * (int)raster->width;
    int x0 = (xa < xb) ? xa : xb;
    int x1 = (xa < xb) ? xb : xa;
    int first;
    int last;

    x0 = (x0 < 0) ? 0 : x0;
    x1 = (x1 < 0) ? 0 : x1;
    first = x0 >> 8;
    last = (x1 + 255) >> 8;

    /* the area under each part of the edge goes to its cell, what is left carries to the right */
    if (last <= first + 1)
    {

        int middle = ((x0 + x1) >> 1) - (first << 8);

        cells[first] += d * (256 - middle);
        cells[first + 1] += d * middle;

    }

    else
    {

        int width = x1 - x0;
        int start = x0 - (first << 8);
        int end = x1 - ((last - 1) << 8);
        int a0 = (256 - start) * (256 - start) / (2 * width);
        int am = end * end / (2 * width);

        cells[first] += d * a0;

        if (last == first + 2)
        {

            cells[first + 1] += d * (256 - a0 - am);

        }

        else
        {

            int step = 65536 / width;
            int a1 = (384 - start) * 256 / width;
            int a2 = a1 + (last - first - 3) * step;
            int i;

            cells[first + 1] += d * (a1 - a0);

            for (i = first + 2; i < last - 1; i++)
                cells[i] += d * step;

            cells[last - 1] += d * (256 - a2 - am);

        }

        cells[last] += d * am;

    }

}

static void drawline(struct ttf_raster *raster, int x0, int y0, int x1, int y1)
{

    int direction = 1;
    int top;
    int bottom;
    int row;

    if (y0 == y1)
        return;

    if (y0 > y1)
    {

        int t = x0;

        x0 = x1;
        x1 = t;
        t = y0;
        y0 = y1;
        y1 = t;
        direction = -1;

    }

    top = (y0 < 0) ? 0 : y0 >> 8;
    bottom = ((y1 + 255) >> 8 > (int)raster->height) ? (int)raster->height : (y1 + 255) >> 8;

    for (row = top; row < bottom; row++)
    {

        int from = (row << 8 > y0) ? row << 8 : y0;
        int to = ((row + 1) << 8 < y1) ? (row + 1) << 8 : y1;

        if (to > from)
            accumulate(raster, row, crossing(x0, y0, x1, y1, from), crossing(x0, y0, x1, y1, to), (to - from) * direction);

    }

}

static void moveto(struct pen *pen, int x, int y)
{

    pen->x = x;
    pen->y = y;

}

static void lineto(struct pen *pen, int x, int y)
{

    drawline(pen->raster, pen->x, pen->y, x, y);

    pen->x = x;
    pen->y = y;

}

static void curveto(struct pen *pen, int cx, int cy, int x, int y)
{

    int length = ((cx > pen->x) ? cx - pen->x : pen->x - cx) + ((cy > pen->y) ? cy - pen->y : pen->y - cy) + ((x > cx) ? x - cx : cx - x) + ((y > cy) ? y - cy : cy - y);
    int steps = 1 + length / 768;
    int sx = pen->x;
    int sy = pen->y;
    int i;

    steps = (steps > 16) ? 16 : steps;

    for (i = 1; i <= steps; i++)
    {

        int u = steps - i;

        lineto(pen, (sx * u * u + 2 * cx * u * i + x * i * i) / (steps * steps), (sy * u * u + 2 * cy * u * i + y * i * i) / (steps * steps));

    }

}

static void drawcontour(struct pen *pen, struct ttf_point *points, unsigned int count)
{

    unsigned int first = 0;
    unsigned int pending = 0;
    int startx;
    int starty;
    int cx = 0;
    int cy = 0;
    unsigned int i;

    while (first < count && !points[first].on)
        first++;

    /* a contour of only control points starts between the first two */
    if (first == count)
    {

        first = 0;
        startx = (points[0].x + points[1 % count].x) / 2;
        starty = (points[0].y + points[1 % count].y) / 2;

    }

    else
    {

        startx = points[first].x;
        starty = points[first].y;

    }

    moveto(pen, startx, starty);

    for (i = 1; i <= count; i++)
    {

        struct ttf_point *point = &points[(first + i) % count];

        if (point->on)
        {

            if (pending)
                curveto(pen, cx, cy, point->x, point->y);
            else
                lineto(pen, point->x, point->y);

            pending = 0;

            continue;

        }

        /* two control points in a row have an implied point halfway between them */
        if (pending)
            curveto(pen, cx, cy, (cx + point->x) / 2, (cy + point->y) / 2);

        cx = point->x;
        cy = point->y;
        pending = 1;

    }

    if (pending)
        curveto(pen, cx, cy, startx, starty);

    lineto(pen, startx, starty);

}

struct ttf_box ttf_render(struct ttf_raster *raster, struct ttf_outline *outline, unsigned int size, unsigned int unitsperem, unsigned char *coverage, unsigned int limit)
{

    struct ttf_box box;
    struct pen pen;
    int minx = 0x7FFFFFFF;
    int miny = 0x7FFFFFFF;
    int maxx = -0x7FFFFFFF;
    int maxy = -0x7FFFFFFF;
    unsigned int start = 0;
    unsigned int i;
    int sum = 0;

    box.left = 0;
    box.top = 0;
    box.width = 0;
    box.height = 0;

    /* points become 1/256 pixel with y growing down from the baseline */
    for (i = 0; i < outline->pointcount; i++)
    {

        struct ttf_point *point = &outline->points[i];

        point->x = point->x * (int)size * 256 / (int)unitsperem;
        point->y = -point->y * (int)size * 256 / (int)unitsperem;
        minx = (point->x < minx) ? point->x : minx;
        miny = (point->y < miny) ? point->y : miny;
        maxx = (point->x > maxx) ? point->x : maxx;
        maxy = (point->y > maxy) ? point->y : maxy;

    }

    if (!outline->pointcount)
        return box;

    box.left = (minx >= 0) ? minx >> 8 : -((-minx + 255) >> 8);
    box.top = (miny >= 0) ? miny >> 8 : -((-miny + 255) >> 8);
    box.width = ((maxx + 255) >> 8) - box.left;
    box.height = ((maxy + 255) >> 8) - box.top;

    if (!box.width || !box.height || box.width >= TTF_SIZE || box.height >= TTF_SIZE || box.width * box.height > limit)
    {

        box.width = 0;
        box.height = 0;

        return box;

    }

    raster->width = box.width;
    raster->height = box.height;

    for (i = 0; i < box.width * box.height + 2; i++)
        raster->cells[i] = 0;

    pen.raster = raster;

    for (i = 0; i < outline->pointcount; i++)
    {

        outline->points[i].x -= box.left << 8;
        outline->points[i].y -= box.top << 8;

    }

    for (i = 0; i < outline->contourcount; i++)
    {

        unsigned int end = outline->ends[i] + 1;

        if (end > start && end <= outline->pointcount)
            drawcontour(&pen, outline->points + start, end - start);

        start = end;

    }

    for (i = 0; i < box.width * box.height; i++)
    {

        int value;

        sum += raster->cells[i];
        value = ((sum < 0) ? -sum : sum) >> 8;
        coverage[i] = (value > 255) ? 255 : value;

    }

    return box;

}

