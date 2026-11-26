#include <fudge.h>
#include "html.h"
#include "css.h"
#include <font.h>
#include "font.h"
#include "layout.h"

#define COLUMNS                         16
#define FLEXLINES                       64
#define GRIDCOLUMNS                     32
#define GRIDROWS                        128
#define MEASUREDEPTH                    8
#define MEASUREWIDTH                    8192
#define NONE                            0xFFFFFFFF
#define FLOW_NORMAL                     0
#define FLOW_FLEXROW                    1
#define LINE_CENTER                     4
#define FLOW_FLEXCOLUMN                 2
#define LABELSIZE                       128

struct flow
{

    int boxleft;
    int boxright;
    int left;
    int right;
    int x;
    int y;
    int pending;
    unsigned int itemstart;
    unsigned int space;
    unsigned int node;
    unsigned int type;
    unsigned int collapsed;

};

struct metrics
{

    struct font_choice choice;
    int glyphheight;
    int glyphascent;
    int ascent;
    int descent;
    int space;
    int spacing;
    unsigned int casing;

};

struct text
{

    unsigned int node;
    struct metrics metrics;
    char *data;
    unsigned int open;
    unsigned int wrap;
    unsigned int breaking;
    unsigned int collapse;

};

struct columns
{

    int widths[COLUMNS];
    unsigned int count;
    int spacing;

};

struct cell
{

    unsigned int node;
    unsigned int fragmentstart;
    unsigned int fragmentend;
    int height;
    int natural;
    int margins;

};

struct band
{

    int left;
    int right;
    int below;

};

struct label
{

    char text[LABELSIZE];
    unsigned int length;

};

static int layoutblock(struct layout *layout, unsigned int node, int x, int top, int availw);
static void layoutnode(struct layout *layout, struct flow *flow, unsigned int node);
static unsigned int isroot(struct layout *layout, unsigned int node);
static int leadingmargin(struct layout *layout, unsigned int node, int availw);

static struct css_style *getstyle(struct layout *layout, unsigned int node)
{

    return &layout->styles[node];

}

static struct html_node *getnode(struct layout *layout, unsigned int node)
{

    return &layout->document->nodes[node];

}

static int max(int a, int b)
{

    return (a > b) ? a : b;

}

static int min(int a, int b)
{

    return (a < b) ? a : b;

}

static int percentof(int base, int value)
{

    int magnitude = (value < 0) ? -value : value;
    int high = base * (magnitude / 100);
    int low = base * (magnitude % 100);
    int result;

    /* the value is a thousandth of a percent, split so tall pages do not overflow, and rounded */
    if (base < 0)
        return -percentof(-base, value);

    result = (high + (low + 50) / 100 + 500) / 1000;

    return (value < 0) ? -result : result;

}

static int resolve(struct css_length *length, int base)
{

    switch (length->unit)
    {

    case CSS_UNIT_PX:
        return length->value;

    case CSS_UNIT_PERCENT:
        return percentof(base, length->value) + length->offset;

    }

    return 0;

}

static unsigned int isspace(char c)
{

    return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f';

}

static struct metrics getmetrics(struct layout *layout, struct css_style *style)
{

    struct metrics metrics;
    int lineheight;
    int halfleading;

    metrics.choice = font_choose(style->fontsize, style->bold, style->monospace);
    metrics.glyphascent = font_getascent(layout->fonts, metrics.choice);
    metrics.glyphheight = metrics.glyphascent + font_getdescent(layout->fonts, metrics.choice);
    metrics.spacing = style->letterspacing;
    metrics.casing = (style->transform == CSS_TRANSFORM_UPPERCASE) ? FONT_UPPERCASE : (style->transform == CSS_TRANSFORM_LOWERCASE) ? FONT_LOWERCASE : 0;
    metrics.space = (font_getglyph(layout->fonts, metrics.choice, ' ')->advance + 32) / 64 + metrics.spacing;
    lineheight = (style->lineheight) ? style->lineheight : font_getlineheight(layout->fonts, metrics.choice);
    halfleading = (lineheight - metrics.glyphheight) / 2;
    metrics.ascent = halfleading + metrics.glyphascent;
    metrics.descent = lineheight - metrics.ascent;

    return metrics;

}

struct pen
{

    int x;
    unsigned int previous;

};

static void advance(struct layout *layout, struct metrics *metrics, struct pen *pen, unsigned int codepoint)
{

    struct font_glyph *glyph = font_getglyph(layout->fonts, metrics->choice, font_setcase(codepoint, metrics->casing));

    /* pairs are kerned within a word only, spaces are measured apart from the words around them */
    pen->x += glyph->advance + metrics->spacing * 64 + font_getkerning(layout->fonts, metrics->choice, pen->previous, glyph->id);
    pen->previous = (codepoint == ' ') ? 0 : glyph->id;

}

static int measuretext(struct layout *layout, struct metrics *metrics, char *text, unsigned int length, unsigned int collapse)
{

    struct pen pen;
    unsigned int space = 0;
    unsigned int i = 0;

    pen.x = 0;
    pen.previous = 0;

    /* the pieces are added in 1/64 pixels and rounded once, as painting does */
    while (i < length)
    {

        unsigned int codepoint;

        if (collapse && isspace(text[i]))
        {

            if (!space)
                advance(layout, metrics, &pen, ' ');

            space = 1;
            i++;

            continue;

        }

        space = 0;
        i += utf8_read(text + i, length - i, &codepoint);
        advance(layout, metrics, &pen, codepoint);

    }

    return (pen.x + 32) / 64;

}

static unsigned int fitbytes(struct layout *layout, struct metrics *metrics, char *text, unsigned int length, int width)
{

    struct pen pen;
    unsigned int i = 0;

    pen.x = 0;
    pen.previous = 0;

    while (i < length)
    {

        unsigned int codepoint;
        unsigned int size = utf8_read(text + i, length - i, &codepoint);

        advance(layout, metrics, &pen, codepoint);

        if ((pen.x + 32) / 64 > width && i)
            break;

        i += size;

    }

    return (i) ? i : length;

}

static unsigned int addfragment(struct layout *layout, unsigned int node, unsigned int type, unsigned int text, unsigned int length, struct metrics *metrics, int x, int y, int width)
{

    struct layout_fragment *fragment;

    if (layout->fragmentcount >= LAYOUT_FRAGMENTS)
    {

        layout->full = 1;

        return NONE;

    }

    fragment = &layout->fragments[layout->fragmentcount];
    fragment->node = node;
    fragment->type = type;
    fragment->text = text;
    fragment->length = length;
    fragment->x = x;
    fragment->y = y;
    fragment->w = width;
    fragment->h = metrics->glyphheight;
    fragment->face = metrics->choice.face;
    fragment->size = metrics->choice.size;
    fragment->casing = metrics->casing;
    fragment->ascent = metrics->glyphascent;
    fragment->collapse = 0;
    fragment->depth = layout->atomicdepth;

    return layout->fragmentcount++;

}

static unsigned int addtextbytes(struct layout *layout, char *text, unsigned int length)
{

    unsigned int offset = layout->textcount;

    if (offset + length > LAYOUT_TEXTSIZE)
    {

        layout->full = 1;

        return NONE;

    }

    buffer_copy(layout->text + offset, text, length);

    layout->textcount += length;

    return offset;

}

static unsigned int addgenerated(struct layout *layout, unsigned int node, char *text, unsigned int length, struct metrics *metrics, int x, int y)
{

    unsigned int offset = addtextbytes(layout, text, length);

    if (offset == NONE)
        return NONE;

    return addfragment(layout, node, LAYOUT_FRAGMENT_GENERATED, offset, length, metrics, x, y, measuretext(layout, metrics, text, length, 0));

}

static unsigned int isempty(struct layout *layout, struct flow *flow)
{

    return layout->itemcount == flow->itemstart;

}

static struct band findband(struct layout *layout, struct flow *flow, int y)
{

    struct band band;
    unsigned int i;

    band.left = flow->boxleft;
    band.right = flow->boxright;
    band.below = y;

    for (i = layout->floatbase; i < layout->floatcount; i++)
    {

        struct layout_float *current = &layout->floats[i];

        if (y < current->top || y >= current->bottom || current->right <= flow->boxleft || current->left >= flow->boxright)
            continue;

        if (current->side == CSS_FLOAT_LEFT)
            band.left = max(band.left, current->right);
        else
            band.right = min(band.right, current->left);

        if (band.below == y || current->bottom < band.below)
            band.below = current->bottom;

    }

    band.right = max(band.right, band.left);

    return band;

}

static void fitline(struct layout *layout, struct flow *flow)
{

    struct band band;

    if (flow->type != FLOW_NORMAL)
        return;

    band = findband(layout, flow, flow->y);

    if (isempty(layout, flow))
        flow->x += band.left - flow->left;

    flow->left = band.left;
    flow->right = band.right;

}

static unsigned int dropline(struct layout *layout, struct flow *flow)
{

    struct band band;

    if (flow->type != FLOW_NORMAL || !isempty(layout, flow))
        return 0;

    band = findband(layout, flow, flow->y);

    if (band.below == flow->y)
        return 0;

    flow->y = band.below;

    fitline(layout, flow);

    return 1;

}

static int clearance(struct layout *layout, unsigned int clear, int y)
{

    unsigned int i;

    for (i = layout->floatbase; i < layout->floatcount; i++)
    {

        struct layout_float *current = &layout->floats[i];

        if (((current->side == CSS_FLOAT_LEFT && (clear & CSS_CLEAR_LEFT)) || (current->side == CSS_FLOAT_RIGHT && (clear & CSS_CLEAR_RIGHT))) && current->bottom > y)
            y = current->bottom;

    }

    return y;

}

static void startline(struct layout *layout, struct flow *flow)
{

    flow->collapsed = 0;

    if (isempty(layout, flow) && flow->pending)
    {

        flow->y += flow->pending;
        flow->pending = 0;

        fitline(layout, flow);

    }

}

static void additem(struct layout *layout, unsigned int node, unsigned int atomic, unsigned int fragmentstart, unsigned int fragmentend, int ascent, int descent, int shift)
{

    struct layout_item *item;

    if (layout->itemcount >= LAYOUT_ITEMS)
    {

        layout->full = 1;

        return;

    }

    item = &layout->items[layout->itemcount++];
    item->node = node;
    item->atomic = atomic;
    item->fragmentstart = fragmentstart;
    item->fragmentend = fragmentend;
    item->ascent = ascent;
    item->descent = descent;
    item->shift = shift;

}

static void translate(struct layout *layout, unsigned int node, unsigned int fragmentstart, unsigned int fragmentend, int dx, int dy, unsigned int self)
{

    unsigned int current = (self) ? node : getnode(layout, node)->first;
    unsigned int i;

    while (current)
    {

        struct layout_box *box = &layout->boxes[current];

        if (box->placed)
        {

            box->x += dx;
            box->y += dy;

        }

        current = html_next(layout->document, current, node);

        if (current == node)
            break;

    }

    for (i = fragmentstart; i < fragmentend; i++)
    {

        layout->fragments[i].x += dx;
        layout->fragments[i].y += dy;

    }

}

static void placemarker(struct layout *layout, int baseline)
{

    struct layout_fragment *fragment = &layout->fragments[layout->marker - 1];

    fragment->y = baseline - fragment->ascent;
    layout->marker = 0;

}

static struct css_style *getitemstyle(struct layout *layout, struct flow *flow, struct layout_item *item)
{

    unsigned int parent = getnode(layout, item->node)->parent;

    if (item->atomic)
        return getstyle(layout, item->node);

    /* text takes the alignment of the inline element around it, not of the block it flows in */
    return (parent == flow->node) ? 0 : getstyle(layout, parent);

}

static unsigned int linealign(struct layout *layout, struct flow *flow, struct layout_item *item)
{

    struct css_style *style = getitemstyle(layout, flow, item);
    unsigned int align;

    if (flow->type != FLOW_FLEXROW)
        return (style) ? style->valign : CSS_VALIGN_BASELINE;

    /* in a flex line the items and the text between them line up as flex items do */
    align = getstyle(layout, flow->node)->alignitems;

    if (item->atomic && style->alignself != CSS_FLEX_AUTO)
        align = style->alignself;

    return (align == CSS_FLEX_CENTER) ? LINE_CENTER : (align == CSS_FLEX_END) ? CSS_VALIGN_BOTTOM : CSS_VALIGN_TOP;

}

static int alignitem(struct layout *layout, struct flow *flow, struct layout_item *item, int top, int baseline, int height, int middle)
{

    struct css_style *style = getitemstyle(layout, flow, item);

    switch (linealign(layout, flow, item))
    {

    case CSS_VALIGN_TOP:
        return top + item->ascent;

    case CSS_VALIGN_BOTTOM:
        return top + height - item->descent;

    case CSS_VALIGN_MIDDLE:
        return baseline - middle - (item->ascent + item->descent) / 2 + item->ascent;

    case LINE_CENTER:
        return top + (height - item->ascent - item->descent) / 2 + item->ascent;

    }

    return baseline - ((style) ? style->valignoffset : 0);

}

static unsigned int countlinemargins(struct layout *layout, struct flow *flow)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = flow->itemstart; i < layout->itemcount; i++)
    {

        struct layout_item *item = &layout->items[i];

        if (item->atomic)
            count += (getstyle(layout, item->node)->margin[CSS_LEFT].unit == CSS_UNIT_AUTO) + (getstyle(layout, item->node)->margin[CSS_RIGHT].unit == CSS_UNIT_AUTO);

    }

    return count;

}

static void flushline(struct layout *layout, struct flow *flow, unsigned int forced)
{

    struct metrics strut = getmetrics(layout, getstyle(layout, flow->node));
    struct css_style *style = getstyle(layout, flow->node);
    int ascent = (flow->type == FLOW_FLEXROW) ? 0 : strut.ascent;
    int descent = (flow->type == FLOW_FLEXROW) ? 0 : strut.descent;
    int used = flow->x - flow->left;
    int middle = style->fontsize / 4;
    int tallest = 0;
    int dx = 0;
    int free = 0;
    unsigned int automargins = 0;
    int baseline;
    unsigned int i;

    if (isempty(layout, flow))
    {

        if (forced)
        {

            startline(layout, flow);

            flow->y += strut.ascent + strut.descent;

        }

        flow->x = flow->left;
        flow->space = 0;

        fitline(layout, flow);

        return;

    }

    for (i = flow->itemstart; i < layout->itemcount; i++)
    {

        struct layout_item *item = &layout->items[i];
        struct css_style *itemstyle = getitemstyle(layout, flow, item);
        int height = item->ascent + item->descent;
        int offset = (itemstyle) ? itemstyle->valignoffset : 0;

        switch (linealign(layout, flow, item))
        {

        case CSS_VALIGN_TOP:
        case CSS_VALIGN_BOTTOM:
        case LINE_CENTER:
            tallest = max(tallest, height);

            break;

        case CSS_VALIGN_MIDDLE:
            ascent = max(ascent, height / 2 + middle);
            descent = max(descent, height - height / 2 - middle);

            break;

        default:
            ascent = max(ascent, item->ascent + offset);
            descent = max(descent, item->descent - offset);

            break;

        }

    }

    /* boxes hung from the top or bottom only make the line taller once the rest has set it */
    descent = max(descent, tallest - ascent);

    if (flow->type == FLOW_FLEXROW)
    {

        /* a flex line gives its free space to auto margins first, then places items as justify-content says */
        automargins = countlinemargins(layout, flow);
        free = max(flow->right - flow->left - used, 0);

        if (!automargins)
            dx = (style->justify == CSS_FLEX_CENTER) ? free / 2 : (style->justify == CSS_FLEX_END) ? free : 0;

    }

    else if (style->align == CSS_ALIGN_CENTER)
    {

        dx = max((flow->right - flow->left - used) / 2, 0);

    }

    else if (style->align == CSS_ALIGN_RIGHT)
    {

        dx = max(flow->right - flow->left - used, 0);

    }

    baseline = flow->y + ascent;

    for (i = flow->itemstart; i < layout->itemcount; i++)
    {

        struct layout_item *item = &layout->items[i];
        int itembaseline = alignitem(layout, flow, item, flow->y, baseline, ascent + descent, middle);

        if (automargins && item->atomic && getstyle(layout, item->node)->margin[CSS_LEFT].unit == CSS_UNIT_AUTO)
            dx += free / (int)automargins;

        if (item->atomic)
        {

            translate(layout, item->node, item->fragmentstart, item->fragmentend, dx, itembaseline - item->ascent - flow->y, 1);

        }

        else if (item->fragmentstart != NONE)
        {

            layout->fragments[item->fragmentstart].x += dx;
            layout->fragments[item->fragmentstart].y = itembaseline - item->shift;

        }

        if (automargins && item->atomic && getstyle(layout, item->node)->margin[CSS_RIGHT].unit == CSS_UNIT_AUTO)
            dx += free / (int)automargins;

    }

    if (layout->marker && layout->markerdepth == layout->atomicdepth && !layout->measuring)
        placemarker(layout, baseline);

    layout->baseline = baseline;
    layout->baselinedepth = layout->atomicdepth;

    layout->extent = max(layout->extent, flow->left + used);
    layout->itemcount = flow->itemstart;
    flow->y = baseline + descent;
    flow->x = flow->left;
    flow->space = 0;

    fitline(layout, flow);

}

static void closefragment(struct text *text)
{

    text->open = NONE;

}

static void placepiece(struct layout *layout, struct flow *flow, struct text *text, unsigned int start, unsigned int end, int width, int spacewidth)
{

    struct layout_fragment *fragment = (text->open != NONE) ? &layout->fragments[text->open] : 0;

    startline(layout, flow);

    if (fragment)
    {

        fragment->length = end - fragment->text;
        fragment->w += spacewidth + width;

    }

    else
    {

        unsigned int leading = spacewidth && flow->space == text->node && start > getnode(layout, text->node)->text;
        struct layout_item *item = (!isempty(layout, flow)) ? &layout->items[layout->itemcount - 1] : 0;

        /* a collapsed space is drawn with the text it came from, so backgrounds have no gaps */
        if (spacewidth && !leading && item && !item->atomic && item->fragmentstart == layout->fragmentcount - 1 && layout->fragments[item->fragmentstart].node == flow->space)
            layout->fragments[item->fragmentstart].w += spacewidth;

        if (leading)
        {

            start--;
            width += spacewidth;

        }

        else
        {

            flow->x += spacewidth;

        }

        spacewidth = 0;
        text->open = addfragment(layout, text->node, LAYOUT_FRAGMENT_TEXT, start, end - start, &text->metrics, flow->x, flow->y, width);

        if (text->open != NONE)
        {

            layout->fragments[text->open].collapse = text->collapse;

            additem(layout, text->node, 0, text->open, text->open + 1, text->metrics.ascent, text->metrics.descent, text->metrics.glyphascent);

        }

    }

    flow->x += spacewidth + width;
    flow->space = 0;

}

static void countwrap(struct layout *layout, struct flow *flow)
{

    layout->wrapped++;

    /* a line broken against the edge of the box being measured means its content wants more room than it has */
    if (flow->right >= layout->wrapedge)
        layout->edgewraps++;

}

static void placeword(struct layout *layout, struct flow *flow, struct text *text, unsigned int start, unsigned int end)
{

    char *data = text->data;
    int width = measuretext(layout, &text->metrics, data + start, end - start, 0);
    int spacewidth = (flow->space && !isempty(layout, flow)) ? text->metrics.space : 0;

    if (text->wrap && !isempty(layout, flow) && flow->x + spacewidth + width > flow->right)
    {

        countwrap(layout, flow);

        closefragment(text);
        flushline(layout, flow, 0);

        spacewidth = 0;

    }

    while (text->wrap && width > flow->right - flow->left && dropline(layout, flow));

    /* long words break to fit anyway, but only text allowed to break anywhere makes the narrowest box narrower */
    while (text->wrap && (!layout->minimum || text->breaking) && isempty(layout, flow) && width > flow->right - flow->left && end - start > 1)
    {

        unsigned int split = fitbytes(layout, &text->metrics, data + start, end - start, flow->right - flow->left);

        if (split >= end - start)
            break;

        countwrap(layout, flow);

        placepiece(layout, flow, text, start, start + split, measuretext(layout, &text->metrics, data + start, split, 0), 0);
        closefragment(text);
        flushline(layout, flow, 0);

        start += split;
        width = measuretext(layout, &text->metrics, data + start, end - start, 0);

    }

    placepiece(layout, flow, text, start, end, width, spacewidth);

}

static void addtext(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct html_node *textnode = getnode(layout, node);
    struct css_style *style = getstyle(layout, textnode->parent);
    unsigned int whitespace = style->whitespace;
    unsigned int breaks = whitespace == CSS_WHITESPACE_PRE || whitespace == CSS_WHITESPACE_PREWRAP || whitespace == CSS_WHITESPACE_PRELINE;
    unsigned int end = textnode->text + textnode->length;
    struct text text;
    unsigned int i = textnode->text;

    text.node = node;
    text.metrics = getmetrics(layout, style);
    text.data = layout->document->text;
    text.open = NONE;
    text.collapse = whitespace != CSS_WHITESPACE_PRE && whitespace != CSS_WHITESPACE_PREWRAP;
    text.wrap = whitespace == CSS_WHITESPACE_NORMAL || whitespace == CSS_WHITESPACE_PRELINE;
    text.breaking = style->breaking;

    while (i < end)
    {

        char c = text.data[i];
        unsigned int stop = i;

        if (breaks && c == '\n')
        {

            closefragment(&text);
            flushline(layout, flow, 1);

            i++;

            continue;

        }

        if (!text.collapse && c == '\t')
        {

            int tab = text.metrics.space * 8;

            closefragment(&text);
            startline(layout, flow);

            if (tab)
                flow->x = flow->left + ((flow->x - flow->left) / tab + 1) * tab;

            i++;

            continue;

        }

        if (text.collapse && isspace(c))
        {

            flow->space = node;
            i++;

            continue;

        }

        while (stop < end && !((text.collapse && isspace(text.data[stop])) || (!text.collapse && (text.data[stop] == '\n' || text.data[stop] == '\t'))))
            stop++;

        if (text.collapse && text.open != NONE && flow->space)
        {

            int width = measuretext(layout, &text.metrics, text.data + i, stop - i, 0);

            if (!text.wrap || flow->x + text.metrics.space + width <= flow->right)
            {

                placepiece(layout, flow, &text, i, stop, width, text.metrics.space);

                i = stop;

                continue;

            }

        }

        if (text.open != NONE && flow->space)
            closefragment(&text);

        placeword(layout, flow, &text, i, stop);

        i = stop;

    }

}

static int horizontalextra(struct css_style *style, int base, unsigned int side)
{

    return resolve(&style->margin[side], base) + style->borderwidth[side] + resolve(&style->padding[side], base);

}

static void layoutinline(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    int base = flow->boxright - flow->boxleft;
    int left = horizontalextra(style, base, CSS_LEFT);
    int right = horizontalextra(style, base, CSS_RIGHT);
    unsigned int child;

    if (left)
    {

        if (flow->space && !isempty(layout, flow))
            flow->x += getmetrics(layout, style).space;

        flow->space = 0;
        flow->x += left;

    }

    for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
        layoutnode(layout, flow, child);

    flow->x += right;

}

static void getlabel(struct layout *layout, unsigned int node, struct label *label)
{

    struct html_document *document = layout->document;
    struct html_node *element = getnode(layout, node);
    char *name = html_getstring(document, element->name);
    char *text = 0;

    label->length = 0;

    if (element->tag == HTML_TAG_IMG)
    {

        text = html_getattribute(document, node, "alt");

    }

    else if (element->tag == HTML_TAG_INPUT)
    {

        char *type = html_getattribute(document, node, "type");

        if (type && (cstring_match(type, "checkbox") || cstring_match(type, "radio")))
            text = (html_getattribute(document, node, "checked")) ? "x" : " ";
        else
            text = html_getattribute(document, node, "value");

        if (!text && type && cstring_match(type, "submit"))
            text = "Submit";

        if (!text && type && cstring_match(type, "reset"))
            text = "Reset";

        if (!text || !text[0])
            text = html_getattribute(document, node, "placeholder");

    }

    else if (element->tag == HTML_TAG_SELECT)
    {

        unsigned int current = html_next(document, node, node);
        unsigned int option = 0;

        while (current)
        {

            if (getnode(layout, current)->tag == HTML_TAG_OPTION && (!option || html_getattribute(document, current, "selected")))
                option = current;

            current = html_next(document, current, node);

        }

        if (option && getnode(layout, option)->first && getnode(layout, getnode(layout, option)->first)->type == HTML_NODE_TEXT)
        {

            struct html_node *textnode = getnode(layout, getnode(layout, option)->first);

            label->length = (textnode->length < LABELSIZE) ? textnode->length : LABELSIZE;

            buffer_copy(label->text, html_getstring(document, textnode->text), label->length);

        }

    }

    else if (!cstring_match(name, "textarea"))
    {

        text = name;

    }

    if (text)
    {

        unsigned int length = cstring_length(text);

        label->length = (length < LABELSIZE) ? length : LABELSIZE;

        buffer_copy(label->text, text, label->length);

    }

    while (label->length && isspace(label->text[label->length - 1]))
        label->length--;

}

static int replacedwidth(struct layout *layout, unsigned int node, struct metrics *metrics, int labelwidth)
{

    struct html_document *document = layout->document;
    struct html_node *element = getnode(layout, node);
    char *size = html_getattribute(document, node, "size");
    char *type = html_getattribute(document, node, "type");
    int charwidth = max(font_getaveragewidth(layout->fonts, metrics->choice), 1);
    int widest = font_getboxwidth(layout->fonts, metrics->choice);

    if (element->tag == HTML_TAG_INPUT && type && (cstring_match(type, "checkbox") || cstring_match(type, "radio")))
        return metrics->glyphheight;

    if (element->tag == HTML_TAG_INPUT && !(type && (cstring_match(type, "submit") || cstring_match(type, "button") || cstring_match(type, "reset") || cstring_match(type, "image"))))
        return charwidth * ((size) ? (int)cstring_read_value(size, cstring_length(size), 10) : 20) + max(widest - charwidth, 0);

    if (element->tag == HTML_TAG_SELECT)
        return labelwidth + charwidth * 2;

    if (cstring_match(html_getstring(document, element->name), "textarea"))
    {

        char *cols = html_getattribute(document, node, "cols");

        return charwidth * ((cols) ? (int)cstring_read_value(cols, cstring_length(cols), 10) : 20);

    }

    return max(labelwidth, 16);

}

static unsigned int haswidth(struct layout *layout, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);

    /* a percentage of a width still being measured is no width yet, only the box being measured has one */
    if (style->width.unit == CSS_UNIT_PERCENT)
        return !layout->measuring || node == layout->measured;

    return style->width.unit == CSS_UNIT_PX;

}

static int layoutreplaced(struct layout *layout, unsigned int node, int x, int top, int availw)
{

    struct css_style *style = getstyle(layout, node);
    struct layout_box *box = &layout->boxes[node];
    struct metrics metrics = getmetrics(layout, style);
    struct label label;
    int labelwidth;
    int bl = style->borderwidth[CSS_LEFT] + resolve(&style->padding[CSS_LEFT], availw);
    int br = style->borderwidth[CSS_RIGHT] + resolve(&style->padding[CSS_RIGHT], availw);
    int bt = style->borderwidth[CSS_TOP] + resolve(&style->padding[CSS_TOP], availw);
    int bb = style->borderwidth[CSS_BOTTOM] + resolve(&style->padding[CSS_BOTTOM], availw);
    struct layout_picture *picture = (layout->picture[node]) ? &layout->pictures[layout->picture[node] - 1] : 0;
    unsigned int sized = haswidth(layout, node);
    struct css_style *parent = getstyle(layout, getnode(layout, node)->parent);
    unsigned int haspercent = style->height.unit == CSS_UNIT_PERCENT && parent->height.unit == CSS_UNIT_PX;
    unsigned int hasheight = style->height.unit == CSS_UNIT_PX || haspercent;
    int width;
    int natural;
    int height = (haspercent) ? resolve(&style->height, parent->height.value) : (hasheight) ? style->height.value : metrics.ascent + metrics.descent;
    char *rows = html_getattribute(layout->document, node, "rows");

    getlabel(layout, node, &label);

    labelwidth = measuretext(layout, &metrics, label.text, label.length, 1);
    width = (sized) ? resolve(&style->width, availw) : replacedwidth(layout, node, &metrics, labelwidth);

    if (style->height.unit != CSS_UNIT_PX && rows && cstring_match(html_getstring(layout->document, getnode(layout, node)->name), "textarea"))
        height = (metrics.ascent + metrics.descent) * (int)cstring_read_value(rows, cstring_length(rows), 10);

    /* border-box only changes a size the page set, a natural size is always the content */
    if (style->borderbox && sized)
        width = max(width - bl - br, 0);

    if (style->borderbox && hasheight)
        height = max(height - bt - bb, 0);

    if (picture)
    {

        int ratiowidth = (int)picture->ratiowidth;
        int ratioheight = (int)picture->ratioheight;

        /* without a size of its own a picture with only a ratio fills the line, like an svg without width */
        if (!sized && !hasheight)
            width = (picture->width) ? (int)picture->width : (picture->height && ratioheight) ? (int)picture->height * ratiowidth / ratioheight : (ratioheight) ? max(availw - bl - br - resolve(&style->margin[CSS_LEFT], availw) - resolve(&style->margin[CSS_RIGHT], availw), 0) : 300;
        else if (!sized)
            width = (ratioheight) ? height * ratiowidth / ratioheight : (picture->width) ? (int)picture->width : 300;

        if (!hasheight)
            height = (!sized && picture->width && picture->height) ? (int)picture->height : (ratiowidth) ? width * ratioheight / ratiowidth : (picture->height) ? (int)picture->height : 150;

        label.length = 0;

    }

    natural = width;

    if (style->maxwidth.unit == CSS_UNIT_PX || style->maxwidth.unit == CSS_UNIT_PERCENT)
        width = min(width, resolve(&style->maxwidth, availw) - ((style->borderbox) ? bl + br : 0));

    width = min(width, max(availw - bl - br - resolve(&style->margin[CSS_LEFT], availw) - resolve(&style->margin[CSS_RIGHT], availw), 0));

    if (layout->forcenode == node)
    {

        width = max(layout->forcewidth - bl - br, 0);
        layout->forcenode = 0;

    }

    if (picture && !hasheight && width != natural && natural > 0)
        height = height * width / natural;

    box->placed = 1;
    box->x = x + resolve(&style->margin[CSS_LEFT], availw);
    box->y = top;
    box->w = bl + width + br;

    /* a block-level picture centers with auto margins like any block */
    if (style->display != CSS_DISPLAY_INLINE && style->display != CSS_DISPLAY_INLINEBLOCK && style->margin[CSS_LEFT].unit == CSS_UNIT_AUTO)
        box->x = x + max((style->margin[CSS_RIGHT].unit == CSS_UNIT_AUTO) ? (availw - box->w) / 2 : availw - box->w - resolve(&style->margin[CSS_RIGHT], availw), 0);

    box->h = bt + height + bb;

    if (label.length && width > 0)
    {

        unsigned int length = label.length;

        while (length > 1 && measuretext(layout, &metrics, label.text, length, 1) > width)
            length--;

        if (height >= metrics.glyphheight)
            addgenerated(layout, node, label.text, length, &metrics, box->x + bl, top + bt + (height - metrics.glyphheight) / 2);

    }

    layout->extent = max(layout->extent, box->x + box->w);

    return box->h;

}

static unsigned int isreplaced(struct layout *layout, unsigned int node)
{

    struct html_node *element = getnode(layout, node);

    return element->tag == HTML_TAG_IMG || element->tag == HTML_TAG_SVG || element->tag == HTML_TAG_INPUT || element->tag == HTML_TAG_SELECT || element->tag == HTML_TAG_TEXTAREA;

}

static int placesized(struct layout *layout, unsigned int node, int x, int top, int ml, int mr, int width)
{

    int height;

    /* the margins were resolved against the containing block, the outer width would give percentages the wrong base */
    layout->forcenode = node;
    layout->forcewidth = width;
    layout->marginnode = node;
    layout->forceleft = ml;
    layout->forceright = mr;
    height = layoutblock(layout, node, x, top, ml + width + mr);
    layout->marginnode = 0;

    return height;

}

static int measure(struct layout *layout, unsigned int node, int availw)
{

    struct css_style *style = getstyle(layout, node);
    struct layout_measure *cached = &layout->measures[node];
    unsigned int fragmentcount = layout->fragmentcount;
    unsigned int floatcount = layout->floatcount;
    unsigned int textcount = layout->textcount;
    unsigned int marker = layout->marker;
    unsigned int truncated = layout->truncated;
    int extent = layout->extent;
    int wrapedge = layout->wrapedge;
    unsigned int edgewraps = layout->edgewraps;
    int ml = resolve(&style->margin[CSS_LEFT], availw);
    int needed;
    int extra;

    if (cached->availw == availw && !layout->minimum)
        return cached->width;

    if (layout->measuring >= MEASUREDEPTH)
    {

        layout->truncated = 1;

        return max(availw - ml - resolve(&style->margin[CSS_RIGHT], availw), 0);

    }

    if (!layout->measuring)
        layout->measured = node;

    layout->wrapedge = availw - resolve(&style->margin[CSS_RIGHT], availw) - style->borderwidth[CSS_RIGHT] - resolve(&style->padding[CSS_RIGHT], availw);
    layout->edgewraps = 0;
    layout->measuring++;
    layout->truncated = 0;
    layout->extent = 0;

    layoutblock(layout, node, 0, 0, availw);

    needed = layout->extent - ml - resolve(&style->margin[CSS_RIGHT], availw);

    /* a percentage of a width still being found is no width, so the content decides the narrowest box */
    if (style->width.unit == CSS_UNIT_PX || (style->width.unit == CSS_UNIT_PERCENT && !layout->minimum) || isreplaced(layout, node))
        needed = layout->boxes[node].w;

    /* content that cannot wrap any narrower makes the box wider than the room it was given, as in css */
    needed = max(needed, 0);

    /* content that had to break against the box's own edge would rather have all the room there is */
    if (!layout->minimum && layout->edgewraps)
        needed = max(needed, availw - ml - resolve(&style->margin[CSS_RIGHT], availw));

    extra = style->borderwidth[CSS_LEFT] + style->borderwidth[CSS_RIGHT] + resolve(&style->padding[CSS_LEFT], availw) + resolve(&style->padding[CSS_RIGHT], availw);

    /* but never past a max-width or short of a min-width, whatever the content wants */
    if (style->maxwidth.unit == CSS_UNIT_PX)
        needed = min(needed, style->maxwidth.value + ((style->borderbox) ? 0 : extra));

    if (style->minwidth.unit == CSS_UNIT_PX)
        needed = max(needed, style->minwidth.value + ((style->borderbox) ? 0 : extra));

    /* a width guessed at the depth limit is not kept, so it never stands in for a real one */
    if (!layout->truncated && !layout->minimum && layout->measured == node)
    {

        cached->availw = availw;
        cached->width = needed;

    }

    layout->measuring--;
    layout->truncated = layout->truncated || truncated;
    layout->wrapedge = wrapedge;
    layout->edgewraps = edgewraps;
    layout->fragmentcount = fragmentcount;
    layout->floatcount = floatcount;
    layout->textcount = textcount;
    layout->marker = marker;
    layout->extent = extent;

    return needed;

}

static void placeatomic(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    struct css_style *container = getstyle(layout, flow->node);
    int availw = flow->boxright - flow->boxleft;
    int ml = resolve(&style->margin[CSS_LEFT], availw);
    int mr = resolve(&style->margin[CSS_RIGHT], availw);
    int mt = resolve(&style->margin[CSS_TOP], availw);
    int mb = resolve(&style->margin[CSS_BOTTOM], availw);
    int width = measure(layout, node, availw);
    int spacewidth = (flow->space && !isempty(layout, flow) && flow->type == FLOW_NORMAL) ? getmetrics(layout, container).space : 0;
    int gap = (flow->type == FLOW_FLEXROW && !isempty(layout, flow)) ? container->columngap : 0;
    unsigned int wrap = container->whitespace == CSS_WHITESPACE_NORMAL || container->whitespace == CSS_WHITESPACE_PRELINE;
    unsigned int fragmentstart;
    int height;
    int ascent;

    if (wrap && !isempty(layout, flow) && flow->x + spacewidth + gap + ml + width + mr > flow->right)
    {

        countwrap(layout, flow);

        flushline(layout, flow, 0);

        spacewidth = 0;
        gap = 0;

    }

    while (ml + width + mr > flow->right - flow->left && dropline(layout, flow));

    startline(layout, flow);

    flow->x += spacewidth + gap;
    fragmentstart = layout->fragmentcount;
    layout->atomicdepth++;
    layout->baselinedepth = NONE;

    height = placesized(layout, node, flow->x, flow->y + mt, ml, mr, width);

    layout->atomicdepth--;

    ascent = mt + height + mb;

    /* an inline-block sits on the baseline of its last line box, or on its bottom edge without one */
    if (flow->type == FLOW_NORMAL && !isreplaced(layout, node) && layout->baselinedepth == layout->atomicdepth + 1)
        ascent = min(max(layout->baseline - flow->y, 0), mt + height + mb);

    additem(layout, node, 1, fragmentstart, layout->fragmentcount, ascent, mt + height + mb - ascent, 0);

    flow->x += ml + layout->boxes[node].w + mr;
    flow->space = 0;

}

static int collapse(int a, int b)
{

    if (a >= 0 && b >= 0)
        return max(a, b);

    if (a < 0 && b < 0)
        return min(a, b);

    return a + b;

}

static void layoutblockchild(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    int availw = flow->boxright - flow->boxleft;
    int top;

    flushline(layout, flow, 0);

    /* the first block's margin has already been joined with the parent's, which moved the parent instead */
    top = (flow->collapsed) ? flow->y : flow->y + collapse(flow->pending, collapse(resolve(&style->margin[CSS_TOP], availw), leadingmargin(layout, node, availw)));
    flow->collapsed = 0;
    layout->trailing = 0;

    if (style->clear)
        top = max(top, clearance(layout, style->clear, flow->y));

    /* a box sized to its content is measured first and then placed at that width */
    if (style->width.unit == CSS_UNIT_FIT && !isreplaced(layout, node))
    {

        int width = measure(layout, node, availw);

        layout->forcenode = node;
        layout->forcewidth = width;

    }

    /* a box with its own formatting context sits beside floats instead of under them */
    if (style->overflow || style->display == CSS_DISPLAY_TABLE)
    {

        struct band band = findband(layout, flow, top);

        flow->y = top + layoutblock(layout, node, band.left, top, band.right - band.left);

    }

    else
    {

        flow->y = top + layoutblock(layout, node, flow->boxleft, top, availw);

    }

    flow->pending = collapse(layout->trailing, resolve(&style->margin[CSS_BOTTOM], availw));

    fitline(layout, flow);

}

static void placefloat(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    int availw = flow->boxright - flow->boxleft;
    int ml = resolve(&style->margin[CSS_LEFT], availw);
    int mr = resolve(&style->margin[CSS_RIGHT], availw);
    int mt = resolve(&style->margin[CSS_TOP], availw);
    int mb = resolve(&style->margin[CSS_BOTTOM], availw);
    int width;
    int outer;
    int height;
    int x;
    int y;
    struct band band;
    unsigned int tries;

    if (!isempty(layout, flow))
        flushline(layout, flow, 0);

    width = measure(layout, node, availw);
    outer = ml + width + mr;
    y = flow->y + max(flow->pending, 0);

    if (layout->floatcount > layout->floatbase)
        y = max(y, layout->floats[layout->floatcount - 1].top);

    if (style->clear)
        y = clearance(layout, style->clear, y);

    band = findband(layout, flow, y);

    for (tries = 0; tries < LAYOUT_FLOATS && band.right - band.left < outer && band.below != y; tries++)
    {

        y = band.below;
        band = findband(layout, flow, y);

    }

    x = (style->floating == CSS_FLOAT_LEFT) ? band.left : band.right - outer;

    height = placesized(layout, node, x, y + mt, ml, mr, width);

    if (layout->floatcount < LAYOUT_FLOATS)
    {

        struct layout_float *current = &layout->floats[layout->floatcount++];

        current->left = x;
        current->right = x + outer;
        current->top = y;
        current->bottom = y + mt + height + mb;
        current->side = style->floating;

    }

    /* a right float pushed past the left edge still needs its whole width when shrinking to fit */
    layout->extent = max(layout->extent, max(x, band.left) + outer);

    fitline(layout, flow);

}

static unsigned int isvisible(struct css_style *style)
{

    return style->display != CSS_DISPLAY_NONE && style->position != CSS_POSITION_ABSOLUTE && style->position != CSS_POSITION_FIXED;

}

static unsigned int isabsolute(struct css_style *style)
{

    return style->display != CSS_DISPLAY_NONE && (style->position == CSS_POSITION_ABSOLUTE || style->position == CSS_POSITION_FIXED);

}

static void queueabsolute(struct layout *layout, unsigned int node, int x, int y)
{

    struct layout_absolute *absolute;

    if (layout->measuring)
        return;

    if (layout->absolutecount >= LAYOUT_ABSOLUTES)
    {

        layout->full = 1;

        return;

    }

    absolute = &layout->absolutes[layout->absolutecount++];
    absolute->node = node;
    absolute->x = x;
    absolute->y = y;

}

static unsigned int isblank(struct layout *layout, unsigned int node)
{

    struct html_node *textnode = getnode(layout, node);
    unsigned int i;

    for (i = 0; i < textnode->length; i++)
    {

        if (!isspace(layout->document->text[textnode->text + i]))
            return 0;

    }

    return 1;

}

static unsigned int isopen(struct layout *layout, unsigned int node, unsigned int side)
{

    struct css_style *style = getstyle(layout, node);

    return !style->borderwidth[side] && !style->padding[side].value && !isroot(layout, node);

}

static int leadingmargin(struct layout *layout, unsigned int node, int availw)
{

    unsigned int child;

    if (!isopen(layout, node, CSS_TOP))
        return 0;

    for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
    {

        struct html_node *element = getnode(layout, child);
        struct css_style *style;

        if (element->type == HTML_NODE_TEXT && !isblank(layout, child))
            return 0;

        if (element->type != HTML_NODE_ELEMENT)
            continue;

        style = getstyle(layout, child);

        if (!isvisible(style) || style->floating)
            continue;

        /* an inline that starts with a block lets that block's margin through, as if it were not there */
        if (style->display == CSS_DISPLAY_INLINE)
            return leadingmargin(layout, child, availw);

        if (style->display == CSS_DISPLAY_INLINEBLOCK || style->clear)
            return 0;

        return collapse(resolve(&style->margin[CSS_TOP], availw), leadingmargin(layout, child, availw));

    }

    return 0;

}

static void layoutnode(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct html_node *element = getnode(layout, node);
    struct css_style *style;

    if (element->type == HTML_NODE_TEXT)
    {

        if (flow->type == FLOW_NORMAL || !isblank(layout, node))
            addtext(layout, flow, node);

        return;

    }

    if (element->type != HTML_NODE_ELEMENT)
        return;

    style = getstyle(layout, node);

    if (isabsolute(style))
    {

        queueabsolute(layout, node, flow->x, flow->y);

        return;

    }

    if (!isvisible(style))
        return;

    if (element->tag == HTML_TAG_BR)
    {

        flushline(layout, flow, 1);

        if (style->clear)
        {

            flow->y = clearance(layout, style->clear, flow->y);

            fitline(layout, flow);

        }

        return;

    }

    if (flow->type == FLOW_FLEXROW)
    {

        placeatomic(layout, flow, node);

        return;

    }

    if (flow->type == FLOW_FLEXCOLUMN)
    {

        layoutblockchild(layout, flow, node);

        return;

    }

    if (style->floating)
    {

        placefloat(layout, flow, node);

        return;

    }

    switch (style->display)
    {

    case CSS_DISPLAY_INLINE:
        if (isreplaced(layout, node))
            placeatomic(layout, flow, node);
        else
            layoutinline(layout, flow, node);

        break;

    case CSS_DISPLAY_INLINEBLOCK:
        placeatomic(layout, flow, node);

        break;

    default:
        layoutblockchild(layout, flow, node);

        break;

    }

}

static unsigned int countprevious(struct layout *layout, unsigned int node)
{

    unsigned int count = 1;
    unsigned int prev;

    for (prev = getnode(layout, node)->prev; prev; prev = getnode(layout, prev)->prev)
    {

        if (getnode(layout, prev)->type == HTML_NODE_ELEMENT && getstyle(layout, prev)->display == CSS_DISPLAY_LISTITEM)
        {

            char *value = html_getattribute(layout->document, prev, "value");

            if (value)
                return count + (unsigned int)cstring_read_value(value, cstring_length(value), 10);

            count++;

        }

    }

    return count;

}

static void setmarker(struct layout *layout, unsigned int node, int x, int y)
{

    struct css_style *style = getstyle(layout, node);
    struct metrics metrics = getmetrics(layout, style);
    unsigned int index = NONE;

    if (style->liststyle == CSS_LIST_DECIMAL)
    {

        unsigned int parent = getnode(layout, node)->parent;
        char *start = (parent) ? html_getattribute(layout->document, parent, "start") : 0;
        char *value = html_getattribute(layout->document, node, "value");
        unsigned int number = (value) ? cstring_read_value(value, cstring_length(value), 10) : countprevious(layout, node) + ((start) ? cstring_read_value(start, cstring_length(start), 10) - 1 : 0);
        char text[16];
        unsigned int length = cstring_write_fmt(text, 16, 0, "%u.", &number);

        index = addgenerated(layout, node, text, length, &metrics, x, y);

    }

    else
    {

        index = addfragment(layout, node, LAYOUT_FRAGMENT_MARKER, style->liststyle, 0, &metrics, x, y, metrics.glyphheight / 3 + 2);

    }

    if (index != NONE)
    {

        struct layout_fragment *fragment = &layout->fragments[index];

        fragment->x = x - fragment->w - metrics.space;
        layout->marker = index + 1;
        layout->markerdepth = layout->atomicdepth;

    }

}

static void initflow(struct layout *layout, struct flow *flow, unsigned int node, int left, int right, int top)
{

    struct css_style *style = getstyle(layout, node);

    flow->boxleft = left;
    flow->boxright = right;
    flow->left = left;
    flow->right = right;
    flow->x = left;
    flow->y = top;
    flow->pending = 0;
    flow->itemstart = layout->itemcount;
    flow->space = 0;
    flow->collapsed = 0;
    flow->node = node;
    flow->type = FLOW_NORMAL;

    if (style->display == CSS_DISPLAY_FLEX)
        flow->type = (style->column) ? FLOW_FLEXCOLUMN : FLOW_FLEXROW;

    fitline(layout, flow);

}

static unsigned int iscell(struct layout *layout, unsigned int node)
{

    return getnode(layout, node)->type == HTML_NODE_ELEMENT && isvisible(getstyle(layout, node));

}

static unsigned int getspan(struct layout *layout, unsigned int node, unsigned int limit)
{

    char *colspan = html_getattribute(layout->document, node, "colspan");
    unsigned int span = (colspan) ? cstring_read_value(colspan, cstring_length(colspan), 10) : 1;

    return (span < 1) ? 1 : (span > limit) ? limit : span;

}

static int measuremin(struct layout *layout, unsigned int node)
{

    int width;

    layout->minimum++;
    width = measure(layout, node, 0);
    layout->minimum--;

    return width;

}

static unsigned int isrow(struct layout *layout, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    unsigned int cells = 0;
    unsigned int child;

    if (style->display == CSS_DISPLAY_ROW)
        return 1;

    if (style->display != CSS_DISPLAY_TABLE)
        return 0;

    /* a table of bare cells holds them in one anonymous row */
    for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
    {

        unsigned int display = (iscell(layout, child)) ? getstyle(layout, child)->display : CSS_DISPLAY_NONE;

        if (display == CSS_DISPLAY_ROW || display == CSS_DISPLAY_ROWGROUP)
            return 0;

        cells += display == CSS_DISPLAY_CELL;

    }

    return cells > 0;

}

static void measurerow(struct layout *layout, unsigned int row, struct columns *columns, int minwidths[COLUMNS], int maxwidths[COLUMNS], int fixed[COLUMNS], int availw)
{

    unsigned int column = 0;
    unsigned int cell;

    for (cell = getnode(layout, row)->first; cell && column < COLUMNS; cell = getnode(layout, cell)->next)
    {

        struct css_style *style;
        unsigned int span;

        if (!iscell(layout, cell))
            continue;

        style = getstyle(layout, cell);
        span = getspan(layout, cell, COLUMNS - column);

        if (span == 1)
        {

            maxwidths[column] = max(maxwidths[column], measure(layout, cell, MEASUREWIDTH));
            minwidths[column] = max(minwidths[column], measuremin(layout, cell));

            if (style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT)
                fixed[column] = max(fixed[column], resolve(&style->width, availw) + ((style->borderbox) ? 0 : style->borderwidth[CSS_LEFT] + style->borderwidth[CSS_RIGHT] + resolve(&style->padding[CSS_LEFT], availw) + resolve(&style->padding[CSS_RIGHT], availw)));

        }

        column += span;

    }

    columns->count = (column > columns->count) ? column : columns->count;

}

static int computecolumns(struct layout *layout, unsigned int node, struct columns *columns, int availw, unsigned int stretch)
{

    int minwidths[COLUMNS];
    int maxwidths[COLUMNS];
    int fixed[COLUMNS];
    int total = 0;
    int least = 0;
    int fixedtotal = 0;
    int space;
    unsigned int child;
    unsigned int i;

    buffer_clear(minwidths, sizeof (int) * COLUMNS);
    buffer_clear(maxwidths, sizeof (int) * COLUMNS);
    buffer_clear(fixed, sizeof (int) * COLUMNS);

    columns->count = 0;
    columns->spacing = (getstyle(layout, node)->collapse) ? 0 : getstyle(layout, node)->spacing;

    if (isrow(layout, node))
    {

        measurerow(layout, node, columns, minwidths, maxwidths, fixed, availw);

    }

    else
    {

        for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
        {

            unsigned int display;
            unsigned int row;

            if (!iscell(layout, child))
                continue;

            display = getstyle(layout, child)->display;

            if (display == CSS_DISPLAY_ROW)
                measurerow(layout, child, columns, minwidths, maxwidths, fixed, availw);

            if (display == CSS_DISPLAY_ROWGROUP)
            {

                for (row = getnode(layout, child)->first; row; row = getnode(layout, row)->next)
                {

                    if (iscell(layout, row) && getstyle(layout, row)->display == CSS_DISPLAY_ROW)
                        measurerow(layout, row, columns, minwidths, maxwidths, fixed, availw);

                }

            }

        }

    }

    if (!columns->count)
        return 0;

    space = max(availw - columns->spacing * ((int)columns->count + 1), 0);

    for (i = 0; i < columns->count; i++)
    {

        minwidths[i] = min(minwidths[i], maxwidths[i]);

        if (fixed[i])
        {

            fixed[i] = max(fixed[i], minwidths[i]);
            fixedtotal += fixed[i];

        }

        else
        {

            total += maxwidths[i];
            least += minwidths[i];

        }

    }

    /* every column gets its narrowest width and the rest is shared by how much more each wants */
    for (i = 0; i < columns->count; i++)
    {

        if (fixed[i])
            columns->widths[i] = (fixedtotal > space) ? max(fixed[i] * space / fixedtotal, minwidths[i]) : fixed[i];
        else if (total <= space - fixedtotal)
            columns->widths[i] = maxwidths[i];
        else if (least >= space - fixedtotal)
            columns->widths[i] = minwidths[i];
        else
            columns->widths[i] = minwidths[i] + (maxwidths[i] - minwidths[i]) * (space - fixedtotal - least) / (total - least);

    }

    total = 0;

    for (i = 0; i < columns->count; i++)
        total += columns->widths[i];

    if (stretch && total < space)
    {

        unsigned int autos = 0;
        int weight = 0;
        int extra = space - total;

        for (i = 0; i < columns->count; i++)
        {

            if (!fixed[i])
            {

                autos++;
                weight += maxwidths[i];

            }

        }

        for (i = 0; i < columns->count; i++)
        {

            if (!autos)
                columns->widths[i] += extra / (int)columns->count;
            else if (!fixed[i])
                columns->widths[i] += (weight) ? extra * maxwidths[i] / weight : extra / (int)autos;

        }

    }

    total = columns->spacing * ((int)columns->count + 1);

    for (i = 0; i < columns->count; i++)
        total += columns->widths[i];

    return total;

}

static void layoutrow(struct layout *layout, struct flow *flow, unsigned int row, struct columns *columns)
{

    struct layout_box *box = &layout->boxes[row];
    struct cell cells[COLUMNS];
    unsigned int count = 0;
    unsigned int column = 0;
    int x = flow->left + columns->spacing;
    int rowheight = 0;
    unsigned int cell;
    unsigned int i;

    flushline(layout, flow, 0);

    flow->y += max(flow->pending, 0);
    flow->pending = 0;
    box->placed = 1;
    box->x = flow->left;
    box->y = flow->y + columns->spacing;
    box->w = flow->right - flow->left;

    for (cell = getnode(layout, row)->first; cell && column < columns->count && count < COLUMNS; cell = getnode(layout, cell)->next)
    {

        unsigned int span;
        int width = 0;
        int top = 0;

        if (!iscell(layout, cell))
            continue;

        span = getspan(layout, cell, columns->count - column);

        for (i = column; i < column + span; i++)
            width += columns->widths[i];

        width += columns->spacing * ((int)span - 1);
        cells[count].margins = 0;

        /* a box that is not a cell sits in an anonymous cell, so its margins count */
        if (getstyle(layout, cell)->display != CSS_DISPLAY_CELL)
        {

            top = resolve(&getstyle(layout, cell)->margin[CSS_TOP], width);
            cells[count].margins = top + resolve(&getstyle(layout, cell)->margin[CSS_BOTTOM], width);

        }

        cells[count].node = cell;
        cells[count].fragmentstart = layout->fragmentcount;

        /* the columns already settled how wide the cell is, its own width took part in that */
        if (getstyle(layout, cell)->display == CSS_DISPLAY_CELL)
        {

            layout->forcenode = cell;
            layout->forcewidth = width;

        }

        cells[count].height = layoutblock(layout, cell, x, box->y + top, width) + cells[count].margins;
        cells[count].natural = layout->natural + cells[count].margins;
        cells[count].fragmentend = layout->fragmentcount;
        rowheight = max(rowheight, cells[count].height);
        x += width + columns->spacing;
        column += span;
        count++;

    }

    for (i = 0; i < count; i++)
    {

        struct layout_box *cellbox = &layout->boxes[cells[i].node];
        unsigned int valign = getstyle(layout, cells[i].node)->valign;
        int space = rowheight - cells[i].natural;

        /* baseline is taken as top, which it is for cells starting with a line of the same font */
        if (valign == CSS_VALIGN_MIDDLE || valign == CSS_VALIGN_BOTTOM)
            translate(layout, cells[i].node, cells[i].fragmentstart, cells[i].fragmentend, 0, (valign == CSS_VALIGN_MIDDLE) ? space / 2 : space, 0);

        cellbox->h = rowheight - cells[i].margins;

    }

    box->h = rowheight;
    flow->y = box->y + rowheight;

}

static unsigned int tablepass(struct layout *layout, unsigned int node)
{

    unsigned int tag = getnode(layout, node)->tag;

    return (tag == HTML_TAG_THEAD) ? 0 : (tag == HTML_TAG_TFOOT) ? 2 : 1;

}

static void layouttablepass(struct layout *layout, struct flow *flow, unsigned int table, struct columns *columns, unsigned int pass)
{

    unsigned int child;

    for (child = getnode(layout, table)->first; child; child = getnode(layout, child)->next)
    {

        unsigned int display;

        if (!iscell(layout, child) || tablepass(layout, child) != pass)
            continue;

        display = getstyle(layout, child)->display;

        if (display == CSS_DISPLAY_ROW)
        {

            layoutrow(layout, flow, child, columns);

        }

        else if (display == CSS_DISPLAY_ROWGROUP)
        {

            struct layout_box *box = &layout->boxes[child];
            unsigned int row;

            flushline(layout, flow, 0);

            box->placed = 1;
            box->x = flow->left;
            box->y = flow->y;
            box->w = flow->right - flow->left;

            for (row = getnode(layout, child)->first; row; row = getnode(layout, row)->next)
            {

                if (iscell(layout, row) && getstyle(layout, row)->display == CSS_DISPLAY_ROW)
                    layoutrow(layout, flow, row, columns);

            }

            box->h = flow->y - box->y;

        }

        else
        {

            layoutnode(layout, flow, child);

        }

    }

}

static void layouttable(struct layout *layout, struct flow *flow, unsigned int table, struct columns *columns)
{

    /* the header goes first and the footer last, wherever they are in the source */
    layouttablepass(layout, flow, table, columns, 0);
    layouttablepass(layout, flow, table, columns, 1);
    layouttablepass(layout, flow, table, columns, 2);
    flushline(layout, flow, 0);

    flow->y += columns->spacing;

}

static unsigned int hastext(struct layout *layout, unsigned int node)
{

    unsigned int child;

    for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
    {

        if (getnode(layout, child)->type == HTML_NODE_TEXT && !isblank(layout, child))
            return 1;

    }

    return 0;

}

static unsigned int getalign(struct css_style *container, struct css_style *style)
{

    return (style->alignself == CSS_FLEX_AUTO) ? container->alignitems : style->alignself;

}

static int padded(struct css_style *style, int availw)
{

    return style->borderwidth[CSS_LEFT] + style->borderwidth[CSS_RIGHT] + resolve(&style->padding[CSS_LEFT], availw) + resolve(&style->padding[CSS_RIGHT], availw);

}

static int basesize(struct layout *layout, unsigned int node, int availw, unsigned int usebasis)
{

    struct css_style *style = getstyle(layout, node);
    int extra = (style->borderbox) ? 0 : padded(style, availw);
    int size;

    if (usebasis && (style->basis.unit == CSS_UNIT_PX || style->basis.unit == CSS_UNIT_PERCENT))
        size = resolve(&style->basis, availw) + extra;
    else if (usebasis && style->basis.unit == CSS_UNIT_MINCONTENT)
        size = measuremin(layout, node);
    else if (haswidth(layout, node) && !isreplaced(layout, node))
        size = resolve(&style->width, availw) + extra;
    else
        size = measure(layout, node, availw);

    if (style->minwidth.unit == CSS_UNIT_PX || style->minwidth.unit == CSS_UNIT_PERCENT)
        size = max(size, resolve(&style->minwidth, availw) + extra);

    if (style->maxwidth.unit == CSS_UNIT_PX || style->maxwidth.unit == CSS_UNIT_PERCENT)
        size = min(size, resolve(&style->maxwidth, availw) + extra);

    return max(size, 0);

}

static unsigned int isflexitem(struct layout *layout, unsigned int node)
{

    struct html_node *element = getnode(layout, node);

    return element->type == HTML_NODE_ELEMENT && element->tag != HTML_TAG_BR && isvisible(getstyle(layout, node));

}

static unsigned int isordered(struct layout *layout, unsigned int node)
{

    unsigned int child;

    if (getstyle(layout, node)->reverse)
        return 1;

    for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
    {

        if (isflexitem(layout, child) && getstyle(layout, child)->order)
            return 1;

    }

    return 0;

}

static unsigned int follows(struct layout *layout, unsigned int a, unsigned int b, unsigned int reverse)
{

    int ordera = getstyle(layout, a)->order;
    int orderb = getstyle(layout, b)->order;

    if (ordera != orderb)
        return (reverse) ? ordera < orderb : ordera > orderb;

    return (reverse) ? a < b : a > b;

}

static unsigned int nextitem(struct layout *layout, unsigned int node, unsigned int prev, unsigned int ordered)
{

    unsigned int reverse = getstyle(layout, node)->reverse;
    unsigned int best = 0;
    unsigned int child;

    if (!ordered)
    {

        child = (prev) ? getnode(layout, prev)->next : getnode(layout, node)->first;

        while (child && !isflexitem(layout, child))
            child = getnode(layout, child)->next;

        return child;

    }

    /* items go by order and then by source, backwards for a reversed direction */
    for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
    {

        if (!isflexitem(layout, child) || (prev && !follows(layout, child, prev, reverse)))
            continue;

        if (!best || follows(layout, best, child, reverse))
            best = child;

    }

    return best;

}

static int placeflexitem(struct layout *layout, struct layout_flexitem *item, int x, int y)
{

    item->fragmentstart = layout->fragmentcount;
    item->height = placesized(layout, item->node, x, y + item->mt, item->ml, item->mr, item->size);
    item->fragmentend = layout->fragmentcount;

    return item->ml + layout->boxes[item->node].w + item->mr;

}

static void alignflexitem(struct layout *layout, struct layout_flexitem *item, unsigned int align, int space, unsigned int horizontal)
{

    struct css_style *style = getstyle(layout, item->node);
    int delta = (align == CSS_FLEX_CENTER) ? space / 2 : (align == CSS_FLEX_END) ? space : 0;

    /* an item taller than its line still centers on it or hangs from its end */
    if (space <= 0 && !delta)
        return;

    if (align == CSS_FLEX_STRETCH && !horizontal && style->height.unit != CSS_UNIT_PX && !isreplaced(layout, item->node))
        layout->boxes[item->node].h += space;
    else if (delta)
        translate(layout, item->node, item->fragmentstart, item->fragmentend, (horizontal) ? delta : 0, (horizontal) ? 0 : delta, 1);

}

static int minimumsize(struct layout *layout, unsigned int node, int availw)
{

    struct css_style *style = getstyle(layout, node);
    int size;

    /* an item with an automatic minimum does not shrink below its content, unless it clips */
    if (style->minwidth.unit != CSS_UNIT_AUTO)
        return resolve(&style->minwidth, availw) + ((style->borderbox) ? 0 : padded(style, availw));

    if (style->overflow)
        return 0;

    size = measuremin(layout, node);

    if (haswidth(layout, node) && !isreplaced(layout, node))
        size = min(size, resolve(&style->width, availw) + ((style->borderbox) ? 0 : padded(style, availw)));

    return size;

}

static void flexsizes(struct layout *layout, unsigned int start, unsigned int end, int free, int availw)
{

    unsigned int i;

    for (i = start; i < end; i++)
    {

        struct layout_flexitem *item = &layout->flexitems[i];
        struct css_style *style = getstyle(layout, item->node);

        /* growing only goes below the content from a basis smaller than it, like flex: 1 */
        item->frozen = 0;
        item->minimum = (free < 0 || style->basis.unit == CSS_UNIT_PX || style->basis.unit == CSS_UNIT_PERCENT) ? minimumsize(layout, item->node, availw) : 0;
        item->maximum = (style->maxwidth.unit == CSS_UNIT_PX || style->maxwidth.unit == CSS_UNIT_PERCENT) ? resolve(&style->maxwidth, availw) + ((style->borderbox) ? 0 : padded(style, availw)) : 0x7FFFFFFF;

    }

    /* items that would shrink below their minimum or grow past their maximum are held there and the rest share again */
    while (free)
    {

        unsigned int total = 0;
        unsigned int violated = 0;
        int remaining = free;

        for (i = start; i < end; i++)
        {

            struct layout_flexitem *item = &layout->flexitems[i];
            struct css_style *style = getstyle(layout, item->node);

            if (!item->frozen)
                total += (free > 0) ? style->grow : style->shrink * (unsigned int)item->size / 100;

        }

        if (!total)
            break;

        for (i = start; i < end; i++)
        {

            struct layout_flexitem *item = &layout->flexitems[i];
            struct css_style *style = getstyle(layout, item->node);
            unsigned int weight = (free > 0) ? style->grow : style->shrink * (unsigned int)item->size / 100;

            if (!item->frozen && item->size + free * (int)(weight * 1024 / total) / 1024 < item->minimum)
            {

                remaining -= item->minimum - item->size;
                item->size = item->minimum;
                item->frozen = 1;
                violated = 1;

            }

            else if (!item->frozen && free > 0 && item->size + free * (int)(weight * 1024 / total) / 1024 > item->maximum)
            {

                remaining -= max(item->maximum - item->size, 0);
                item->size = max(item->maximum, item->size);
                item->frozen = 1;
                violated = 1;

            }

        }

        if (violated)
        {

            free = remaining;

            continue;

        }

        for (i = start; i < end; i++)
        {

            struct layout_flexitem *item = &layout->flexitems[i];
            struct css_style *style = getstyle(layout, item->node);
            unsigned int weight = (free > 0) ? style->grow : style->shrink * (unsigned int)item->size / 100;

            if (!item->frozen)
                item->size = item->size + free * (int)(weight * 1024 / total) / 1024;

        }

        break;

    }

    for (i = start; i < end; i++)
        layout->flexitems[i].size = max(layout->flexitems[i].size, max(layout->flexitems[i].minimum, 0));

}

static int flexminimum(struct layout *layout, unsigned int node, int availw)
{

    struct css_style *style = getstyle(layout, node);
    int vertical = style->borderwidth[CSS_TOP] + style->borderwidth[CSS_BOTTOM] + resolve(&style->padding[CSS_TOP], availw) + resolve(&style->padding[CSS_BOTTOM], availw);

    if (style->minheight.unit != CSS_UNIT_PX)
        return 0;

    return max(style->minheight.value - ((style->borderbox) ? vertical : 0), 0);

}

static unsigned int isstretched(struct layout *layout, struct css_style *container, struct layout_flexitem *item)
{

    struct css_style *style = getstyle(layout, item->node);

    return getalign(container, style) == CSS_FLEX_STRETCH && style->height.unit == CSS_UNIT_AUTO && !isreplaced(layout, item->node);

}

static unsigned int haspercentchild(struct layout *layout, unsigned int node)
{

    unsigned int child;

    for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
    {

        if (getnode(layout, child)->type == HTML_NODE_ELEMENT && getstyle(layout, child)->height.unit == CSS_UNIT_PERCENT)
            return 1;

    }

    return 0;

}

static unsigned int needsrestretch(struct layout *layout, struct css_style *container, unsigned int start, unsigned int end, int cross)
{

    unsigned int i;

    for (i = start; i < end; i++)
    {

        struct layout_flexitem *item = &layout->flexitems[i];

        if (isstretched(layout, container, item) && item->mt + item->height + item->mb < cross && haspercentchild(layout, item->node))
            return 1;

    }

    return 0;

}

struct gridplace
{

    int from;
    unsigned int span;

};

struct gridtrack
{

    int base;
    int limit;
    int fraction;

};

static struct css_track autotrack(void)
{

    struct css_track track;

    track.minkind = CSS_TRACK_AUTO;
    track.maxkind = CSS_TRACK_AUTO;
    track.minvalue = 0;
    track.maxvalue = 0;

    return track;

}

static struct css_track columntrack(struct css_grid *grid, unsigned int index)
{

    if (grid && grid->repeat)
        return grid->columns[0];

    return (grid && index < grid->columncount) ? grid->columns[index] : autotrack();

}

static struct css_track rowtrack(struct css_grid *grid, unsigned int index)
{

    if (!grid)
        return autotrack();

    return (index < grid->rowcount) ? grid->rows[index] : grid->autorow;

}

static int fixedsize(unsigned int kind, int value, int base)
{

    return (kind == CSS_TRACK_PX) ? value : (kind == CSS_TRACK_PERCENT) ? percentof(base, value) : 0;

}

static unsigned int countcolumns(struct css_grid *grid, int width, int gap, unsigned int items)
{

    unsigned int count;

    if (!grid)
        return 1;

    /* repeated tracks fill the row with as many as fit, auto-fit drops the ones left empty */
    if (grid->repeat)
    {

        int size = max(fixedsize(grid->columns[0].minkind, grid->columns[0].minvalue, width), fixedsize(grid->columns[0].maxkind, grid->columns[0].maxvalue, width));

        count = (size + gap > 0) ? (unsigned int)max((width + gap) / (size + gap), 1) : 1;

        if (grid->repeat == CSS_REPEAT_FIT && items && items < count)
            count = items;

    }

    else
    {

        count = (grid->columncount > grid->areacolumns) ? grid->columncount : grid->areacolumns;

    }

    return (count < 1) ? 1 : (count > GRIDCOLUMNS) ? GRIDCOLUMNS : count;

}

static int lineindex(struct css_gridline *line, unsigned int count)
{

    if (line->span || !line->value)
        return -1;

    return (line->value > 0) ? line->value - 1 : (int)count + 1 + line->value;

}

static struct gridplace placeaxis(struct css_gridline *start, struct css_gridline *end, unsigned int count)
{

    struct gridplace place;
    int from = lineindex(start, count);
    int to = lineindex(end, count);

    place.from = -1;
    place.span = 1;

    if (from >= 0 && to >= 0)
    {

        place.from = min(from, to);
        place.span = (from == to) ? 1 : (unsigned int)((from > to) ? from - to : to - from);

    }

    else if (from >= 0)
    {

        place.from = from;
        place.span = (end->span) ? end->span : 1;

    }

    else if (to >= 0)
    {

        place.span = (start->span) ? start->span : 1;
        place.from = max(to - (int)place.span, 0);

    }

    else
    {

        place.span = max(max(start->span, end->span), 1);

    }

    return place;

}

static unsigned int findarea(struct css_grid *grid, struct layout_flexitem *item, unsigned int name)
{

    unsigned int top = GRIDROWS;
    unsigned int left = GRIDCOLUMNS;
    unsigned int bottom = 0;
    unsigned int right = 0;
    unsigned int r;
    unsigned int c;

    for (r = 0; grid && r < grid->arearows; r++)
    {

        for (c = 0; c < grid->areacolumns; c++)
        {

            if (grid->areas[r][c] != name)
                continue;

            top = min(top, r);
            left = min(left, c);
            bottom = max(bottom, r + 1);
            right = max(right, c + 1);

        }

    }

    if (bottom <= top)
        return 0;

    item->row = top;
    item->column = left;
    item->rows = bottom - top;
    item->columns = right - left;

    return 1;

}

static unsigned int isfree(unsigned int cells[GRIDROWS], unsigned int row, unsigned int column, unsigned int rows, unsigned int columns, unsigned int count)
{

    unsigned int mask = ((columns >= 32) ? 0xFFFFFFFF : ((1u << columns) - 1)) << column;
    unsigned int r;

    if (column + columns > count || row + rows > GRIDROWS)
        return 0;

    for (r = row; r < row + rows; r++)
    {

        if (cells[r] & mask)
            return 0;

    }

    return 1;

}

static void occupy(unsigned int cells[GRIDROWS], struct layout_flexitem *item)
{

    unsigned int mask = ((item->columns >= 32) ? 0xFFFFFFFF : ((1u << item->columns) - 1)) << item->column;
    unsigned int r;

    for (r = item->row; r < item->row + item->rows && r < GRIDROWS; r++)
        cells[r] |= mask;

}

static void placegriditems(struct layout *layout, struct css_grid *grid, unsigned int base, unsigned int count)
{

    unsigned int cells[GRIDROWS];
    unsigned int cursorrow = 0;
    unsigned int cursorcolumn = 0;
    unsigned int pass;
    unsigned int i;

    for (i = 0; i < GRIDROWS; i++)
        cells[i] = 0;

    /* items with a fixed place go first, then the rest fill the gaps row by row */
    for (pass = 0; pass < 2; pass++)
    {

        for (i = base; i < layout->flexcount; i++)
        {

            struct layout_flexitem *item = &layout->flexitems[i];
            struct css_style *style = getstyle(layout, item->node);
            struct gridplace rows = placeaxis(&style->gridlines[CSS_GRIDROWSTART], &style->gridlines[CSS_GRIDROWEND], GRIDROWS);
            struct gridplace columns = placeaxis(&style->gridlines[CSS_GRIDCOLUMNSTART], &style->gridlines[CSS_GRIDCOLUMNEND], count);
            unsigned int named = style->gridarea && findarea(grid, item, style->gridarea);

            if (!named)
            {

                item->rows = min(rows.span, GRIDROWS);
                item->columns = min(columns.span, count);
                item->row = (rows.from >= 0) ? (unsigned int)rows.from : 0;
                item->column = (columns.from >= 0) ? min((unsigned int)columns.from, count - item->columns) : 0;

            }

            if (pass == 0)
            {

                if (named || (rows.from >= 0 && columns.from >= 0))
                    occupy(cells, item);

                continue;

            }

            if (named || (rows.from >= 0 && columns.from >= 0))
                continue;

            if (columns.from >= 0)
            {

                while (item->row < GRIDROWS - 1 && !isfree(cells, item->row, item->column, item->rows, item->columns, count))
                    item->row++;

            }

            else if (rows.from >= 0)
            {

                while (item->column + item->columns < count && !isfree(cells, item->row, item->column, item->rows, item->columns, count))
                    item->column++;

            }

            else
            {

                while (cursorrow < GRIDROWS - 1 && !isfree(cells, cursorrow, cursorcolumn, item->rows, item->columns, count))
                {

                    cursorcolumn++;

                    if (cursorcolumn + item->columns > count)
                    {

                        cursorcolumn = 0;
                        cursorrow++;

                    }

                }

                item->row = cursorrow;
                item->column = cursorcolumn;
                cursorcolumn += item->columns;

            }

            occupy(cells, item);

        }

    }

}

static void sizegridcolumns(struct layout *layout, struct css_grid *grid, unsigned int base, unsigned int count, int width, int gap, int sizes[GRIDCOLUMNS])
{

    struct gridtrack tracks[GRIDCOLUMNS];
    int avail = width - gap * ((int)count - 1);
    int used = 0;
    int fractions = 0;
    int stretch = 0;
    unsigned int c;
    unsigned int i;

    for (c = 0; c < count; c++)
    {

        struct css_track track = columntrack(grid, c);
        int smallest = 0;
        int largest = 0;

        /* content only counts for items in this one column, wider ones are left to the leftover space */
        if (track.minkind >= CSS_TRACK_FR || track.minkind == CSS_TRACK_AUTO || track.maxkind >= CSS_TRACK_FR || track.maxkind == CSS_TRACK_AUTO)
        {

            for (i = base; i < layout->flexcount; i++)
            {

                struct layout_flexitem *item = &layout->flexitems[i];

                if (item->column != c || item->columns != 1)
                    continue;

                smallest = max(smallest, measuremin(layout, item->node) + item->ml + item->mr);

                if (track.maxkind == CSS_TRACK_AUTO || track.maxkind == CSS_TRACK_MAXCONTENT)
                    largest = max(largest, measure(layout, item->node, max(avail, 0)) + item->ml + item->mr);

            }

        }

        tracks[c].base = (track.minkind == CSS_TRACK_PX || track.minkind == CSS_TRACK_PERCENT) ? fixedsize(track.minkind, track.minvalue, width) : (track.minkind == CSS_TRACK_MAXCONTENT) ? largest : smallest;
        tracks[c].fraction = (track.maxkind == CSS_TRACK_FR) ? track.maxvalue : 0;
        tracks[c].limit = (track.maxkind == CSS_TRACK_PX || track.maxkind == CSS_TRACK_PERCENT) ? fixedsize(track.maxkind, track.maxvalue, width) : (track.maxkind == CSS_TRACK_MINCONTENT) ? smallest : (track.maxkind == CSS_TRACK_FR) ? tracks[c].base : max(largest, smallest);
        tracks[c].limit = max(tracks[c].limit, tracks[c].base);
        used += tracks[c].base;
        fractions += tracks[c].fraction;
        stretch += (track.maxkind == CSS_TRACK_AUTO);

    }

    /* tracks grow towards their limit first, what is left goes to the fractions or else to the auto tracks */
    for (c = 0; c < count; c++)
    {

        int room = tracks[c].limit - tracks[c].base;
        int grow = (tracks[c].fraction) ? 0 : min(room, max(avail - used, 0));

        sizes[c] = tracks[c].base + grow;
        used += grow;

    }

    if (fractions && avail > used)
    {

        int leftover = avail;

        for (c = 0; c < count; c++)
            leftover -= (tracks[c].fraction) ? 0 : sizes[c];

        for (c = 0; c < count; c++)
        {

            if (tracks[c].fraction)
                sizes[c] = max(tracks[c].base, leftover * tracks[c].fraction / fractions);

        }

    }

    else if (stretch && avail > used)
    {

        int extra = (avail - used) / stretch;

        for (c = 0; c < count; c++)
        {

            if (columntrack(grid, c).maxkind == CSS_TRACK_AUTO)
                sizes[c] += extra;

        }

    }

}

static void layoutgrid(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    struct css_grid *grid = (style->grid) ? &layout->grids[style->grid - 1] : 0;
    int width = flow->right - flow->left;
    int columngap = style->columngap;
    int rowgap = style->rowgap;
    int fixed = layout->boxes[node].definite - 1;
    unsigned int base = layout->flexcount;
    unsigned int ordered = isordered(layout, node);
    int sizes[GRIDCOLUMNS];
    int offsets[GRIDCOLUMNS + 1];
    int heights[GRIDROWS];
    int tops[GRIDROWS + 1];
    unsigned int count;
    unsigned int rows = 0;
    unsigned int child;
    unsigned int i;
    unsigned int r;
    int fractions = 0;
    int y = flow->y;

    for (child = nextitem(layout, node, 0, ordered); child; child = nextitem(layout, node, child, ordered))
    {

        struct css_style *childstyle = getstyle(layout, child);
        struct layout_flexitem *item;

        if (layout->flexcount >= LAYOUT_FLEXITEMS)
        {

            layout->full = 1;

            break;

        }

        item = &layout->flexitems[layout->flexcount++];
        item->node = child;
        item->ml = resolve(&childstyle->margin[CSS_LEFT], width);
        item->mr = resolve(&childstyle->margin[CSS_RIGHT], width);
        item->mt = resolve(&childstyle->margin[CSS_TOP], width);
        item->mb = resolve(&childstyle->margin[CSS_BOTTOM], width);

    }

    count = countcolumns(grid, width, columngap, layout->flexcount - base);

    placegriditems(layout, grid, base, count);
    sizegridcolumns(layout, grid, base, count, width, columngap, sizes);

    offsets[0] = 0;

    for (i = 0; i < count; i++)
        offsets[i + 1] = offsets[i] + sizes[i] + columngap;

    for (i = base; i < layout->flexcount; i++)
        rows = max(rows, layout->flexitems[i].row + layout->flexitems[i].rows);

    rows = max(rows, (grid) ? max(grid->rowcount, grid->arearows) : 0);
    rows = min(rows, GRIDROWS);

    /* rows are filled top down, an item spanning rows makes the last of them big enough */
    for (r = 0; r < rows; r++)
    {

        struct css_track track = rowtrack(grid, r);
        int height = fixedsize(track.minkind, track.minvalue, (fixed >= 0) ? fixed : 0);

        tops[r] = y;

        for (i = base; i < layout->flexcount; i++)
        {

            struct layout_flexitem *item = &layout->flexitems[i];
            struct css_style *itemstyle = getstyle(layout, item->node);
            int span = offsets[item->column + item->columns] - offsets[item->column] - columngap;

            if (item->row != r)
                continue;

            item->size = (itemstyle->width.unit == CSS_UNIT_PX || itemstyle->width.unit == CSS_UNIT_PERCENT) ? min(basesize(layout, item->node, span, 0), max(span - item->ml - item->mr, 0)) : max(span - item->ml - item->mr, 0);

            placeflexitem(layout, item, flow->left + offsets[item->column], y);

        }

        for (i = base; i < layout->flexcount; i++)
        {

            struct layout_flexitem *item = &layout->flexitems[i];

            if (item->row + item->rows - 1 == r)
                height = max(height, item->mt + item->height + item->mb - (y - tops[item->row]));

        }

        if (track.maxkind == CSS_TRACK_PX || track.maxkind == CSS_TRACK_PERCENT)
            height = fixedsize(track.maxkind, track.maxvalue, (fixed >= 0) ? fixed : 0);

        fractions += (track.maxkind == CSS_TRACK_FR) ? track.maxvalue : 0;
        heights[r] = height;
        y += height + rowgap;

    }

    tops[rows] = y;

    /* flexible rows share the height a container with one has left */
    if (fixed >= 0 && fractions && !layout->measuring && fixed > y - flow->y - rowgap)
    {

        int extra = fixed - (y - flow->y - rowgap);
        int shift = 0;

        for (r = 0; r < rows; r++)
        {

            struct css_track track = rowtrack(grid, r);
            int grow = (track.maxkind == CSS_TRACK_FR) ? extra * track.maxvalue / fractions : 0;

            tops[r] += shift;
            heights[r] += grow;
            shift += grow;

        }

        tops[rows] += shift;

        for (i = base; i < layout->flexcount; i++)
        {

            struct layout_flexitem *item = &layout->flexitems[i];
            int moved = tops[item->row] - layout->boxes[item->node].y + item->mt;

            if (moved)
                translate(layout, item->node, item->fragmentstart, item->fragmentend, 0, moved, 1);

        }

    }

    /* every item is stretched over its rows unless it says otherwise */
    for (i = base; i < layout->flexcount; i++)
    {

        struct layout_flexitem *item = &layout->flexitems[i];
        unsigned int last = min(item->row + item->rows, rows);
        int span = tops[last] - tops[item->row] - rowgap;

        alignflexitem(layout, item, getalign(style, getstyle(layout, item->node)), span - item->mt - item->height - item->mb, 0);

    }

    layout->flexcount = base;
    flow->y = (rows) ? tops[rows] - rowgap : y;

}

static void layoutflexrow(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    int availw = flow->right - flow->left;
    int gap = style->columngap;
    unsigned int justify = (style->reverse && style->justify == CSS_FLEX_START) ? CSS_FLEX_END : (style->reverse && style->justify == CSS_FLEX_END) ? CSS_FLEX_START : style->justify;
    int fixed = layout->boxes[node].definite - 1;
    int minimum = flexminimum(layout, node, availw);
    unsigned int base = layout->flexcount;
    unsigned int ordered = isordered(layout, node);
    unsigned int lines[FLEXLINES];
    unsigned int linecount = 0;
    unsigned int start;
    unsigned int child;
    int y = flow->y;

    for (child = nextitem(layout, node, 0, ordered); child; child = nextitem(layout, node, child, ordered))
    {

        struct css_style *childstyle;
        struct layout_flexitem *item;

        if (layout->flexcount >= LAYOUT_FLEXITEMS)
        {

            layout->full = 1;

            break;

        }

        childstyle = getstyle(layout, child);
        item = &layout->flexitems[layout->flexcount++];
        item->node = child;
        item->ml = resolve(&childstyle->margin[CSS_LEFT], availw);
        item->mr = resolve(&childstyle->margin[CSS_RIGHT], availw);
        item->mt = resolve(&childstyle->margin[CSS_TOP], availw);
        item->mb = resolve(&childstyle->margin[CSS_BOTTOM], availw);
        item->automargins = (childstyle->margin[CSS_LEFT].unit == CSS_UNIT_AUTO) + (childstyle->margin[CSS_RIGHT].unit == CSS_UNIT_AUTO) * 2;
        item->size = basesize(layout, child, availw, !layout->measuring);

    }

    for (start = base; start < layout->flexcount;)
    {

        unsigned int end;
        unsigned int autos = 0;
        unsigned int count;
        unsigned int i;
        int used = 0;
        int free;
        int offset = 0;
        int spacing = 0;
        unsigned int fragmentcount;
        unsigned int textcount;
        unsigned int absolutecount;
        int cross = 0;
        int x;

        for (end = start; end < layout->flexcount; end++)
        {

            struct layout_flexitem *item = &layout->flexitems[end];
            int outer = item->ml + item->size + item->mr + ((end > start) ? gap : 0);

            /* every item may have been rounded up by half a pixel, which must not push the last one down */
            if (style->wrap && end > start && used + outer > availw + (int)(end - start + 2) / 2)
                break;

            used += outer;

        }

        count = end - start;
        free = availw - used;

        /* a container being measured takes the width of its items, so no free space is handed out */
        if (!layout->measuring && free)
        {

            flexsizes(layout, start, end, free, availw);

            for (used = 0, i = start; i < end; i++)
                used += layout->flexitems[i].ml + layout->flexitems[i].size + layout->flexitems[i].mr + ((i > start) ? gap : 0);

            free = availw - used;

        }

        for (i = start; i < end; i++)
            autos += (layout->flexitems[i].automargins & 1) + (layout->flexitems[i].automargins >> 1);

        if (!layout->measuring && free > 0 && autos)
        {

            for (i = start; i < end; i++)
            {

                struct layout_flexitem *item = &layout->flexitems[i];

                if (item->automargins & 1)
                    item->ml += free / (int)autos;

                if (item->automargins & 2)
                    item->mr += free / (int)autos;

            }

            free = 0;

        }

        if (!layout->measuring && free > 0)
        {

            switch (justify)
            {

            case CSS_FLEX_END:
                offset = free;

                break;

            case CSS_FLEX_CENTER:
                offset = free / 2;

                break;

            case CSS_FLEX_BETWEEN:
                spacing = (count > 1) ? free / (int)(count - 1) : 0;

                break;

            case CSS_FLEX_AROUND:
                spacing = free / (int)count;
                offset = spacing / 2;

                break;

            case CSS_FLEX_EVENLY:
                spacing = free / (int)(count + 1);
                offset = spacing;

                break;

            }

        }

        x = flow->left + offset;
        fragmentcount = layout->fragmentcount;
        textcount = layout->textcount;
        absolutecount = layout->absolutecount;

        for (i = start; i < end; i++)
        {

            struct layout_flexitem *item = &layout->flexitems[i];

            if (i > start)
                x += gap + spacing;

            x += placeflexitem(layout, item, x, y);
            cross = max(cross, item->mt + item->height + item->mb);

        }

        /* a single line is as tall as a container with a height, even when its items are taller */
        if (start == base && end == layout->flexcount)
            cross = max((fixed >= 0) ? fixed : cross, minimum);

        /* a stretched item has a definite height, so the line is placed again for children sized by percent */
        if (!layout->measuring && needsrestretch(layout, style, start, end, cross))
        {

            layout->fragmentcount = fragmentcount;
            layout->textcount = textcount;
            layout->absolutecount = absolutecount;

            for (i = start; i < end; i++)
            {

                struct layout_flexitem *item = &layout->flexitems[i];
                int left = layout->boxes[item->node].x - item->ml;

                if (isstretched(layout, style, item))
                {

                    layout->heightnode = item->node;
                    layout->forceheight = cross - item->mt - item->mb;

                }

                placeflexitem(layout, item, left, y);

                layout->heightnode = 0;

            }

        }

        for (i = start; i < end; i++)
        {

            struct layout_flexitem *item = &layout->flexitems[i];

            alignflexitem(layout, item, getalign(style, getstyle(layout, item->node)), cross - item->mt - item->height - item->mb, 0);

        }

        y += cross;

        if (linecount < FLEXLINES)
            lines[linecount++] = start;

        start = end;

        if (start < layout->flexcount)
            y += style->rowgap;

    }

    /* lines of a wrapping container with a height share what is left of it, each item aligns in its taller line */
    if (!layout->measuring && linecount > 1 && fixed > y - flow->y)
    {

        int extra = (fixed - (y - flow->y)) / (int)linecount;
        unsigned int line;

        for (line = 0; line < linecount; line++)
        {

            unsigned int last = (line + 1 < linecount) ? lines[line + 1] : layout->flexcount;
            unsigned int i;

            for (i = lines[line]; i < last; i++)
            {

                struct layout_flexitem *item = &layout->flexitems[i];

                if (line)
                    translate(layout, item->node, item->fragmentstart, item->fragmentend, 0, extra * (int)line, 1);

                alignflexitem(layout, item, getalign(style, getstyle(layout, item->node)), extra, 0);

            }

        }

        y = flow->y + fixed;

    }

    layout->flexcount = base;
    flow->y = y;
    flow->pending = 0;

}

static unsigned int countautomargins(struct css_style *style, unsigned int first, unsigned int second)
{

    return (style->margin[first].unit == CSS_UNIT_AUTO) + (style->margin[second].unit == CSS_UNIT_AUTO);

}

static unsigned int columnalign(struct css_style *container, struct css_style *style)
{

    /* auto margins across a column push the item over instead of stretching it */
    if (style->margin[CSS_LEFT].unit == CSS_UNIT_AUTO)
        return (style->margin[CSS_RIGHT].unit == CSS_UNIT_AUTO) ? CSS_FLEX_CENTER : CSS_FLEX_END;

    if (style->margin[CSS_RIGHT].unit == CSS_UNIT_AUTO)
        return CSS_FLEX_START;

    return getalign(container, style);

}

static unsigned int needsregrow(struct layout *layout, unsigned int base)
{

    unsigned int i;

    for (i = base; i < layout->flexcount; i++)
    {

        struct css_style *style = getstyle(layout, layout->flexitems[i].node);

        if (style->grow && (style->display == CSS_DISPLAY_FLEX || style->display == CSS_DISPLAY_GRID || haspercentchild(layout, layout->flexitems[i].node)))
            return 1;

    }

    return 0;

}

static void layoutflexcolumn(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    int availw = flow->right - flow->left;
    int gap = (style->rowgap) ? style->rowgap : style->columngap;
    int fixed = layout->boxes[node].definite - 1;
    int minimum = flexminimum(layout, node, availw);
    unsigned int base = layout->flexcount;
    unsigned int ordered = isordered(layout, node);
    unsigned int fragmentcount = layout->fragmentcount;
    unsigned int textcount = layout->textcount;
    unsigned int absolutecount = layout->absolutecount;
    unsigned int child;
    unsigned int i;
    int y = flow->y;

    for (child = nextitem(layout, node, 0, ordered); child; child = nextitem(layout, node, child, ordered))
    {

        struct css_style *childstyle;
        struct layout_flexitem *item;
        unsigned int align;
        int outer;

        if (layout->flexcount >= LAYOUT_FLEXITEMS)
        {

            layout->full = 1;

            break;

        }

        childstyle = getstyle(layout, child);
        align = columnalign(style, childstyle);
        item = &layout->flexitems[layout->flexcount++];
        item->node = child;
        item->ml = resolve(&childstyle->margin[CSS_LEFT], availw);
        item->mr = resolve(&childstyle->margin[CSS_RIGHT], availw);
        item->mt = resolve(&childstyle->margin[CSS_TOP], availw);
        item->mb = resolve(&childstyle->margin[CSS_BOTTOM], availw);
        /* while shrinking to fit, a stretched item asks only for its content so the column can be narrow */
        item->size = (align == CSS_FLEX_STRETCH && !layout->measuring && childstyle->width.unit != CSS_UNIT_PX && childstyle->width.unit != CSS_UNIT_PERCENT && !isreplaced(layout, child)) ? max(availw - item->ml - item->mr, 0) : min(basesize(layout, child, availw, 0), max(availw - item->ml - item->mr, 0));

        if (y > flow->y)
            y += gap;

        outer = placeflexitem(layout, item, flow->left, y);

        alignflexitem(layout, item, align, availw - outer, 1);

        y += item->mt + item->height + item->mb;

    }

    /* justify-content needs a height to share, so only a container with one gets it */
    /* a min-height only gives room when the items leave some */
    if (fixed < 0 && minimum > y - flow->y)
        fixed = minimum;

    if (fixed >= 0 && !layout->measuring && layout->flexcount > base)
    {

        int free = fixed - (y - flow->y);
        unsigned int count = layout->flexcount - base;
        unsigned int automargins = 0;
        unsigned int grow = 0;
        int offset = 0;
        int spacing = 0;

        for (i = base; i < layout->flexcount; i++)
            automargins += countautomargins(getstyle(layout, layout->flexitems[i].node), CSS_TOP, CSS_BOTTOM);

        for (i = base; i < layout->flexcount; i++)
            grow += getstyle(layout, layout->flexitems[i].node)->grow;

        /* a grown item whose content is placed against its height is laid out again at that height */
        if (free > 0 && grow && needsregrow(layout, base))
        {

            layout->fragmentcount = fragmentcount;
            layout->textcount = textcount;
            layout->absolutecount = absolutecount;
            y = flow->y;

            for (i = base; i < layout->flexcount; i++)
            {

                struct layout_flexitem *item = &layout->flexitems[i];
                int share = free * getstyle(layout, item->node)->grow / (int)grow;

                if (i > base)
                    y += gap;

                if (share > 0)
                {

                    layout->heightnode = item->node;
                    layout->forceheight = item->height + share;

                }

                alignflexitem(layout, item, columnalign(style, getstyle(layout, item->node)), availw - placeflexitem(layout, item, flow->left, y), 1);

                layout->heightnode = 0;
                y += item->mt + item->height + item->mb;

            }

        }

        /* growing items take the free height first, the box gets taller and the rest moves down */
        else if (free > 0 && grow)
        {

            for (i = base; i < layout->flexcount; i++)
            {

                struct layout_flexitem *item = &layout->flexitems[i];
                int share = free * getstyle(layout, item->node)->grow / (int)grow;

                if (offset)
                    translate(layout, item->node, item->fragmentstart, item->fragmentend, 0, offset, 1);

                layout->boxes[item->node].h += share;
                offset += share;

            }

        }

        /* auto margins take the free height before justify-content gets any */
        else if (free > 0 && automargins)
        {

            for (i = base; i < layout->flexcount; i++)
            {

                struct layout_flexitem *item = &layout->flexitems[i];
                struct css_style *childstyle = getstyle(layout, item->node);

                if (childstyle->margin[CSS_TOP].unit == CSS_UNIT_AUTO)
                    offset += free / (int)automargins;

                if (offset)
                    translate(layout, item->node, item->fragmentstart, item->fragmentend, 0, offset, 1);

                if (childstyle->margin[CSS_BOTTOM].unit == CSS_UNIT_AUTO)
                    offset += free / (int)automargins;

            }

        }

        /* items that overflow are still centered or pushed to the end, past the top if need be */
        else if (free > 0 || style->justify == CSS_FLEX_CENTER || style->justify == CSS_FLEX_END)
        {

            switch (style->justify)
            {

            case CSS_FLEX_END:
                offset = free;

                break;

            case CSS_FLEX_CENTER:
                offset = free / 2;

                break;

            case CSS_FLEX_BETWEEN:
                spacing = (count > 1) ? free / (int)(count - 1) : 0;

                break;

            case CSS_FLEX_AROUND:
                spacing = free / (int)count;
                offset = spacing / 2;

                break;

            case CSS_FLEX_EVENLY:
                spacing = free / (int)(count + 1);
                offset = spacing;

                break;

            }

            for (i = base; i < layout->flexcount; i++)
            {

                struct layout_flexitem *item = &layout->flexitems[i];
                int delta = offset + spacing * (int)(i - base);

                if (delta)
                    translate(layout, item->node, item->fragmentstart, item->fragmentend, 0, delta, 1);

            }

        }

    }

    layout->flexcount = base;
    flow->y = y;
    flow->pending = 0;

}

static unsigned int isroot(struct layout *layout, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    unsigned int parent = getnode(layout, node)->parent;
    struct css_style *container = (parent) ? getstyle(layout, parent) : 0;

    if (style->floating || style->overflow || isabsolute(style) || style->flexbox || style->display == CSS_DISPLAY_INLINEBLOCK || style->display == CSS_DISPLAY_TABLE || style->display == CSS_DISPLAY_ROW || style->display == CSS_DISPLAY_CELL || style->display == CSS_DISPLAY_GRID || style->display == CSS_DISPLAY_FLEX)
        return 1;

    return !container || container->flexbox || container->display == CSS_DISPLAY_FLEX || container->display == CSS_DISPLAY_GRID;

}

static unsigned int findcontainer(struct layout *layout, unsigned int node)
{

    unsigned int html = layout->document->html;

    if (getstyle(layout, node)->position == CSS_POSITION_FIXED)
        return html;

    for (node = getnode(layout, node)->parent; node && node != html; node = getnode(layout, node)->parent)
    {

        struct css_style *style = getstyle(layout, node);

        if (style->position != CSS_POSITION_STATIC && style->display != CSS_DISPLAY_INLINE)
            return node;

    }

    return html;

}

static int staticshift(struct layout *layout, unsigned int node, int top, int outer)
{

    unsigned int parent = getnode(layout, node)->parent;
    struct css_style *style = getstyle(layout, parent);
    struct layout_box *box = &layout->boxes[parent];
    unsigned int align = getalign(style, getstyle(layout, node));
    int inner = box->y + style->borderwidth[CSS_TOP] + resolve(&style->padding[CSS_TOP], box->w);
    int space = box->h - style->borderwidth[CSS_TOP] - style->borderwidth[CSS_BOTTOM] - resolve(&style->padding[CSS_TOP], box->w) - resolve(&style->padding[CSS_BOTTOM], box->w) - outer;

    /* a box taken out of a flex row still lines up with the other items when nothing places it */
    if (!(style->flexbox || style->display == CSS_DISPLAY_FLEX) || style->column)
        return 0;

    if (align == CSS_FLEX_CENTER)
        return inner + space / 2 - top;

    if (align == CSS_FLEX_END)
        return inner + space - top;

    return 0;

}

static void placeabsolute(struct layout *layout, unsigned int container, struct layout_absolute *absolute)
{

    struct css_style *outer = getstyle(layout, container);
    struct layout_box *box = &layout->boxes[container];
    struct css_style *style = getstyle(layout, absolute->node);
    int cx = box->x + outer->borderwidth[CSS_LEFT];
    int cy = box->y + outer->borderwidth[CSS_TOP];
    int cw = box->w - outer->borderwidth[CSS_LEFT] - outer->borderwidth[CSS_RIGHT];
    int ch = box->h - outer->borderwidth[CSS_TOP] - outer->borderwidth[CSS_BOTTOM];
    unsigned int hasleft = style->inset[CSS_LEFT].unit <= CSS_UNIT_PERCENT;
    unsigned int hasright = style->inset[CSS_RIGHT].unit <= CSS_UNIT_PERCENT;
    unsigned int hastop = style->inset[CSS_TOP].unit <= CSS_UNIT_PERCENT;
    unsigned int hasbottom = style->inset[CSS_BOTTOM].unit <= CSS_UNIT_PERCENT;
    int left = (hasleft) ? resolve(&style->inset[CSS_LEFT], cw) : 0;
    int right = (hasright) ? resolve(&style->inset[CSS_RIGHT], cw) : 0;
    int ml = resolve(&style->margin[CSS_LEFT], cw);
    int mr = resolve(&style->margin[CSS_RIGHT], cw);
    int mt = resolve(&style->margin[CSS_TOP], cw);
    int mb = resolve(&style->margin[CSS_BOTTOM], cw);
    unsigned int fragmentstart = layout->fragmentcount;
    int extent = layout->extent;
    unsigned int haswidth = style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT;
    unsigned int heightnode;
    int forceheight;
    int width;
    int x;
    int y;
    int height;

    /* a fixed box is placed against the first screen, since the page scrolls as one picture */
    if (style->position == CSS_POSITION_FIXED)
    {

        cy = 0;
        ch = layout->viewheight;

    }

    if (!haswidth && hasleft && hasright && !isreplaced(layout, absolute->node))
        width = max(cw - left - right - ml - mr, 0);
    else
        width = measure(layout, absolute->node, (haswidth) ? cw : max(cw - left - right, 0));

    x = (hasleft) ? cx + left : (hasright) ? cx + cw - right - ml - width - mr : absolute->x;
    y = ((hastop) ? cy + resolve(&style->inset[CSS_TOP], ch) : absolute->y) + mt;

    heightnode = layout->heightnode;
    forceheight = layout->forceheight;

    /* heights against the containing block are known here, before the box is laid out */
    if (style->height.unit == CSS_UNIT_PERCENT)
    {

        layout->heightnode = absolute->node;
        layout->forceheight = resolve(&style->height, ch) + ((style->borderbox) ? 0 : style->borderwidth[CSS_TOP] + style->borderwidth[CSS_BOTTOM] + resolve(&style->padding[CSS_TOP], cw) + resolve(&style->padding[CSS_BOTTOM], cw));

    }

    else if (style->height.unit == CSS_UNIT_AUTO && hastop && hasbottom)
    {

        layout->heightnode = absolute->node;
        layout->forceheight = max(ch - resolve(&style->inset[CSS_TOP], ch) - resolve(&style->inset[CSS_BOTTOM], ch) - mt - mb, 0);

    }

    height = placesized(layout, absolute->node, x, y, ml, mr, width);
    layout->heightnode = heightnode;
    layout->forceheight = forceheight;

    if (!hastop && hasbottom)
        translate(layout, absolute->node, fragmentstart, layout->fragmentcount, 0, cy + ch - resolve(&style->inset[CSS_BOTTOM], ch) - mb - height - y, 1);
    else if (!hastop)
        translate(layout, absolute->node, fragmentstart, layout->fragmentcount, 0, staticshift(layout, absolute->node, y - mt, mt + height + mb), 1);

    layout->extent = extent;

}

static void queuechildren(struct layout *layout, unsigned int node, int x, int y)
{

    unsigned int child;

    for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
    {

        if (getnode(layout, child)->type == HTML_NODE_ELEMENT && isabsolute(getstyle(layout, child)))
            queueabsolute(layout, child, x, y);

    }

}

static void placeabsolutes(struct layout *layout, unsigned int node, unsigned int start)
{

    unsigned int i = start;

    while (i < layout->absolutecount)
    {

        struct layout_absolute absolute;
        unsigned int j;

        if (findcontainer(layout, layout->absolutes[i].node) != node)
        {

            i++;

            continue;

        }

        absolute.node = layout->absolutes[i].node;
        absolute.x = layout->absolutes[i].x;
        absolute.y = layout->absolutes[i].y;

        for (j = i + 1; j < layout->absolutecount; j++)
        {

            layout->absolutes[j - 1].node = layout->absolutes[j].node;
            layout->absolutes[j - 1].x = layout->absolutes[j].x;
            layout->absolutes[j - 1].y = layout->absolutes[j].y;

        }

        layout->absolutecount--;

        placeabsolute(layout, node, &absolute);

    }

}

static void shiftrelative(struct layout *layout, unsigned int node, unsigned int fragmentstart, int availw)
{

    struct css_style *style = getstyle(layout, node);
    int dx = 0;
    int dy = 0;

    if (style->inset[CSS_LEFT].unit <= CSS_UNIT_PERCENT)
        dx = resolve(&style->inset[CSS_LEFT], availw);
    else if (style->inset[CSS_RIGHT].unit <= CSS_UNIT_PERCENT)
        dx = -resolve(&style->inset[CSS_RIGHT], availw);

    if (style->inset[CSS_TOP].unit <= CSS_UNIT_PERCENT)
        dy = resolve(&style->inset[CSS_TOP], availw);
    else if (style->inset[CSS_BOTTOM].unit <= CSS_UNIT_PERCENT)
        dy = -resolve(&style->inset[CSS_BOTTOM], availw);

    if (dx || dy)
        translate(layout, node, fragmentstart, layout->fragmentcount, dx, dy, 1);

}

static int floatbottom(struct layout *layout, unsigned int start, int y)
{

    unsigned int i;

    for (i = start; i < layout->floatcount; i++)
        y = max(y, layout->floats[i].bottom);

    return y;

}

static int definiteheight(struct layout *layout, unsigned int node, int availw)
{

    struct css_style *style = getstyle(layout, node);
    unsigned int parent = getnode(layout, node)->parent;
    int vertical = style->borderwidth[CSS_TOP] + style->borderwidth[CSS_BOTTOM] + resolve(&style->padding[CSS_TOP], availw) + resolve(&style->padding[CSS_BOTTOM], availw);
    int inner = (style->borderbox) ? vertical : 0;

    if (layout->heightnode == node)
        return max(layout->forceheight - vertical, 0);

    if (style->height.unit == CSS_UNIT_PX)
        return max(style->height.value - inner, 0);

    /* a percentage needs a parent whose height does not depend on its content */
    if (style->height.unit == CSS_UNIT_PERCENT && parent && layout->boxes[parent].definite)
        return max(resolve(&style->height, layout->boxes[parent].definite - 1) - inner, 0);

    return -1;

}

static int layoutblock(struct layout *layout, unsigned int node, int x, int top, int availw)
{

    struct css_style *style = getstyle(layout, node);
    struct layout_box *box = &layout->boxes[node];
    int ml = resolve(&style->margin[CSS_LEFT], availw);
    int mr = resolve(&style->margin[CSS_RIGHT], availw);
    int bl = style->borderwidth[CSS_LEFT];
    int br = style->borderwidth[CSS_RIGHT];
    int bt = style->borderwidth[CSS_TOP];
    int bb = style->borderwidth[CSS_BOTTOM];
    int pl = resolve(&style->padding[CSS_LEFT], availw);
    int pr = resolve(&style->padding[CSS_RIGHT], availw);
    int pt = resolve(&style->padding[CSS_TOP], availw);
    int pb = resolve(&style->padding[CSS_BOTTOM], availw);
    int extra = bl + br + pl + pr;
    int cw = availw - ml - mr - extra;
    unsigned int constrained = 0;
    unsigned int sized = 0;
    unsigned int capped = 0;
    unsigned int wrapped = layout->wrapped;
    unsigned int floatstart = layout->floatcount;
    unsigned int floatbase = layout->floatbase;
    unsigned int absolutestart = layout->absolutecount;
    unsigned int fragmentstart = layout->fragmentcount;
    int extent = layout->extent;
    struct columns columns;
    struct flow flow;
    int contentheight;
    int trailing;
    unsigned int child;

    if (isreplaced(layout, node))
        return layoutreplaced(layout, node, x, top, availw);

    if (haswidth(layout, node))
    {

        cw = resolve(&style->width, availw) - ((style->borderbox) ? extra : 0);
        constrained = 1;
        sized = 1;

    }

    if (style->maxwidth.unit == CSS_UNIT_PX || style->maxwidth.unit == CSS_UNIT_PERCENT)
    {

        int limit = resolve(&style->maxwidth, availw) - ((style->borderbox) ? extra : 0);

        if (cw > limit)
        {

            cw = limit;
            constrained = 1;
            capped = 1;

        }

    }

    /* a flex container has already decided this item's width */
    if (layout->forcenode == node)
    {

        cw = layout->forcewidth - extra;
        constrained = 1;
        sized = 1;
        layout->forcenode = 0;

    }

    if (layout->marginnode == node)
    {

        ml = layout->forceleft;
        mr = layout->forceright;

    }

    if (style->display == CSS_DISPLAY_TABLE || style->display == CSS_DISPLAY_ROW)
    {

        int width = computecolumns(layout, node, &columns, cw, style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT);

        if (!(style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT) && width < cw)
        {

            cw = width;
            constrained = 1;
            sized = 1;

        }

    }

    cw = max(cw, resolve(&style->minwidth, availw) - ((style->borderbox) ? extra : 0));
    cw = max(cw, 0);

    if (constrained)
    {

        int free = availw - cw - extra;

        if (style->margin[CSS_LEFT].unit == CSS_UNIT_AUTO && style->margin[CSS_RIGHT].unit == CSS_UNIT_AUTO)
            ml = max(free / 2, 0);
        else if (style->margin[CSS_LEFT].unit == CSS_UNIT_AUTO)
            ml = max(free - mr, 0);

    }

    box->placed = 1;
    box->x = x + ml;
    box->y = top;
    box->definite = definiteheight(layout, node, availw) + 1;

    /* an aspect ratio turns the width into a height when nothing else gives one */
    if (!box->definite && style->aspect)
        box->definite = max(((style->borderbox) ? cw + extra : cw) * 1000 / (int)style->aspect - ((style->borderbox) ? bt + bb + pt + pb : 0), 0) + 1;

    if (isroot(layout, node))
        layout->floatbase = floatstart;

    layout->extent = 0;

    initflow(layout, &flow, node, box->x + bl + pl, box->x + bl + pl + cw, top + bt + pt);

    flow.collapsed = isopen(layout, node, CSS_TOP);

    if (style->display == CSS_DISPLAY_LISTITEM && style->liststyle != CSS_LIST_NONE)
        setmarker(layout, node, flow.left, flow.y);

    if (style->display == CSS_DISPLAY_ROW)
    {

        layoutrow(layout, &flow, node, &columns);

    }

    else if (style->display == CSS_DISPLAY_TABLE && isrow(layout, node))
    {

        int boxx = box->x;
        int boxy = box->y;

        layoutrow(layout, &flow, node, &columns);

        box->x = boxx;
        box->y = boxy;

    }

    else if (style->display == CSS_DISPLAY_TABLE)
    {

        layouttable(layout, &flow, node, &columns);

    }

    else if (style->display == CSS_DISPLAY_GRID)
    {

        queuechildren(layout, node, flow.left, flow.y);
        layoutgrid(layout, &flow, node);

    }

    else if (style->flexbox && !hastext(layout, node))
    {

        queuechildren(layout, node, flow.left, flow.y);

        if (style->column)
            layoutflexcolumn(layout, &flow, node);
        else
            layoutflexrow(layout, &flow, node);

    }

    else
    {

        for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
            layoutnode(layout, &flow, child);

        flushline(layout, &flow, 0);

    }

    if (layout->marker && layout->fragments[layout->marker - 1].node == node)
        placemarker(layout, top + bt + pt + layout->fragments[layout->marker - 1].ascent);

    /* a last child's bottom margin passes through a parent without border, padding or height */
    trailing = (isopen(layout, node, CSS_BOTTOM) && !box->definite && !(style->minheight.unit == CSS_UNIT_PX && style->minheight.value > 0)) ? flow.pending : 0;
    contentheight = flow.y + max(flow.pending - trailing, 0) - (top + bt + pt);

    /* only a formatting context root holds its floats, elsewhere they reach into what follows until cleared */
    if (isroot(layout, node))
    {

        contentheight = max(contentheight, floatbottom(layout, floatstart, top + bt + pt) - (top + bt + pt));
        layout->floatcount = floatstart;

    }

    layout->floatbase = floatbase;

    layout->natural = bt + pt + contentheight + pb + bb;

    if (box->definite)
        contentheight = box->definite - 1;

    if (style->maxheight.unit == CSS_UNIT_PX)
        contentheight = min(contentheight, max(style->maxheight.value - ((style->borderbox) ? bt + bb + pt + pb : 0), 0));

    if (style->minheight.unit == CSS_UNIT_PX)
        contentheight = max(contentheight, style->minheight.value - ((style->borderbox) ? bt + bb + pt + pb : 0));

    box->w = bl + pl + cw + pr + br;
    box->h = bt + pt + contentheight + pb + bb;

    /* content is measured to the right edge of the border, so nested padding counts when shrinking to fit */
    if (layout->extent > 0)
        layout->extent += pr + br;

    /* a max-width only caps the box, it is as wide as the cap when its content had to wrap to fit */
    if (sized || (capped && layout->wrapped != wrapped))
        layout->extent = max(layout->extent, box->x + box->w);

    /* a parent shrinking to fit makes room for the margin too, measure takes its own box's off again */
    if (layout->extent > 0)
        layout->extent += mr;

    layout->extent = max(layout->extent, extent);

    if (!layout->measuring && (style->position != CSS_POSITION_STATIC || node == layout->document->html))
        placeabsolutes(layout, node, absolutestart);

    if (!layout->measuring && style->position == CSS_POSITION_RELATIVE)
        shiftrelative(layout, node, fragmentstart, availw);

    /* a transform moves the box once it has its size, percentages being of that size */
    if (!layout->measuring && (resolve(&style->shift[0], box->w) || resolve(&style->shift[1], box->h)))
        translate(layout, node, fragmentstart, layout->fragmentcount, resolve(&style->shift[0], box->w), resolve(&style->shift[1], box->h), 1);

    layout->trailing = trailing;

    return box->h;

}

static int stackkey(struct layout *layout, unsigned int node)
{

    for (; node; node = getnode(layout, node)->parent)
    {

        struct css_style *style = getstyle(layout, node);

        if (style->stacked && style->position != CSS_POSITION_STATIC)
            return style->zindex;

    }

    return 0;

}

static void setlayers(struct layout *layout)
{

    struct html_document *document = layout->document;
    unsigned int node = document->html;
    unsigned int i;

    /* the page itself is the first layer, then every positioned box starts one of its own */
    buffer_clear(layout->layerroot, sizeof (unsigned short) * document->nodecount);

    layout->layers[0] = 0;
    layout->layerkeys[0] = 0;
    layout->layercount = 1;

    while (node)
    {

        struct html_node *element = getnode(layout, node);
        struct css_style *style = getstyle(layout, node);

        layout->layerroot[node] = layout->layerroot[element->parent];

        if (element->type == HTML_NODE_ELEMENT && style->position != CSS_POSITION_STATIC && layout->boxes[node].placed && layout->layercount < LAYOUT_LAYERS)
        {

            int key = stackkey(layout, node);
            unsigned int j = layout->layercount++;

            /* layers go by z-index and stay in source order otherwise, the page first among those at zero */
            while (j > 0 && layout->layerkeys[j - 1] > key)
            {

                layout->layers[j] = layout->layers[j - 1];
                layout->layerkeys[j] = layout->layerkeys[j - 1];
                j--;

            }

            layout->layers[j] = node;
            layout->layerkeys[j] = key;
            layout->layerroot[node] = node;

        }

        node = html_next(document, node, document->html);

    }

    /* every node gets the place of its layer in the drawing order */
    buffer_clear(layout->layer, sizeof (unsigned short) * document->nodecount);

    for (i = 0; i < layout->layercount; i++)
        layout->layer[layout->layers[i]] = i;

    for (node = document->html; node; node = html_next(document, node, document->html))
    {

        if (layout->layerroot[node] != node)
            layout->layer[node] = layout->layer[getnode(layout, node)->parent];

    }

    /* the fragments are sorted by layer once, so each layer draws only its own */
    buffer_clear(layout->layerstart, sizeof (unsigned int) * (LAYOUT_LAYERS + 1));

    for (i = 0; i < layout->fragmentcount; i++)
        layout->layerstart[layout->layer[layout->fragments[i].node] + 1]++;

    for (i = 0; i < layout->layercount; i++)
        layout->layerstart[i + 1] += layout->layerstart[i];

    for (i = 0; i < layout->fragmentcount; i++)
        layout->layerfragments[layout->layerstart[layout->layer[layout->fragments[i].node]]++] = i;

    for (i = layout->layercount; i > 0; i--)
        layout->layerstart[i] = layout->layerstart[i - 1];

    layout->layerstart[0] = 0;

}

void layout_document(struct layout *layout, unsigned int width, unsigned int height)
{

    struct html_document *document = layout->document;
    unsigned int node;
    unsigned int i;

    layout->fragmentcount = 0;
    layout->itemcount = 0;
    layout->textcount = 0;
    layout->marker = 0;
    layout->markerdepth = 0;
    layout->atomicdepth = 0;
    layout->measuring = 0;
    layout->truncated = 0;
    layout->minimum = 0;
    layout->forcenode = 0;
    layout->heightnode = 0;
    layout->wrapped = 0;
    layout->wrapedge = 0x7FFFFFFF;
    layout->edgewraps = 0;
    layout->flexcount = 0;
    layout->floatcount = 0;
    layout->floatbase = 0;
    layout->absolutecount = 0;
    layout->extent = 0;
    layout->height = 0;
    layout->viewheight = height;
    layout->full = 0;

    for (node = 0; node < document->nodecount; node++)
    {

        layout->boxes[node].placed = 0;
        layout->boxes[node].definite = 0;
        layout->measures[node].availw = -1;

    }

    if (document->html && isvisible(getstyle(layout, document->html)))
        layout->height = layoutblock(layout, document->html, 0, 0, width);

    for (i = 0; i < layout->fragmentcount; i++)
        layout->height = max(layout->height, layout->fragments[i].y + layout->fragments[i].h);

    setlayers(layout);

}

static unsigned int findanchor(struct layout *layout, unsigned int node)
{

    while (node)
    {

        if (getnode(layout, node)->tag == HTML_TAG_A && html_getattribute(layout->document, node, "href"))
            return node;

        node = getnode(layout, node)->parent;

    }

    return 0;

}

static unsigned int hitfragment(struct layout *layout, unsigned int layer, int x, int y)
{

    unsigned int i;

    for (i = layout->layerstart[layer]; i < layout->layerstart[layer + 1]; i++)
    {

        struct layout_fragment *fragment = &layout->fragments[layout->layerfragments[i]];
        int slop = fragment->h / 4;

        if (x >= fragment->x && x < fragment->x + fragment->w && y >= fragment->y - slop && y < fragment->y + fragment->h + slop)
            return layout->layerfragments[i] + 1;

    }

    return 0;

}

static unsigned int hitbox(struct layout *layout, unsigned int layer, int x, int y)
{

    unsigned int found = 0;
    unsigned int node;

    for (node = 1; node < layout->document->nodecount; node++)
    {

        struct layout_box *box = &layout->boxes[node];

        if (layout->layer[node] == layer && box->placed && x >= box->x && x < box->x + box->w && y >= box->y && y < box->y + box->h)
            found = node;

    }

    return found;

}

unsigned int layout_findnode(struct layout *layout, int x, int y)
{

    unsigned int i;

    /* the layer drawn last is on top, so it is asked first */
    for (i = layout->layercount; i > 0; i--)
    {

        unsigned int layer = i - 1;
        unsigned int fragment = hitfragment(layout, layer, x, y);
        unsigned int box;

        if (fragment)
            return (layout->fragments[fragment - 1].type == LAYOUT_FRAGMENT_TEXT) ? getnode(layout, layout->fragments[fragment - 1].node)->parent : layout->fragments[fragment - 1].node;

        box = hitbox(layout, layer, x, y);

        if (box)
            return box;

    }

    return 0;

}

unsigned int layout_findlink(struct layout *layout, int x, int y)
{

    unsigned int node = layout_findnode(layout, x, y);

    return (node) ? findanchor(layout, node) : 0;

}

