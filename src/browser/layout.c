#include <fudge.h>
#include "html.h"
#include "css.h"
#include "font.h"
#include "layout.h"

#define COLUMNS                         16
#define SPACING                         2
#define MEASUREDEPTH                    3
#define MEASUREWIDTH                    8192
#define NONE                            0xFFFFFFFF
#define FLOW_NORMAL                     0
#define FLOW_FLEXROW                    1
#define FLOW_FLEXCOLUMN                 2
#define LABELSIZE                       128

struct flow
{

    int left;
    int right;
    int x;
    int y;
    int pending;
    unsigned int itemstart;
    unsigned int space;
    unsigned int node;
    unsigned int type;

};

struct metrics
{

    struct font_choice choice;
    int glyphheight;
    int glyphascent;
    int ascent;
    int descent;
    int space;

};

struct text
{

    unsigned int node;
    struct metrics metrics;
    char *data;
    unsigned int open;
    unsigned int wrap;
    unsigned int collapse;

};

struct columns
{

    int widths[COLUMNS];
    unsigned int count;

};

struct cell
{

    unsigned int node;
    unsigned int fragmentstart;
    unsigned int fragmentend;
    int height;

};

struct label
{

    char text[LABELSIZE];
    unsigned int length;

};

static int layoutblock(struct layout *layout, unsigned int node, int x, int top, int availw);
static void layoutnode(struct layout *layout, struct flow *flow, unsigned int node);

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

static int resolve(struct css_length *length, int base)
{

    switch (length->unit)
    {

    case CSS_UNIT_PX:
        return length->value;

    case CSS_UNIT_PERCENT:
        return base * (length->value / 10) / 10000;

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
    struct font_face *face;
    int lineheight;
    int halfleading;

    metrics.choice = font_choose(style->fontsize, style->bold);
    face = &layout->faces[metrics.choice.face];
    metrics.glyphheight = face->height * metrics.choice.scale;
    metrics.glyphascent = face->ascent * metrics.choice.scale;
    metrics.space = font_getglyph(face, ' ')->width * metrics.choice.scale;
    lineheight = (style->lineheight) ? style->lineheight : max(metrics.glyphheight + 2, style->fontsize * 6 / 5);
    halfleading = (lineheight - metrics.glyphheight) / 2;
    metrics.ascent = halfleading + metrics.glyphascent;
    metrics.descent = lineheight - metrics.ascent;

    return metrics;

}

static int measuretext(struct layout *layout, struct metrics *metrics, char *text, unsigned int length, unsigned int collapse)
{

    struct font_face *face = &layout->faces[metrics->choice.face];
    unsigned int space = 0;
    int width = 0;
    unsigned int i = 0;

    while (i < length)
    {

        unsigned int codepoint;

        if (collapse && isspace(text[i]))
        {

            if (!space)
                width += metrics->space;

            space = 1;
            i++;

            continue;

        }

        space = 0;
        i += utf8_read(text + i, length - i, &codepoint);
        width += font_getglyph(face, codepoint)->width * metrics->choice.scale;

    }

    return width;

}

static unsigned int fitbytes(struct layout *layout, struct metrics *metrics, char *text, unsigned int length, int width)
{

    struct font_face *face = &layout->faces[metrics->choice.face];
    unsigned int i = 0;
    int used = 0;

    while (i < length)
    {

        unsigned int codepoint;
        unsigned int size = utf8_read(text + i, length - i, &codepoint);

        used += font_getglyph(face, codepoint)->width * metrics->choice.scale;

        if (used > width && i)
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
    fragment->scale = metrics->choice.scale;
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

static void startline(struct layout *layout, struct flow *flow)
{

    if (isempty(layout, flow) && flow->pending)
    {

        flow->y += flow->pending;
        flow->pending = 0;

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

static void flushline(struct layout *layout, struct flow *flow, unsigned int forced)
{

    struct metrics strut = getmetrics(layout, getstyle(layout, flow->node));
    struct css_style *style = getstyle(layout, flow->node);
    int ascent = strut.ascent;
    int descent = strut.descent;
    int used = flow->x - flow->left;
    int dx = 0;
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

        return;

    }

    for (i = flow->itemstart; i < layout->itemcount; i++)
    {

        ascent = max(ascent, layout->items[i].ascent);
        descent = max(descent, layout->items[i].descent);

    }

    if (style->align == CSS_ALIGN_CENTER)
        dx = max((flow->right - flow->left - used) / 2, 0);
    else if (style->align == CSS_ALIGN_RIGHT)
        dx = max(flow->right - flow->left - used, 0);

    baseline = flow->y + ascent;

    for (i = flow->itemstart; i < layout->itemcount; i++)
    {

        struct layout_item *item = &layout->items[i];

        if (item->atomic)
        {

            translate(layout, item->node, item->fragmentstart, item->fragmentend, dx, baseline - item->ascent - flow->y, 1);

        }

        else if (item->fragmentstart != NONE)
        {

            layout->fragments[item->fragmentstart].x += dx;
            layout->fragments[item->fragmentstart].y = baseline - item->shift;

        }

    }

    if (layout->marker && layout->markerdepth == layout->atomicdepth && !layout->measuring)
        placemarker(layout, baseline);

    layout->extent = max(layout->extent, flow->left + used);
    layout->itemcount = flow->itemstart;
    flow->y = baseline + descent;
    flow->x = flow->left;
    flow->space = 0;

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

        flow->x += spacewidth;
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

static void placeword(struct layout *layout, struct flow *flow, struct text *text, unsigned int start, unsigned int end)
{

    char *data = text->data;
    int width = measuretext(layout, &text->metrics, data + start, end - start, 0);
    int spacewidth = (flow->space && !isempty(layout, flow)) ? text->metrics.space : 0;

    if (text->wrap && !isempty(layout, flow) && flow->x + spacewidth + width > flow->right)
    {

        closefragment(text);
        flushline(layout, flow, 0);

        spacewidth = 0;

    }

    while (text->wrap && isempty(layout, flow) && width > flow->right - flow->left && end - start > 1)
    {

        unsigned int split = fitbytes(layout, &text->metrics, data + start, end - start, flow->right - flow->left);

        if (split >= end - start)
            break;

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

            flow->space = 1;
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
    int base = flow->right - flow->left;
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

        if (!text)
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
    int charwidth = max(metrics->space, 1);

    if (element->tag == HTML_TAG_INPUT && type && (cstring_match(type, "checkbox") || cstring_match(type, "radio")))
        return metrics->glyphheight;

    if (element->tag == HTML_TAG_INPUT && !(type && (cstring_match(type, "submit") || cstring_match(type, "button") || cstring_match(type, "reset") || cstring_match(type, "image"))))
        return charwidth * ((size) ? (int)cstring_read_value(size, cstring_length(size), 10) : 20);

    if (element->tag == HTML_TAG_SELECT)
        return labelwidth + charwidth * 2;

    if (cstring_match(html_getstring(document, element->name), "textarea"))
    {

        char *cols = html_getattribute(document, node, "cols");

        return charwidth * ((cols) ? (int)cstring_read_value(cols, cstring_length(cols), 10) : 20);

    }

    return max(labelwidth, 16);

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
    unsigned int haswidth = style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT;
    unsigned int hasheight = style->height.unit == CSS_UNIT_PX;
    int width;
    int natural;
    int height = (hasheight) ? style->height.value : metrics.ascent + metrics.descent;
    char *rows = html_getattribute(layout->document, node, "rows");

    getlabel(layout, node, &label);

    labelwidth = measuretext(layout, &metrics, label.text, label.length, 1);
    width = (haswidth) ? resolve(&style->width, availw) : replacedwidth(layout, node, &metrics, labelwidth);

    if (style->height.unit != CSS_UNIT_PX && rows && cstring_match(html_getstring(layout->document, getnode(layout, node)->name), "textarea"))
        height = (metrics.ascent + metrics.descent) * (int)cstring_read_value(rows, cstring_length(rows), 10);

    if (picture)
    {

        if (!haswidth)
            width = (hasheight) ? height * (int)picture->width / (int)picture->height : (int)picture->width;

        if (!hasheight)
            height = (haswidth) ? width * (int)picture->height / (int)picture->width : (int)picture->height;

        label.length = 0;

    }

    if (style->borderbox)
    {

        width = max(width - bl - br, 0);
        height = max(height - bt - bb, 0);

    }

    natural = width;

    if (style->maxwidth.unit == CSS_UNIT_PX || style->maxwidth.unit == CSS_UNIT_PERCENT)
        width = min(width, resolve(&style->maxwidth, availw) - ((style->borderbox) ? bl + br : 0));

    width = min(width, max(availw - bl - br - resolve(&style->margin[CSS_LEFT], availw) - resolve(&style->margin[CSS_RIGHT], availw), 0));

    if (picture && !hasheight && width != natural && natural > 0)
        height = height * width / natural;

    box->placed = 1;
    box->x = x + resolve(&style->margin[CSS_LEFT], availw);
    box->y = top;
    box->w = bl + width + br;
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

    return element->tag == HTML_TAG_IMG || element->tag == HTML_TAG_INPUT || element->tag == HTML_TAG_SELECT || element->tag == HTML_TAG_TEXTAREA;

}

static int measure(struct layout *layout, unsigned int node, int availw)
{

    struct css_style *style = getstyle(layout, node);
    unsigned int fragmentcount = layout->fragmentcount;
    unsigned int textcount = layout->textcount;
    unsigned int marker = layout->marker;
    int extent = layout->extent;
    int ml = resolve(&style->margin[CSS_LEFT], availw);
    int needed;

    if (layout->measuring >= MEASUREDEPTH)
        return max(availw - ml - resolve(&style->margin[CSS_RIGHT], availw), 0);

    layout->measuring++;
    layout->extent = 0;

    if (isreplaced(layout, node))
        layoutreplaced(layout, node, 0, 0, availw);
    else
        layoutblock(layout, node, 0, 0, availw);

    needed = layout->extent - ml + style->borderwidth[CSS_RIGHT] + resolve(&style->padding[CSS_RIGHT], availw);

    if (style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT || isreplaced(layout, node))
        needed = layout->boxes[node].w;

    needed = min(needed, layout->boxes[node].w);

    layout->measuring--;
    layout->fragmentcount = fragmentcount;
    layout->textcount = textcount;
    layout->marker = marker;
    layout->extent = extent;

    return max(needed, 0);

}

static void placeatomic(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    struct css_style *container = getstyle(layout, flow->node);
    int availw = flow->right - flow->left;
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

    if (wrap && !isempty(layout, flow) && flow->x + spacewidth + gap + ml + width + mr > flow->right)
    {

        flushline(layout, flow, 0);

        spacewidth = 0;
        gap = 0;

    }

    startline(layout, flow);

    flow->x += spacewidth + gap;
    fragmentstart = layout->fragmentcount;
    layout->atomicdepth++;

    if (isreplaced(layout, node))
        height = layoutreplaced(layout, node, flow->x, flow->y + mt, ml + width + mr);
    else
        height = layoutblock(layout, node, flow->x, flow->y + mt, ml + width + mr);

    layout->atomicdepth--;

    additem(layout, node, 1, fragmentstart, layout->fragmentcount, mt + height + mb, 0, 0);

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
    int availw = flow->right - flow->left;
    int top;

    flushline(layout, flow, 0);

    top = flow->y + collapse(flow->pending, resolve(&style->margin[CSS_TOP], availw));
    flow->y = top + layoutblock(layout, node, flow->left, top, availw);
    flow->pending = resolve(&style->margin[CSS_BOTTOM], availw);

}

static unsigned int isvisible(struct css_style *style)
{

    return style->display != CSS_DISPLAY_NONE && style->position != CSS_POSITION_ABSOLUTE && style->position != CSS_POSITION_FIXED;

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

    if (!isvisible(style))
        return;

    if (element->tag == HTML_TAG_BR)
    {

        flushline(layout, flow, 1);

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

    flow->left = left;
    flow->right = right;
    flow->x = left;
    flow->y = top;
    flow->pending = 0;
    flow->itemstart = layout->itemcount;
    flow->space = 0;
    flow->node = node;
    flow->type = FLOW_NORMAL;

    if (style->display == CSS_DISPLAY_FLEX)
        flow->type = (style->column) ? FLOW_FLEXCOLUMN : FLOW_FLEXROW;

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

static void measurerow(struct layout *layout, unsigned int row, struct columns *columns, int maxwidths[COLUMNS], int fixed[COLUMNS], int availw)
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

            if (style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT)
                fixed[column] = max(fixed[column], resolve(&style->width, availw) + ((style->borderbox) ? 0 : style->borderwidth[CSS_LEFT] + style->borderwidth[CSS_RIGHT] + resolve(&style->padding[CSS_LEFT], availw) + resolve(&style->padding[CSS_RIGHT], availw)));

        }

        column += span;

    }

    columns->count = (column > columns->count) ? column : columns->count;

}

static int computecolumns(struct layout *layout, unsigned int node, struct columns *columns, int availw, unsigned int stretch)
{

    int maxwidths[COLUMNS];
    int fixed[COLUMNS];
    int total = 0;
    int fixedtotal = 0;
    int space;
    unsigned int child;
    unsigned int i;

    buffer_clear(maxwidths, sizeof (int) * COLUMNS);
    buffer_clear(fixed, sizeof (int) * COLUMNS);

    columns->count = 0;

    if (getstyle(layout, node)->display == CSS_DISPLAY_ROW)
    {

        measurerow(layout, node, columns, maxwidths, fixed, availw);

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
                measurerow(layout, child, columns, maxwidths, fixed, availw);

            if (display == CSS_DISPLAY_ROWGROUP)
            {

                for (row = getnode(layout, child)->first; row; row = getnode(layout, row)->next)
                {

                    if (iscell(layout, row) && getstyle(layout, row)->display == CSS_DISPLAY_ROW)
                        measurerow(layout, row, columns, maxwidths, fixed, availw);

                }

            }

        }

    }

    if (!columns->count)
        return 0;

    space = max(availw - SPACING * ((int)columns->count + 1), 0);

    for (i = 0; i < columns->count; i++)
    {

        if (fixed[i])
            fixedtotal += fixed[i];
        else
            total += maxwidths[i];

    }

    for (i = 0; i < columns->count; i++)
    {

        if (fixed[i])
            columns->widths[i] = (fixedtotal > space) ? fixed[i] * space / fixedtotal : fixed[i];
        else if (total > space - fixedtotal)
            columns->widths[i] = (total) ? maxwidths[i] * max(space - fixedtotal, 0) / total : 0;
        else
            columns->widths[i] = maxwidths[i];

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

    total = SPACING * ((int)columns->count + 1);

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
    int x = flow->left + SPACING;
    int rowheight = 0;
    unsigned int cell;
    unsigned int i;

    flushline(layout, flow, 0);

    flow->y += max(flow->pending, 0);
    flow->pending = 0;
    box->placed = 1;
    box->x = flow->left;
    box->y = flow->y + SPACING;
    box->w = flow->right - flow->left;

    for (cell = getnode(layout, row)->first; cell && column < columns->count && count < COLUMNS; cell = getnode(layout, cell)->next)
    {

        unsigned int span;
        int width = 0;

        if (!iscell(layout, cell))
            continue;

        span = getspan(layout, cell, columns->count - column);

        for (i = column; i < column + span; i++)
            width += columns->widths[i];

        width += SPACING * ((int)span - 1);
        cells[count].node = cell;
        cells[count].fragmentstart = layout->fragmentcount;
        cells[count].height = layoutblock(layout, cell, x, box->y, width);
        cells[count].fragmentend = layout->fragmentcount;
        rowheight = max(rowheight, cells[count].height);
        x += width + SPACING;
        column += span;
        count++;

    }

    for (i = 0; i < count; i++)
    {

        struct layout_box *cellbox = &layout->boxes[cells[i].node];

        translate(layout, cells[i].node, cells[i].fragmentstart, cells[i].fragmentend, 0, (rowheight - cells[i].height) / 2, 0);

        cellbox->h = rowheight;

    }

    box->h = rowheight;
    flow->y = box->y + rowheight;

}

static void layouttable(struct layout *layout, struct flow *flow, unsigned int table, struct columns *columns)
{

    unsigned int child;

    for (child = getnode(layout, table)->first; child; child = getnode(layout, child)->next)
    {

        unsigned int display;

        if (!iscell(layout, child))
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

    flushline(layout, flow, 0);

    flow->y += SPACING;

}

static void layoutgrid(struct layout *layout, struct flow *flow, unsigned int node)
{

    struct css_style *style = getstyle(layout, node);
    int width = flow->right - flow->left;
    int gap = style->columngap;
    unsigned int columns = style->columns;
    unsigned int column = 0;
    unsigned int rowstart = 0;
    int rowheight = 0;
    int y = flow->y;
    int columnwidth;
    unsigned int child;

    if (columns == CSS_COLUMNS_AUTOFILL)
        columns = (style->trackmin + gap > 0) ? (unsigned int)((width + gap) / (style->trackmin + gap)) : 1;

    if (columns < 1)
        columns = 1;

    columnwidth = max((width - gap * ((int)columns - 1)) / (int)columns, 0);

    for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
    {

        struct css_style *childstyle;
        int mt;
        int mb;
        int height;

        if (getnode(layout, child)->type != HTML_NODE_ELEMENT || !isvisible(getstyle(layout, child)))
            continue;

        childstyle = getstyle(layout, child);
        mt = resolve(&childstyle->margin[CSS_TOP], columnwidth);
        mb = resolve(&childstyle->margin[CSS_BOTTOM], columnwidth);

        if (!column)
            rowstart = child;

        height = layoutblock(layout, child, flow->left + (int)column * (columnwidth + gap), y + mt, columnwidth);
        rowheight = max(rowheight, mt + height + mb);
        column++;

        if (column == columns || !getnode(layout, child)->next)
        {

            unsigned int item;

            for (item = rowstart; item; item = getnode(layout, item)->next)
            {

                struct css_style *itemstyle = getstyle(layout, item);

                if (getnode(layout, item)->type == HTML_NODE_ELEMENT && layout->boxes[item].placed && itemstyle->height.unit == CSS_UNIT_AUTO)
                    layout->boxes[item].h = rowheight - resolve(&itemstyle->margin[CSS_TOP], columnwidth) - resolve(&itemstyle->margin[CSS_BOTTOM], columnwidth);

                if (item == child)
                    break;

            }

            y += rowheight + style->rowgap;
            rowheight = 0;
            column = 0;

        }

    }

    if (column)
        y += rowheight + style->rowgap;

    flow->y = (y > flow->y) ? y - style->rowgap : y;

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
    struct columns columns;
    struct flow flow;
    int contentheight;
    unsigned int child;

    if (style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT)
    {

        cw = resolve(&style->width, availw) - ((style->borderbox) ? extra : 0);
        constrained = 1;

    }

    if (style->maxwidth.unit == CSS_UNIT_PX || style->maxwidth.unit == CSS_UNIT_PERCENT)
    {

        int limit = resolve(&style->maxwidth, availw) - ((style->borderbox) ? extra : 0);

        if (cw > limit)
        {

            cw = limit;
            constrained = 1;

        }

    }

    if (style->display == CSS_DISPLAY_TABLE || style->display == CSS_DISPLAY_ROW)
    {

        int width = computecolumns(layout, node, &columns, cw, style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT);

        if (!(style->width.unit == CSS_UNIT_PX || style->width.unit == CSS_UNIT_PERCENT) && width < cw)
        {

            cw = width;
            constrained = 1;

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

    initflow(layout, &flow, node, box->x + bl + pl, box->x + bl + pl + cw, top + bt + pt);

    if (style->display == CSS_DISPLAY_LISTITEM && style->liststyle != CSS_LIST_NONE)
        setmarker(layout, node, flow.left, flow.y);

    if (style->display == CSS_DISPLAY_TABLE)
    {

        layouttable(layout, &flow, node, &columns);

    }

    else if (style->display == CSS_DISPLAY_ROW)
    {

        layoutrow(layout, &flow, node, &columns);

    }

    else if (style->display == CSS_DISPLAY_GRID)
    {

        layoutgrid(layout, &flow, node);

    }

    else
    {

        for (child = getnode(layout, node)->first; child; child = getnode(layout, child)->next)
            layoutnode(layout, &flow, child);

        flushline(layout, &flow, 0);

    }

    if (layout->marker && layout->fragments[layout->marker - 1].node == node)
        placemarker(layout, top + bt + pt + layout->fragments[layout->marker - 1].ascent);

    contentheight = flow.y + max(flow.pending, 0) - (top + bt + pt);

    if (style->height.unit == CSS_UNIT_PX)
        contentheight = max(style->height.value - ((style->borderbox) ? bt + bb + pt + pb : 0), 0);

    if (style->minheight.unit == CSS_UNIT_PX)
        contentheight = max(contentheight, style->minheight.value - ((style->borderbox) ? bt + bb + pt + pb : 0));

    box->w = bl + pl + cw + pr + br;
    box->h = bt + pt + contentheight + pb + bb;

    if (constrained)
        layout->extent = max(layout->extent, box->x + box->w);

    return box->h;

}

void layout_document(struct layout *layout, unsigned int width)
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
    layout->extent = 0;
    layout->height = 0;
    layout->full = 0;

    for (node = 0; node < document->nodecount; node++)
        layout->boxes[node].placed = 0;

    if (document->html && isvisible(getstyle(layout, document->html)))
        layout->height = layoutblock(layout, document->html, 0, 0, width);

    for (i = 0; i < layout->fragmentcount; i++)
        layout->height = max(layout->height, layout->fragments[i].y + layout->fragments[i].h);

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

unsigned int layout_findnode(struct layout *layout, int x, int y)
{

    unsigned int found = 0;
    unsigned int node;
    unsigned int i;

    for (i = 0; i < layout->fragmentcount; i++)
    {

        struct layout_fragment *fragment = &layout->fragments[i];
        int slop = fragment->h / 4;

        if (x >= fragment->x && x < fragment->x + fragment->w && y >= fragment->y - slop && y < fragment->y + fragment->h + slop)
            return (fragment->type == LAYOUT_FRAGMENT_TEXT) ? getnode(layout, fragment->node)->parent : fragment->node;

    }

    for (node = 1; node < layout->document->nodecount; node++)
    {

        struct layout_box *box = &layout->boxes[node];

        if (box->placed && x >= box->x && x < box->x + box->w && y >= box->y && y < box->y + box->h)
            found = node;

    }

    return found;

}

unsigned int layout_findlink(struct layout *layout, int x, int y)
{

    unsigned int node;
    unsigned int i;

    for (i = 0; i < layout->fragmentcount; i++)
    {

        struct layout_fragment *fragment = &layout->fragments[i];
        int slop = fragment->h / 4;

        if (x >= fragment->x && x < fragment->x + fragment->w && y >= fragment->y - slop && y < fragment->y + fragment->h + slop)
        {

            unsigned int anchor = findanchor(layout, fragment->node);

            if (anchor)
                return anchor;

        }

    }

    for (node = 1; node < layout->document->nodecount; node++)
    {

        struct layout_box *box = &layout->boxes[node];

        if (box->placed && x >= box->x && x < box->x + box->w && y >= box->y && y < box->y + box->h)
        {

            unsigned int anchor = findanchor(layout, node);

            if (anchor)
                return anchor;

        }

    }

    return 0;

}
