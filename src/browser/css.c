#include <fudge.h>
#include "html.h"
#include "css.h"

#define PROPERTIES                      55
#define PROP_DISPLAY                    1
#define PROP_COLOR                      2
#define PROP_BACKGROUND                 3
#define PROP_BACKGROUNDCOLOR            4
#define PROP_FONTSIZE                   5
#define PROP_FONTWEIGHT                 6
#define PROP_FONTSTYLE                  7
#define PROP_FONT                       8
#define PROP_LINEHEIGHT                 9
#define PROP_TEXTALIGN                  10
#define PROP_TEXTDECORATION             11
#define PROP_TEXTDECORATIONLINE         12
#define PROP_WHITESPACE                 13
#define PROP_LISTSTYLE                  14
#define PROP_LISTSTYLETYPE              15
#define PROP_VISIBILITY                 16
#define PROP_MARGIN                     17
#define PROP_MARGINTOP                  18
#define PROP_PADDING                    22
#define PROP_PADDINGTOP                 23
#define PROP_BORDER                     27
#define PROP_BORDERTOP                  28
#define PROP_BORDERWIDTH                32
#define PROP_BORDERSTYLE                33
#define PROP_BORDERCOLOR                34
#define PROP_BORDERTOPWIDTH             35
#define PROP_BORDERTOPSTYLE             39
#define PROP_BORDERTOPCOLOR             43
#define PROP_WIDTH                      47
#define PROP_HEIGHT                     48
#define PROP_MINWIDTH                   49
#define PROP_MAXWIDTH                   50
#define PROP_MINHEIGHT                  51
#define PROP_BOXSIZING                  52
#define PROP_POSITION                   53
#define PROP_FLEXDIRECTION              54
#define PROP_CUSTOM                     255
#define PART_TAG                        1
#define PART_UNIVERSAL                  2
#define PART_ID                         3
#define PART_CLASS                      4
#define PART_ATTRIBUTE                  5
#define PART_FIRSTCHILD                 6
#define PART_LASTCHILD                  7
#define PART_LINK                       8
#define PART_ROOT                       9
#define PART_NEVER                      10
#define PART_NTHCHILD                   11
#define PART_CHECKED                    12
#define PART_DISABLED                   13
#define PART_EMPTY                      14
#define PART_FIRSTOFTYPE                15
#define PART_LASTOFTYPE                 16
#define COMBINATOR_NONE                 0
#define COMBINATOR_FIRST                1
#define COMBINATOR_DESCENDANT           2
#define COMBINATOR_CHILD                3
#define COMBINATOR_ADJACENT             4
#define COMBINATOR_SIBLING              5
#define OP_EXISTS                       0
#define OP_EQUAL                        1
#define OP_INCLUDES                     2
#define OP_DASH                         3
#define OP_PREFIX                       4
#define OP_SUFFIX                       5
#define OP_SUBSTRING                    6
#define KEY_NONE                        0
#define KEY_TAG                         1
#define KEY_ID                          2
#define KEY_CLASS                       3
#define MATCHES                         128
#define COLORS                          40
#define CURRENTCOLOR                    0x00000001
#define NOLIMIT                         0x7FFFFFFF
#define VALUESIZE                       64
#define FONTTOKENS                      8
#define FONTSIZES                       8
#define NAMESIZE                        64
#define EXPANDSIZE                      512
#define EXPANDDEPTH                     4
#define PHASE_CUSTOM                    0
#define PHASE_FONT                      1
#define PHASE_OTHER                     2
#define PHASES                          3

struct parser
{

    struct css_sheet *sheet;
    char *text;
    unsigned int offset;
    unsigned int origin;
    int minwidth;
    int maxwidth;

};

struct selectorparser
{

    struct css_sheet *sheet;
    char *text;
    unsigned int specificity;

};

struct span
{

    unsigned int start;
    unsigned int length;

};

struct number
{

    int milli;
    unsigned int length;

};

struct lengthvalue
{

    struct css_length length;
    unsigned int valid;

};

struct colorvalue
{

    unsigned int value;
    unsigned int valid;

};

struct matcher
{

    struct css_sheet *sheet;
    struct html_document *document;

};

struct context
{

    struct css_sheet *sheet;
    struct html_document *document;
    unsigned int node;
    struct css_style *style;
    struct css_style *parent;
    unsigned int rootsize;
    unsigned int width;
    unsigned int height;

};

struct expansion
{

    char text[EXPANDSIZE];
    unsigned int length;
    unsigned int valid;

};

struct match
{

    unsigned int origin;
    unsigned int specificity;
    unsigned int rule;

};

struct namedcolor
{

    char *name;
    unsigned int value;

};

static char *properties[PROPERTIES] = {
    "", "display", "color", "background", "background-color", "font-size", "font-weight", "font-style", "font", "line-height",
    "text-align", "text-decoration", "text-decoration-line", "white-space", "list-style", "list-style-type", "visibility", "margin", "margin-top", "margin-right",
    "margin-bottom", "margin-left", "padding", "padding-top", "padding-right", "padding-bottom", "padding-left", "border", "border-top", "border-right",
    "border-bottom", "border-left", "border-width", "border-style", "border-color", "border-top-width", "border-right-width", "border-bottom-width", "border-left-width", "border-top-style",
    "border-right-style", "border-bottom-style", "border-left-style", "border-top-color", "border-right-color", "border-bottom-color", "border-left-color", "width", "height", "min-width",
    "max-width", "min-height", "box-sizing", "position", "flex-direction"
};
static struct namedcolor colors[COLORS] = {
    {"black", 0x000000}, {"white", 0xFFFFFF}, {"red", 0xFF0000}, {"green", 0x008000}, {"blue", 0x0000FF}, {"yellow", 0xFFFF00}, {"gray", 0x808080}, {"grey", 0x808080},
    {"silver", 0xC0C0C0}, {"maroon", 0x800000}, {"purple", 0x800080}, {"fuchsia", 0xFF00FF}, {"magenta", 0xFF00FF}, {"lime", 0x00FF00}, {"olive", 0x808000}, {"navy", 0x000080},
    {"teal", 0x008080}, {"aqua", 0x00FFFF}, {"cyan", 0x00FFFF}, {"orange", 0xFFA500}, {"pink", 0xFFC0CB}, {"brown", 0xA52A2A}, {"gold", 0xFFD700}, {"darkgray", 0xA9A9A9},
    {"darkgrey", 0xA9A9A9}, {"lightgray", 0xD3D3D3}, {"lightgrey", 0xD3D3D3}, {"darkblue", 0x00008B}, {"darkred", 0x8B0000}, {"darkgreen", 0x006400}, {"lightblue", 0xADD8E6}, {"lightyellow", 0xFFFFE0},
    {"whitesmoke", 0xF5F5F5}, {"gainsboro", 0xDCDCDC}, {"dimgray", 0x696969}, {"orangered", 0xFF4500}, {"steelblue", 0x4682B4}, {"indigo", 0x4B0082}, {"violet", 0xEE82EE}, {"crimson", 0xDC143C}
};
static char *agent =
    "html,address,blockquote,body,center,dd,details,dialog,div,dl,dt,fieldset,figcaption,figure,footer,form,h1,h2,h3,h4,h5,h6,header,hgroup,hr,legend,main,menu,nav,ol,p,pre,search,section,summary,ul,article,aside,caption,listing,plaintext,xmp,optgroup{display:block}"
    "head,script,style,title,meta,link,base,template,option,datalist,param,area,map,[hidden],input[type=hidden]{display:none}"
    "li{display:list-item}"
    "table{display:table;border-spacing:2px}"
    "tr{display:table-row}"
    "thead,tbody,tfoot{display:table-row-group}"
    "td,th{display:table-cell;padding:1px}"
    "th{font-weight:bold;text-align:center}"
    "caption{text-align:center}"
    "body{margin:8px}"
    "h1{font-size:2em;margin:.67em 0;font-weight:bold}"
    "h2{font-size:1.5em;margin:.83em 0;font-weight:bold}"
    "h3{font-size:1.17em;margin:1em 0;font-weight:bold}"
    "h4{margin:1.33em 0;font-weight:bold}"
    "h5{font-size:.83em;margin:1.67em 0;font-weight:bold}"
    "h6{font-size:.67em;margin:2.33em 0;font-weight:bold}"
    "p,dl,figure,pre,listing,xmp,plaintext,menu,fieldset{margin:1em 0}"
    "blockquote,figure{margin:1em 40px}"
    "ul,ol,menu{margin:1em 0;padding-left:40px}"
    "ul{list-style-type:disc}"
    "ol{list-style-type:decimal}"
    "ul ul,ol ul{list-style-type:circle;margin:0}"
    "ol ol,ul ol{margin:0}"
    "dd{margin-left:40px}"
    "pre,listing,xmp,plaintext,textarea{white-space:pre}"
    "b,strong{font-weight:bold}"
    "i,em,cite,var,dfn,address{font-style:italic}"
    "u,ins{text-decoration:underline}"
    "s,strike,del{text-decoration:line-through}"
    "a:link{color:#0000ee;text-decoration:underline}"
    "center{text-align:center}"
    "small,sub,sup{font-size:smaller}"
    "big{font-size:larger}"
    "mark{background-color:yellow}"
    "hr{border:1px inset gray;margin:.5em 0}"
    "fieldset{border:2px groove gray;padding:.35em .75em .625em;margin:0 2px}"
    "button{display:inline-block;padding:1px 6px;border:2px outset #c0c0c0;background-color:#e0e0e0}"
    "input,select,textarea{display:inline-block;border:2px inset #c0c0c0;padding:1px 2px}"
    "img{display:inline-block}"
    "nobr{white-space:nowrap}";

static unsigned int isspace(char c)
{

    return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f';

}

static char lower(char c)
{

    return (c >= 'A' && c <= 'Z') ? c + 32 : c;

}

static unsigned int isidentchar(char c)
{

    return cstring_isalpha(c) || cstring_isdigit(c) || c == '-' || c == '_' || (unsigned char)c >= 0x80 || c == '\\';

}

static unsigned int matchword(char *text, unsigned int length, char *word)
{

    return cstring_length(word) == length && buffer_match(text, word, length);

}

static unsigned int skipcomment(char *text, unsigned int offset, unsigned int end)
{

    if (offset + 1 < end && text[offset] == '/' && text[offset + 1] == '*')
    {

        offset += 2;

        while (offset + 1 < end && !(text[offset] == '*' && text[offset + 1] == '/'))
            offset++;

        return (offset + 2 <= end) ? offset + 2 : end;

    }

    return offset;

}

static unsigned int skipspace(char *text, unsigned int offset, unsigned int end)
{

    while (offset < end)
    {

        unsigned int next = skipcomment(text, offset, end);

        if (next != offset)
            offset = next;
        else if (isspace(text[offset]))
            offset++;
        else
            break;

    }

    return offset;

}

static unsigned int skipstring(char *text, unsigned int offset, unsigned int end)
{

    char quote = text[offset++];

    while (offset < end && text[offset] != quote)
    {

        if (text[offset] == '\\')
            offset++;

        offset++;

    }

    return (offset < end) ? offset + 1 : end;

}

static unsigned int findclose(char *text, unsigned int offset, unsigned int end)
{

    unsigned int depth = 0;

    while (offset < end)
    {

        unsigned int next = skipcomment(text, offset, end);
        char c = text[offset];

        if (next != offset)
        {

            offset = next;

            continue;

        }

        if (c == '"' || c == '\'')
        {

            offset = skipstring(text, offset, end);

            continue;

        }

        if (c == '{' || c == '(' || c == '[')
            depth++;

        if (c == '}' || c == ')' || c == ']')
        {

            if (depth <= 1)
                return offset;

            depth--;

        }

        offset++;

    }

    return end;

}

static unsigned int findstop(char *text, unsigned int offset, unsigned int end, char *stops)
{

    while (offset < end)
    {

        unsigned int next = skipcomment(text, offset, end);
        char c = text[offset];

        if (next != offset)
            offset = next;
        else if (buffer_findbyte(stops, cstring_length(stops), c) < cstring_length(stops))
            return offset;
        else if (c == '"' || c == '\'')
            offset = skipstring(text, offset, end);
        else if (c == '(' || c == '[' || c == '{')
            offset = findclose(text, offset, end) + 1;
        else
            offset++;

    }

    return end;

}

static unsigned int pooladd(struct css_sheet *sheet, char *text, unsigned int length, unsigned int value)
{

    unsigned int offset = sheet->poolcount;
    unsigned int space = 0;
    unsigned int i = 0;

    while (i < length)
    {

        unsigned int next = skipcomment(text, i, length);
        char c;

        if (next != i)
        {

            i = next;

            continue;

        }

        c = (value) ? lower(text[i]) : text[i];

        if (c == '\\' && i + 1 < length)
            c = text[++i];

        i++;

        if (value && isspace(c))
        {

            space = 1;

            continue;

        }

        if (sheet->poolcount + 3 > CSS_POOLSIZE)
        {

            sheet->full = 1;

            return 0;

        }

        if (space && sheet->poolcount > offset)
            sheet->pool[sheet->poolcount++] = ' ';

        space = 0;
        sheet->pool[sheet->poolcount++] = c;

    }

    if (sheet->poolcount + 1 > CSS_POOLSIZE)
    {

        sheet->full = 1;

        return 0;

    }

    sheet->pool[sheet->poolcount++] = '\0';

    return offset;

}

static char *poolstring(struct css_sheet *sheet, unsigned int offset)
{

    return sheet->pool + offset;

}

static unsigned int findproperty(char *text, unsigned int length)
{

    unsigned int i;

    for (i = 1; i < PROPERTIES; i++)
    {

        if (matchword(text, length, properties[i]))
            return i;

    }

    return 0;

}

static unsigned int contains(char *text, unsigned int length, char *word)
{

    unsigned int wlength = cstring_length(word);
    unsigned int i;

    for (i = 0; i + wlength <= length; i++)
    {

        if (buffer_match(text + i, word, wlength))
            return 1;

    }

    return 0;

}

static void parsedeclaration(struct css_sheet *sheet, char *text, unsigned int start, unsigned int end)
{

    unsigned int colon = findstop(text, start, end, ":{");
    char name[NAMESIZE];
    unsigned int namelength = 0;
    unsigned int property;
    unsigned int important = 0;
    unsigned int i;

    if (colon >= end || text[colon] != ':')
        return;

    start = skipspace(text, start, colon);

    for (i = start; i < colon && !isspace(text[i]) && namelength < NAMESIZE - 1; i++)
        name[namelength++] = lower(text[i]);

    property = (namelength > 2 && name[0] == '-' && name[1] == '-') ? PROP_CUSTOM : findproperty(name, namelength);

    if (!property)
        return;

    start = skipspace(text, colon + 1, end);

    while (end > start && isspace(text[end - 1]))
        end--;

    for (i = end; i > start; i--)
    {

        if (text[i - 1] == '!')
        {

            if (contains(text + i, end - i, "important") || contains(text + i, end - i, "IMPORTANT"))
            {

                important = 1;
                end = i - 1;

                while (end > start && isspace(text[end - 1]))
                    end--;

            }

            break;

        }

    }

    if ((end <= start && property != PROP_CUSTOM) || findstop(text, start, end, "{") < end)
        return;

    if (sheet->declarationcount >= CSS_DECLARATIONS)
    {

        sheet->full = 1;

        return;

    }

    sheet->declarations[sheet->declarationcount].property = property;
    sheet->declarations[sheet->declarationcount].important = important;
    sheet->declarations[sheet->declarationcount].name = (property == PROP_CUSTOM) ? pooladd(sheet, name, namelength, 1) : 0;
    sheet->declarations[sheet->declarationcount].value = pooladd(sheet, text + start, (end > start) ? end - start : 0, 1);

    if (sheet->declarations[sheet->declarationcount].value && (property != PROP_CUSTOM || sheet->declarations[sheet->declarationcount].name))
        sheet->declarationcount++;

}

static void parsedeclarations(struct css_sheet *sheet, char *text, unsigned int start, unsigned int end)
{

    while (start < end)
    {

        unsigned int stop = findstop(text, start, end, ";");

        parsedeclaration(sheet, text, start, stop);

        start = stop + 1;

    }

}

static unsigned int readident(char *text, unsigned int offset, unsigned int end)
{

    while (offset < end && isidentchar(text[offset]))
    {

        if (text[offset] == '\\' && offset + 1 < end)
            offset++;

        offset++;

    }

    return offset;

}

static struct css_part *addpart(struct css_sheet *sheet, unsigned int type, unsigned int combinator)
{

    struct css_part *part;

    if (sheet->partcount >= CSS_PARTS)
    {

        sheet->full = 1;

        return 0;

    }

    part = &sheet->parts[sheet->partcount++];

    buffer_clear(part, sizeof (struct css_part));

    part->type = type;
    part->combinator = combinator;

    return part;

}

static unsigned int parsenth(char *text, unsigned int start, unsigned int end, struct css_part *part)
{

    int a = 0;
    int b = 0;
    int sign = 1;
    unsigned int digits = 0;
    int value = 0;

    start = skipspace(text, start, end);

    while (end > start && isspace(text[end - 1]))
        end--;

    if (matchword(text + start, end - start, "odd"))
    {

        part->name = 2;
        part->value = 1;

        return 1;

    }

    if (matchword(text + start, end - start, "even"))
    {

        part->name = 2;
        part->value = 0;

        return 1;

    }

    if (start < end && (text[start] == '-' || text[start] == '+'))
        sign = (text[start++] == '-') ? -1 : 1;

    while (start < end && cstring_isdigit(text[start]))
    {

        value = value * 10 + (text[start++] - '0');
        digits++;

    }

    if (start < end && lower(text[start]) == 'n')
    {

        a = (digits) ? sign * value : sign;
        start = skipspace(text, start + 1, end);
        sign = 1;
        value = 0;
        digits = 0;

        if (start < end && (text[start] == '-' || text[start] == '+'))
        {

            sign = (text[start] == '-') ? -1 : 1;
            start = skipspace(text, start + 1, end);

        }

        while (start < end && cstring_isdigit(text[start]))
        {

            value = value * 10 + (text[start++] - '0');
            digits++;

        }

        b = sign * value;

    }

    else if (digits)
    {

        b = sign * value;

    }

    else
    {

        return 0;

    }

    if (start != end)
        return 0;

    part->name = (unsigned int)a;
    part->value = (unsigned int)b;

    return 1;

}

static unsigned int parsesimple(struct selectorparser *selectorparser, unsigned int offset, unsigned int end, unsigned int combinator);

static unsigned int parsepseudo(struct selectorparser *selectorparser, unsigned int offset, unsigned int end, unsigned int combinator)
{

    struct css_sheet *sheet = selectorparser->sheet;
    char *text = selectorparser->text;
    unsigned int start = offset;
    unsigned int stop = readident(text, start, end);
    unsigned int length = stop - start;
    char name[24];
    struct css_part *part;
    unsigned int i;

    if (!length || length > 23)
        return 0;

    for (i = 0; i < length; i++)
        name[i] = lower(text[start + i]);

    name[length] = '\0';

    if (stop < end && text[stop] == '(')
    {

        unsigned int close = findclose(text, stop, end);

        if (close >= end)
            return 0;

        if (cstring_match(name, "not"))
        {

            unsigned int inner = skipspace(text, stop + 1, close);
            unsigned int first = sheet->partcount;
            unsigned int next = parsesimple(selectorparser, inner, close, combinator);

            if (!next || skipspace(text, next, close) != close || sheet->partcount != first + 1)
                return 0;

            sheet->parts[first].negate = 1;

            return close + 1;

        }

        if (cstring_match(name, "nth-child"))
        {

            part = addpart(sheet, PART_NTHCHILD, combinator);

            if (!part || !parsenth(text, stop + 1, close, part))
                return 0;

            selectorparser->specificity += 0x100;

            return close + 1;

        }

        return 0;

    }

    if (cstring_match(name, "first-child"))
        part = addpart(sheet, PART_FIRSTCHILD, combinator);
    else if (cstring_match(name, "last-child"))
        part = addpart(sheet, PART_LASTCHILD, combinator);
    else if (cstring_match(name, "first-of-type"))
        part = addpart(sheet, PART_FIRSTOFTYPE, combinator);
    else if (cstring_match(name, "last-of-type"))
        part = addpart(sheet, PART_LASTOFTYPE, combinator);
    else if (cstring_match(name, "link") || cstring_match(name, "any-link"))
        part = addpart(sheet, PART_LINK, combinator);
    else if (cstring_match(name, "root"))
        part = addpart(sheet, PART_ROOT, combinator);
    else if (cstring_match(name, "checked"))
        part = addpart(sheet, PART_CHECKED, combinator);
    else if (cstring_match(name, "disabled"))
        part = addpart(sheet, PART_DISABLED, combinator);
    else if (cstring_match(name, "empty"))
        part = addpart(sheet, PART_EMPTY, combinator);
    else if (cstring_match(name, "hover") || cstring_match(name, "focus") || cstring_match(name, "active") || cstring_match(name, "visited") || cstring_match(name, "focus-within") || cstring_match(name, "focus-visible") || cstring_match(name, "target"))
        part = addpart(sheet, PART_NEVER, combinator);
    else
        return 0;

    if (!part)
        return 0;

    selectorparser->specificity += 0x100;

    return stop;

}

static unsigned int parseattribute(struct selectorparser *selectorparser, unsigned int offset, unsigned int end, unsigned int combinator)
{

    struct css_sheet *sheet = selectorparser->sheet;
    char *text = selectorparser->text;
    unsigned int close = findclose(text, offset, end);
    unsigned int start = skipspace(text, offset + 1, close);
    unsigned int stop = readident(text, start, close);
    struct css_part *part;

    if (close >= end || stop == start)
        return 0;

    part = addpart(sheet, PART_ATTRIBUTE, combinator);

    if (!part)
        return 0;

    part->name = pooladd(sheet, text + start, stop - start, 1);
    start = skipspace(text, stop, close);

    if (start < close)
    {

        char c = text[start];

        if (c == '=')
            part->op = OP_EQUAL;
        else if (c == '~')
            part->op = OP_INCLUDES;
        else if (c == '|')
            part->op = OP_DASH;
        else if (c == '^')
            part->op = OP_PREFIX;
        else if (c == '$')
            part->op = OP_SUFFIX;
        else if (c == '*')
            part->op = OP_SUBSTRING;
        else
            return 0;

        start += (c == '=') ? 1 : 2;
        start = skipspace(text, start, close);

        if (start < close && (text[start] == '"' || text[start] == '\''))
        {

            stop = skipstring(text, start, close);
            part->value = pooladd(sheet, text + start + 1, (stop > start + 1) ? stop - start - 2 : 0, 0);

        }

        else
        {

            stop = readident(text, start, close);
            part->value = pooladd(sheet, text + start, stop - start, 0);

        }

        if (!part->value && sheet->full)
            return 0;

    }

    return close + 1;

}

static unsigned int parsesimple(struct selectorparser *selectorparser, unsigned int offset, unsigned int end, unsigned int combinator)
{

    struct css_sheet *sheet = selectorparser->sheet;
    char *text = selectorparser->text;
    char c = text[offset];
    struct css_part *part;
    unsigned int stop;

    if (c == '*')
        return (addpart(sheet, PART_UNIVERSAL, combinator)) ? offset + 1 : 0;

    if (c == '[')
    {

        stop = parseattribute(selectorparser, offset, end, combinator);
        selectorparser->specificity += 0x100;

        return stop;

    }

    if (c == ':')
    {

        if (offset + 1 < end && text[offset + 1] == ':')
            return 0;

        return parsepseudo(selectorparser, offset + 1, end, combinator);

    }

    if (c == '#' || c == '.')
    {

        stop = readident(text, offset + 1, end);

        if (stop == offset + 1)
            return 0;

        part = addpart(sheet, (c == '#') ? PART_ID : PART_CLASS, combinator);

        if (!part)
            return 0;

        part->name = pooladd(sheet, text + offset + 1, stop - offset - 1, 0);
        selectorparser->specificity += (c == '#') ? 0x10000 : 0x100;

        return (part->name) ? stop : 0;

    }

    if (isidentchar(c))
    {

        stop = readident(text, offset, end);
        part = addpart(sheet, PART_TAG, combinator);

        if (!part)
            return 0;

        part->name = pooladd(sheet, text + offset, stop - offset, 1);
        selectorparser->specificity += 1;

        return (part->name) ? stop : 0;

    }

    return 0;

}

static void setkey(struct css_sheet *sheet, struct css_selector *selector)
{

    unsigned int start = selector->part + selector->count;
    unsigned int i;

    while (start > selector->part && sheet->parts[start - 1].combinator == COMBINATOR_NONE)
        start--;

    if (start > selector->part)
        start--;

    selector->keytype = KEY_NONE;

    for (i = start; i < selector->part + selector->count; i++)
    {

        struct css_part *part = &sheet->parts[i];

        if (part->negate)
            continue;

        if (part->type == PART_ID)
        {

            selector->keytype = KEY_ID;
            selector->key = part->name;

        }

        else if (part->type == PART_CLASS && selector->keytype != KEY_ID)
        {

            selector->keytype = KEY_CLASS;
            selector->key = part->name;

        }

        else if (part->type == PART_TAG && selector->keytype == KEY_NONE)
        {

            selector->keytype = KEY_TAG;
            selector->key = part->name;

        }

    }

}

static void parseselector(struct css_sheet *sheet, char *text, unsigned int start, unsigned int end, unsigned int rule)
{

    unsigned int partstart = sheet->partcount;
    unsigned int poolstart = sheet->poolcount;
    unsigned int combinator = COMBINATOR_FIRST;
    unsigned int offset = skipspace(text, start, end);
    struct selectorparser selectorparser;
    struct css_selector *selector;

    selectorparser.sheet = sheet;
    selectorparser.text = text;
    selectorparser.specificity = 0;

    while (end > offset && isspace(text[end - 1]))
        end--;

    if (offset >= end)
        return;

    while (offset < end)
    {

        char c = text[offset];

        if (isspace(c) || c == '>' || c == '+' || c == '~')
        {

            offset = skipspace(text, offset, end);

            if (offset < end && (text[offset] == '>' || text[offset] == '+' || text[offset] == '~'))
            {

                c = text[offset];
                combinator = (c == '>') ? COMBINATOR_CHILD : (c == '+') ? COMBINATOR_ADJACENT : COMBINATOR_SIBLING;
                offset = skipspace(text, offset + 1, end);

            }

            else
            {

                combinator = COMBINATOR_DESCENDANT;

            }

            if (sheet->partcount == partstart)
                break;

            continue;

        }

        offset = parsesimple(&selectorparser, offset, end, combinator);

        if (!offset)
            break;

        combinator = COMBINATOR_NONE;

    }

    if (!offset || offset < end || combinator != COMBINATOR_NONE || sheet->selectorcount >= CSS_SELECTORS)
    {

        sheet->partcount = partstart;
        sheet->poolcount = poolstart;

        return;

    }

    selector = &sheet->selectors[sheet->selectorcount++];
    selector->part = partstart;
    selector->count = sheet->partcount - partstart;
    selector->specificity = selectorparser.specificity;
    selector->rule = rule;

    setkey(sheet, selector);

}

static void parserule(struct parser *parser, unsigned int start, unsigned int end, unsigned int blockstart, unsigned int blockend)
{

    struct css_sheet *sheet = parser->sheet;
    unsigned int selectors = sheet->selectorcount;
    struct css_rule *rule;

    if (sheet->rulecount >= CSS_RULES)
    {

        sheet->full = 1;

        return;

    }

    while (start < end)
    {

        unsigned int stop = findstop(parser->text, start, end, ",");

        parseselector(sheet, parser->text, start, stop, sheet->rulecount);

        start = stop + 1;

    }

    if (sheet->selectorcount == selectors)
        return;

    rule = &sheet->rules[sheet->rulecount++];
    rule->declaration = sheet->declarationcount;
    rule->origin = parser->origin;
    rule->minwidth = parser->minwidth;
    rule->maxwidth = parser->maxwidth;

    parsedeclarations(sheet, parser->text, blockstart, blockend);

    rule->declarations = sheet->declarationcount - rule->declaration;

}

static struct number parsenumber(char *text, unsigned int length)
{

    struct number number;
    unsigned int i = 0;
    int sign = 1;
    int milli = 0;
    unsigned int digits = 0;
    unsigned int scale = 1000;

    number.milli = 0;
    number.length = 0;

    if (i < length && (text[i] == '-' || text[i] == '+'))
        sign = (text[i++] == '-') ? -1 : 1;

    while (i < length && cstring_isdigit(text[i]))
    {

        if (milli < 1000000000 / 10 - 1000)
            milli = milli * 10 + (text[i] - '0') * 1000;

        i++;
        digits++;

    }

    if (i + 1 < length && text[i] == '.' && cstring_isdigit(text[i + 1]))
    {

        i++;

        while (i < length && cstring_isdigit(text[i]))
        {

            scale /= 10;
            milli += (text[i] - '0') * scale;
            i++;
            digits++;

        }

    }

    if (!digits)
        return number;

    number.milli = sign * milli;
    number.length = i;

    return number;

}

static int roundmilli(int milli)
{

    return (milli >= 0) ? (milli + 500) / 1000 : -((-milli + 500) / 1000);

}

static struct lengthvalue lengthvalue(int value, unsigned int unit, unsigned int valid)
{

    struct lengthvalue result;

    result.length.value = value;
    result.length.unit = unit;
    result.valid = valid;

    return result;

}

static struct lengthvalue parselength(struct context *context, char *text, unsigned int length, unsigned int fontsize)
{

    struct number number;
    char *unit;
    unsigned int unitlength;

    if (matchword(text, length, "auto"))
        return lengthvalue(0, CSS_UNIT_AUTO, 1);

    if (matchword(text, length, "none"))
        return lengthvalue(0, CSS_UNIT_NONE, 1);

    number = parsenumber(text, length);

    if (!number.length)
        return lengthvalue(0, CSS_UNIT_PX, 0);

    unit = text + number.length;
    unitlength = length - number.length;

    if (!unitlength || matchword(unit, unitlength, "px"))
        return lengthvalue(roundmilli(number.milli), CSS_UNIT_PX, 1);

    if (matchword(unit, unitlength, "%"))
        return lengthvalue(number.milli, CSS_UNIT_PERCENT, 1);

    if (matchword(unit, unitlength, "em"))
        return lengthvalue(roundmilli(number.milli * (int)fontsize), CSS_UNIT_PX, 1);

    if (matchword(unit, unitlength, "rem"))
        return lengthvalue(roundmilli(number.milli * (int)context->rootsize), CSS_UNIT_PX, 1);

    if (matchword(unit, unitlength, "ex") || matchword(unit, unitlength, "ch"))
        return lengthvalue(roundmilli(number.milli * (int)fontsize / 2), CSS_UNIT_PX, 1);

    if (matchword(unit, unitlength, "pt"))
        return lengthvalue(roundmilli(number.milli * 4 / 3), CSS_UNIT_PX, 1);

    if (matchword(unit, unitlength, "vw"))
        return lengthvalue(roundmilli(number.milli / 100 * (int)context->width), CSS_UNIT_PX, 1);

    if (matchword(unit, unitlength, "vh"))
        return lengthvalue(roundmilli(number.milli / 100 * (int)context->height), CSS_UNIT_PX, 1);

    if (matchword(unit, unitlength, "cm"))
        return lengthvalue(roundmilli(number.milli * 38), CSS_UNIT_PX, 1);

    if (matchword(unit, unitlength, "mm"))
        return lengthvalue(roundmilli(number.milli * 38 / 10), CSS_UNIT_PX, 1);

    if (matchword(unit, unitlength, "in"))
        return lengthvalue(roundmilli(number.milli * 96), CSS_UNIT_PX, 1);

    return lengthvalue(0, CSS_UNIT_PX, 0);

}

static void copylength(struct css_length *to, struct css_length *from)
{

    to->value = from->value;
    to->unit = from->unit;

}

static struct colorvalue colorvalue(unsigned int value, unsigned int valid)
{

    struct colorvalue result;

    result.value = value;
    result.valid = valid;

    return result;

}

static unsigned int hexdigit(char c)
{

    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 16;

}

static unsigned int clampbyte(int value)
{

    return (value < 0) ? 0 : (value > 255) ? 255 : value;

}

static struct colorvalue parsefunctioncolor(char *text, unsigned int length)
{

    unsigned int channels[4];
    unsigned int count = 0;
    unsigned int i = 0;

    channels[3] = 255;

    while (i < length && text[i] != '(')
        i++;

    i++;

    while (i < length && count < 4)
    {

        struct number number;

        while (i < length && (isspace(text[i]) || text[i] == ',' || text[i] == '/'))
            i++;

        number = parsenumber(text + i, length - i);

        if (!number.length)
            break;

        i += number.length;

        if (i < length && text[i] == '%')
        {

            channels[count] = clampbyte(number.milli * 255 / 100000);
            i++;

        }

        else
        {

            channels[count] = (count == 3) ? clampbyte(number.milli * 255 / 1000) : clampbyte(roundmilli(number.milli));

        }

        count++;

    }

    if (count < 3)
        return colorvalue(0, 0);

    return colorvalue((channels[3] << 24) | (channels[0] << 16) | (channels[1] << 8) | channels[2], 1);

}

static struct colorvalue parsecolor(char *text, unsigned int length)
{

    unsigned int i;

    if (length && text[0] == '#')
    {

        unsigned int digits[8];

        if (length != 4 && length != 5 && length != 7 && length != 9)
            return colorvalue(0, 0);

        for (i = 1; i < length; i++)
        {

            digits[i - 1] = hexdigit(text[i]);

            if (digits[i - 1] > 15)
                return colorvalue(0, 0);

        }

        if (length == 4 || length == 5)
        {

            unsigned int alpha = (length == 5) ? digits[3] * 17 : 255;

            return colorvalue((alpha << 24) | (digits[0] * 17 << 16) | (digits[1] * 17 << 8) | (digits[2] * 17), 1);

        }

        return colorvalue((((length == 9) ? digits[6] * 16 + digits[7] : 255) << 24) | ((digits[0] * 16 + digits[1]) << 16) | ((digits[2] * 16 + digits[3]) << 8) | (digits[4] * 16 + digits[5]), 1);

    }

    if (length > 4 && (buffer_match(text, "rgb(", 4) || buffer_match(text, "rgba(", 5)))
        return parsefunctioncolor(text, length);

    if (matchword(text, length, "transparent"))
        return colorvalue(0, 1);

    if (matchword(text, length, "currentcolor"))
        return colorvalue(CURRENTCOLOR, 1);

    for (i = 0; i < COLORS; i++)
    {

        if (matchword(text, length, colors[i].name))
            return colorvalue(0xFF000000 | colors[i].value, 1);

    }

    return colorvalue(0, 0);

}

static struct span nexttoken(char *text, unsigned int offset)
{

    struct span span;
    unsigned int depth = 0;

    while (text[offset] == ' ' || text[offset] == ',')
        offset++;

    span.start = offset;

    while (text[offset] && (depth || (text[offset] != ' ' && text[offset] != ',')))
    {

        if (text[offset] == '(')
            depth++;

        if (text[offset] == ')' && depth)
            depth--;

        offset++;

    }

    span.length = offset - span.start;

    return span;

}

static unsigned int countspans(char *text, struct span *spans, unsigned int max)
{

    unsigned int count = 0;
    unsigned int offset = 0;

    while (count < max)
    {

        struct span span = nexttoken(text, offset);

        if (!span.length)
            break;

        spans[count++] = span;
        offset = span.start + span.length;

    }

    return count;

}

static unsigned int fontkeyword(struct context *context, char *text, unsigned int length)
{

    unsigned int parent = context->parent->fontsize;

    if (matchword(text, length, "xx-small"))
        return 9;

    if (matchword(text, length, "x-small"))
        return 10;

    if (matchword(text, length, "small"))
        return 13;

    if (matchword(text, length, "medium"))
        return 16;

    if (matchword(text, length, "large"))
        return 18;

    if (matchword(text, length, "x-large"))
        return 24;

    if (matchword(text, length, "xx-large"))
        return 32;

    if (matchword(text, length, "smaller"))
        return parent * 5 / 6;

    if (matchword(text, length, "larger"))
        return parent * 6 / 5;

    return 0;

}

static unsigned int setfontsize(struct context *context, char *text, unsigned int length)
{

    unsigned int size = fontkeyword(context, text, length);

    if (!size)
    {

        struct lengthvalue value = parselength(context, text, length, context->parent->fontsize);

        if (!value.valid || value.length.unit == CSS_UNIT_AUTO || value.length.unit == CSS_UNIT_NONE)
            return 0;

        size = (value.length.unit == CSS_UNIT_PERCENT) ? (unsigned int)roundmilli(value.length.value * (int)context->parent->fontsize / 100) : (unsigned int)value.length.value;

    }

    context->style->fontsize = (size < 6) ? 6 : (size > 200) ? 200 : size;

    return 1;

}

static unsigned int setlineheight(struct context *context, char *text, unsigned int length)
{

    struct lengthvalue value;
    struct number number;

    if (matchword(text, length, "normal"))
    {

        context->style->lineheight = 0;
        context->style->lineheightfactor = 0;

        return 1;

    }

    number = parsenumber(text, length);

    if (number.length && number.length == length && number.milli >= 0)
    {

        context->style->lineheight = 0;
        context->style->lineheightfactor = (number.milli > 10000) ? 10000 : number.milli;

        return 1;

    }

    value = parselength(context, text, length, context->style->fontsize);

    if (!value.valid || value.length.unit > CSS_UNIT_PERCENT || value.length.value < 0)
        return 0;

    context->style->lineheightfactor = 0;
    context->style->lineheight = (value.length.unit == CSS_UNIT_PERCENT) ? roundmilli(value.length.value * (int)context->style->fontsize / 100) : value.length.value;

    return 1;

}

static unsigned int setweight(struct context *context, char *text, unsigned int length)
{

    struct number number = parsenumber(text, length);

    if (matchword(text, length, "bold") || matchword(text, length, "bolder"))
        context->style->bold = 1;
    else if (matchword(text, length, "normal") || matchword(text, length, "lighter"))
        context->style->bold = 0;
    else if (number.length == length)
        context->style->bold = number.milli >= 600000;
    else
        return 0;

    return 1;

}

static void setfont(struct context *context, char *value)
{

    struct span spans[FONTTOKENS];
    unsigned int count = countspans(value, spans, FONTTOKENS);
    unsigned int size;
    unsigned int i;

    for (size = 0; size < count; size++)
    {

        char *text = value + spans[size].start;

        if (cstring_isdigit(text[0]) || text[0] == '.' || fontkeyword(context, text, buffer_findbyte(text, spans[size].length, '/')))
            break;

    }

    if (size == count)
        return;

    context->style->bold = 0;
    context->style->italic = 0;
    context->style->lineheight = 0;
    context->style->lineheightfactor = 0;

    for (i = 0; i < size; i++)
    {

        char *text = value + spans[i].start;
        unsigned int length = spans[i].length;

        if (matchword(text, length, "italic") || matchword(text, length, "oblique"))
            context->style->italic = 1;
        else
            setweight(context, text, length);

    }

    {

        char *text = value + spans[size].start;
        unsigned int length = spans[size].length;
        unsigned int slash = buffer_findbyte(text, length, '/');

        if (setfontsize(context, text, slash) && slash < length)
            setlineheight(context, text + slash + 1, length - slash - 1);

    }

}

static unsigned int parsedisplay(char *text, unsigned int length)
{

    if (matchword(text, length, "block") || matchword(text, length, "flow-root") || matchword(text, length, "grid") || matchword(text, length, "table-caption"))
        return CSS_DISPLAY_BLOCK;

    if (matchword(text, length, "inline") || matchword(text, length, "contents"))
        return CSS_DISPLAY_INLINE;

    if (matchword(text, length, "inline-block") || matchword(text, length, "inline-flex") || matchword(text, length, "inline-grid") || matchword(text, length, "inline-table"))
        return CSS_DISPLAY_INLINEBLOCK;

    if (matchword(text, length, "list-item"))
        return CSS_DISPLAY_LISTITEM;

    if (matchword(text, length, "none"))
        return CSS_DISPLAY_NONE;

    if (matchword(text, length, "table"))
        return CSS_DISPLAY_TABLE;

    if (matchword(text, length, "table-row-group") || matchword(text, length, "table-header-group") || matchword(text, length, "table-footer-group"))
        return CSS_DISPLAY_ROWGROUP;

    if (matchword(text, length, "table-row"))
        return CSS_DISPLAY_ROW;

    if (matchword(text, length, "table-cell"))
        return CSS_DISPLAY_CELL;

    if (matchword(text, length, "flex"))
        return CSS_DISPLAY_FLEX;

    return 0xFF;

}

static unsigned int parseliststyle(char *text, unsigned int length)
{

    if (matchword(text, length, "none"))
        return CSS_LIST_NONE;

    if (matchword(text, length, "disc"))
        return CSS_LIST_DISC;

    if (matchword(text, length, "circle"))
        return CSS_LIST_CIRCLE;

    if (matchword(text, length, "square"))
        return CSS_LIST_SQUARE;

    if (matchword(text, length, "decimal") || matchword(text, length, "decimal-leading-zero") || matchword(text, length, "lower-alpha") || matchword(text, length, "upper-alpha") || matchword(text, length, "lower-roman") || matchword(text, length, "upper-roman") || matchword(text, length, "lower-latin") || matchword(text, length, "upper-latin"))
        return CSS_LIST_DECIMAL;

    return 0xFF;

}

static unsigned int parseborderstyle(char *text, unsigned int length)
{

    if (matchword(text, length, "none") || matchword(text, length, "hidden"))
        return 0;

    if (matchword(text, length, "solid") || matchword(text, length, "dashed") || matchword(text, length, "dotted") || matchword(text, length, "double") || matchword(text, length, "groove") || matchword(text, length, "ridge") || matchword(text, length, "inset") || matchword(text, length, "outset"))
        return 1;

    return 0xFF;

}

static unsigned int parseborderwidth(struct context *context, char *text, unsigned int length)
{

    struct lengthvalue value;

    if (matchword(text, length, "thin"))
        return 1;

    if (matchword(text, length, "medium"))
        return 3;

    if (matchword(text, length, "thick"))
        return 5;

    value = parselength(context, text, length, context->style->fontsize);

    if (!value.valid || value.length.unit != CSS_UNIT_PX || value.length.value < 0)
        return 0xFFFF;

    return (value.length.value > 255) ? 255 : value.length.value;

}

static void setborderside(struct context *context, unsigned int side, char *value)
{

    struct span spans[4];
    unsigned int count = countspans(value, spans, 4);
    unsigned int width = 3;
    unsigned int style = 0;
    unsigned int color = CURRENTCOLOR;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char *text = value + spans[i].start;
        unsigned int length = spans[i].length;
        unsigned int parsedstyle = parseborderstyle(text, length);
        unsigned int parsedwidth = parseborderwidth(context, text, length);
        struct colorvalue parsedcolor = parsecolor(text, length);

        if (parsedstyle != 0xFF)
            style = parsedstyle;
        else if (parsedwidth != 0xFFFF)
            width = parsedwidth;
        else if (parsedcolor.valid)
            color = parsedcolor.value;
        else
            return;

    }

    context->style->borderwidth[side] = width;
    context->style->borderstyle[side] = style;
    context->style->bordercolor[side] = color;

}

static unsigned int sideindex(unsigned int count, unsigned int side)
{

    switch (count)
    {

    case 1:
        return 0;

    case 2:
        return side % 2;

    case 3:
        return (side == CSS_LEFT) ? 1 : side;

    }

    return side;

}

static void setsides(struct context *context, unsigned int property, char *value)
{

    struct span spans[CSS_SIDES];
    unsigned int count = countspans(value, spans, CSS_SIDES);
    struct css_style *style = context->style;
    unsigned int side;

    if (!count)
        return;

    for (side = 0; side < CSS_SIDES; side++)
    {

        struct span *span = &spans[sideindex(count, side)];
        char *text = value + span->start;

        if (property == PROP_MARGIN || property == PROP_PADDING)
        {

            struct lengthvalue length = parselength(context, text, span->length, style->fontsize);

            if (!length.valid || length.length.unit == CSS_UNIT_NONE || (property == PROP_PADDING && length.length.unit == CSS_UNIT_AUTO))
                return;

        }

        else if (property == PROP_BORDERWIDTH)
        {

            if (parseborderwidth(context, text, span->length) == 0xFFFF)
                return;

        }

        else if (property == PROP_BORDERSTYLE)
        {

            if (parseborderstyle(text, span->length) == 0xFF)
                return;

        }

        else if (!parsecolor(text, span->length).valid)
        {

            return;

        }

    }

    for (side = 0; side < CSS_SIDES; side++)
    {

        struct span *span = &spans[sideindex(count, side)];
        char *text = value + span->start;
        struct lengthvalue length = parselength(context, text, span->length, style->fontsize);

        if (property == PROP_MARGIN)
            copylength(&style->margin[side], &length.length);
        else if (property == PROP_PADDING)
            copylength(&style->padding[side], &length.length);
        else if (property == PROP_BORDERWIDTH)
            style->borderwidth[side] = parseborderwidth(context, text, span->length);
        else if (property == PROP_BORDERSTYLE)
            style->borderstyle[side] = parseborderstyle(text, span->length);
        else
            style->bordercolor[side] = parsecolor(text, span->length).value;

    }

}

static void setlength(struct context *context, struct css_length *length, char *value, unsigned int noauto)
{

    struct lengthvalue parsed = parselength(context, value, cstring_length(value), context->style->fontsize);

    if (parsed.valid && !(noauto && parsed.length.unit == CSS_UNIT_AUTO))
    {

        length->value = parsed.length.value;
        length->unit = parsed.length.unit;

    }

}

static void setbackground(struct context *context, char *value)
{

    struct span spans[8];
    unsigned int count = countspans(value, spans, 8);
    unsigned int color = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct colorvalue parsed = parsecolor(value + spans[i].start, spans[i].length);

        if (parsed.valid)
            color = parsed.value;

    }

    context->style->background = color;

}

static void setdecoration(struct context *context, char *value)
{

    struct span spans[4];
    unsigned int count = countspans(value, spans, 4);
    unsigned int decoration = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char *text = value + spans[i].start;

        if (matchword(text, spans[i].length, "underline"))
            decoration |= CSS_DECORATION_UNDERLINE;

        if (matchword(text, spans[i].length, "line-through"))
            decoration |= CSS_DECORATION_LINETHROUGH;

    }

    context->style->decoration = decoration;

}

static void copyproperty(struct css_style *style, struct css_style *from, unsigned int property)
{

    unsigned int side;

    switch (property)
    {

    case PROP_DISPLAY:
        style->display = from->display;

        break;

    case PROP_COLOR:
        style->color = from->color;

        break;

    case PROP_BACKGROUND:
    case PROP_BACKGROUNDCOLOR:
        style->background = from->background;

        break;

    case PROP_FONTSIZE:
        style->fontsize = from->fontsize;

        break;

    case PROP_FONTWEIGHT:
        style->bold = from->bold;

        break;

    case PROP_FONTSTYLE:
        style->italic = from->italic;

        break;

    case PROP_FONT:
        style->fontsize = from->fontsize;
        style->bold = from->bold;
        style->italic = from->italic;
        style->lineheight = from->lineheight;
        style->lineheightfactor = from->lineheightfactor;

        break;

    case PROP_LINEHEIGHT:
        style->lineheight = from->lineheight;
        style->lineheightfactor = from->lineheightfactor;

        break;

    case PROP_TEXTALIGN:
        style->align = from->align;

        break;

    case PROP_TEXTDECORATION:
    case PROP_TEXTDECORATIONLINE:
        style->decoration = from->decoration;

        break;

    case PROP_WHITESPACE:
        style->whitespace = from->whitespace;

        break;

    case PROP_LISTSTYLE:
    case PROP_LISTSTYLETYPE:
        style->liststyle = from->liststyle;

        break;

    case PROP_VISIBILITY:
        style->hidden = from->hidden;

        break;

    case PROP_WIDTH:
        copylength(&style->width, &from->width);

        break;

    case PROP_HEIGHT:
        copylength(&style->height, &from->height);

        break;

    case PROP_MINWIDTH:
        copylength(&style->minwidth, &from->minwidth);

        break;

    case PROP_MAXWIDTH:
        copylength(&style->maxwidth, &from->maxwidth);

        break;

    case PROP_MINHEIGHT:
        copylength(&style->minheight, &from->minheight);

        break;

    case PROP_BOXSIZING:
        style->borderbox = from->borderbox;

        break;

    case PROP_POSITION:
        style->position = from->position;

        break;

    case PROP_FLEXDIRECTION:
        style->column = from->column;

        break;

    default:
        for (side = 0; side < CSS_SIDES; side++)
        {

            if (property == PROP_MARGIN || property == PROP_MARGINTOP + side)
                copylength(&style->margin[side], &from->margin[side]);

            if (property == PROP_PADDING || property == PROP_PADDINGTOP + side)
                copylength(&style->padding[side], &from->padding[side]);

            if (property == PROP_BORDER || property == PROP_BORDERTOP + side || property == PROP_BORDERWIDTH || property == PROP_BORDERTOPWIDTH + side)
                style->borderwidth[side] = from->borderwidth[side];

            if (property == PROP_BORDER || property == PROP_BORDERTOP + side || property == PROP_BORDERSTYLE || property == PROP_BORDERTOPSTYLE + side)
                style->borderstyle[side] = from->borderstyle[side];

            if (property == PROP_BORDER || property == PROP_BORDERTOP + side || property == PROP_BORDERCOLOR || property == PROP_BORDERTOPCOLOR + side)
                style->bordercolor[side] = from->bordercolor[side];

        }

        break;

    }

}

static void initstyle(struct css_style *style)
{

    unsigned int side;

    buffer_clear(style, sizeof (struct css_style));

    style->color = 0xFF000000;
    style->fontsize = 16;
    style->liststyle = CSS_LIST_DISC;
    style->width.unit = CSS_UNIT_AUTO;
    style->height.unit = CSS_UNIT_AUTO;
    style->maxwidth.unit = CSS_UNIT_NONE;

    for (side = 0; side < CSS_SIDES; side++)
    {

        style->borderwidth[side] = 3;
        style->bordercolor[side] = CURRENTCOLOR;

    }

}

static void apply(struct context *context, unsigned int property, char *value)
{

    struct css_style *style = context->style;
    unsigned int length = cstring_length(value);
    struct colorvalue color;
    unsigned int parsed;

    if (cstring_match(value, "inherit"))
    {

        copyproperty(style, context->parent, property);

        return;

    }

    if (cstring_match(value, "initial") || cstring_match(value, "unset") || cstring_match(value, "revert"))
    {

        struct css_style initial;

        initstyle(&initial);
        copyproperty(style, &initial, property);

        return;

    }

    switch (property)
    {

    case PROP_DISPLAY:
        parsed = parsedisplay(value, length);

        if (parsed != 0xFF)
            style->display = parsed;

        break;

    case PROP_COLOR:
        color = parsecolor(value, length);

        if (color.valid)
            style->color = (color.value == CURRENTCOLOR) ? context->parent->color : color.value;

        break;

    case PROP_BACKGROUND:
        setbackground(context, value);

        break;

    case PROP_BACKGROUNDCOLOR:
        color = parsecolor(value, length);

        if (color.valid)
            style->background = (color.value == CURRENTCOLOR) ? style->color : color.value;

        break;

    case PROP_FONTSIZE:
        setfontsize(context, value, length);

        break;

    case PROP_FONTWEIGHT:
        setweight(context, value, length);

        break;

    case PROP_FONTSTYLE:
        style->italic = matchword(value, length, "italic") || matchword(value, length, "oblique");

        break;

    case PROP_FONT:
        setfont(context, value);

        break;

    case PROP_LINEHEIGHT:
        setlineheight(context, value, length);

        break;

    case PROP_TEXTALIGN:
        if (matchword(value, length, "center") || matchword(value, length, "-webkit-center"))
            style->align = CSS_ALIGN_CENTER;
        else if (matchword(value, length, "right") || matchword(value, length, "end"))
            style->align = CSS_ALIGN_RIGHT;
        else if (matchword(value, length, "left") || matchword(value, length, "start") || matchword(value, length, "justify"))
            style->align = CSS_ALIGN_LEFT;

        break;

    case PROP_TEXTDECORATION:
    case PROP_TEXTDECORATIONLINE:
        setdecoration(context, value);

        break;

    case PROP_WHITESPACE:
        if (matchword(value, length, "normal"))
            style->whitespace = CSS_WHITESPACE_NORMAL;
        else if (matchword(value, length, "pre"))
            style->whitespace = CSS_WHITESPACE_PRE;
        else if (matchword(value, length, "nowrap"))
            style->whitespace = CSS_WHITESPACE_NOWRAP;
        else if (matchword(value, length, "pre-wrap") || matchword(value, length, "break-spaces"))
            style->whitespace = CSS_WHITESPACE_PREWRAP;
        else if (matchword(value, length, "pre-line"))
            style->whitespace = CSS_WHITESPACE_PRELINE;

        break;

    case PROP_LISTSTYLE:
    case PROP_LISTSTYLETYPE:
        {

            struct span spans[4];
            unsigned int count = countspans(value, spans, 4);
            unsigned int i;

            for (i = 0; i < count; i++)
            {

                parsed = parseliststyle(value + spans[i].start, spans[i].length);

                if (parsed != 0xFF)
                    style->liststyle = parsed;

            }

        }

        break;

    case PROP_VISIBILITY:
        style->hidden = matchword(value, length, "hidden") || matchword(value, length, "collapse");

        break;

    case PROP_MARGIN:
    case PROP_PADDING:
    case PROP_BORDERWIDTH:
    case PROP_BORDERSTYLE:
    case PROP_BORDERCOLOR:
        setsides(context, property, value);

        break;

    case PROP_BORDER:
        setborderside(context, CSS_TOP, value);
        setborderside(context, CSS_RIGHT, value);
        setborderside(context, CSS_BOTTOM, value);
        setborderside(context, CSS_LEFT, value);

        break;

    case PROP_WIDTH:
        setlength(context, &style->width, value, 0);

        break;

    case PROP_HEIGHT:
        setlength(context, &style->height, value, 0);

        break;

    case PROP_MINWIDTH:
        setlength(context, &style->minwidth, value, 1);

        break;

    case PROP_MAXWIDTH:
        setlength(context, &style->maxwidth, value, 1);

        break;

    case PROP_MINHEIGHT:
        setlength(context, &style->minheight, value, 1);

        break;

    case PROP_BOXSIZING:
        style->borderbox = matchword(value, length, "border-box");

        break;

    case PROP_POSITION:
        if (matchword(value, length, "static"))
            style->position = CSS_POSITION_STATIC;
        else if (matchword(value, length, "relative") || matchword(value, length, "sticky"))
            style->position = CSS_POSITION_RELATIVE;
        else if (matchword(value, length, "absolute"))
            style->position = CSS_POSITION_ABSOLUTE;
        else if (matchword(value, length, "fixed"))
            style->position = CSS_POSITION_FIXED;

        break;

    case PROP_FLEXDIRECTION:
        style->column = matchword(value, length, "column") || matchword(value, length, "column-reverse");

        break;

    default:
        if (property >= PROP_MARGINTOP && property < PROP_MARGINTOP + CSS_SIDES)
        {

            struct lengthvalue parsedlength = parselength(context, value, length, style->fontsize);

            if (parsedlength.valid && parsedlength.length.unit != CSS_UNIT_NONE)
                copylength(&style->margin[property - PROP_MARGINTOP], &parsedlength.length);

        }

        else if (property >= PROP_PADDINGTOP && property < PROP_PADDINGTOP + CSS_SIDES)
        {

            struct lengthvalue parsedlength = parselength(context, value, length, style->fontsize);

            if (parsedlength.valid && parsedlength.length.unit <= CSS_UNIT_PERCENT)
                copylength(&style->padding[property - PROP_PADDINGTOP], &parsedlength.length);

        }

        else if (property >= PROP_BORDERTOP && property < PROP_BORDERTOP + CSS_SIDES)
        {

            setborderside(context, property - PROP_BORDERTOP, value);

        }

        else if (property >= PROP_BORDERTOPWIDTH && property < PROP_BORDERTOPWIDTH + CSS_SIDES)
        {

            parsed = parseborderwidth(context, value, length);

            if (parsed != 0xFFFF)
                style->borderwidth[property - PROP_BORDERTOPWIDTH] = parsed;

        }

        else if (property >= PROP_BORDERTOPSTYLE && property < PROP_BORDERTOPSTYLE + CSS_SIDES)
        {

            parsed = parseborderstyle(value, length);

            if (parsed != 0xFF)
                style->borderstyle[property - PROP_BORDERTOPSTYLE] = parsed;

        }

        else if (property >= PROP_BORDERTOPCOLOR && property < PROP_BORDERTOPCOLOR + CSS_SIDES)
        {

            color = parsecolor(value, length);

            if (color.valid)
                style->bordercolor[property - PROP_BORDERTOPCOLOR] = color.value;

        }

        break;

    }

}

static unsigned int phaseof(unsigned int property)
{

    if (property == PROP_CUSTOM)
        return PHASE_CUSTOM;

    if (property == PROP_FONTSIZE || property == PROP_FONT)
        return PHASE_FONT;

    return PHASE_OTHER;

}

static unsigned int parentelement(struct html_document *document, unsigned int node)
{

    unsigned int parent = document->nodes[node].parent;

    return (parent && document->nodes[parent].type == HTML_NODE_ELEMENT) ? parent : 0;

}

static unsigned int prevelement(struct html_document *document, unsigned int node)
{

    unsigned int prev = document->nodes[node].prev;

    while (prev && document->nodes[prev].type != HTML_NODE_ELEMENT)
        prev = document->nodes[prev].prev;

    return prev;

}

static unsigned int nextelement(struct html_document *document, unsigned int node)
{

    unsigned int next = document->nodes[node].next;

    while (next && document->nodes[next].type != HTML_NODE_ELEMENT)
        next = document->nodes[next].next;

    return next;

}

static unsigned int hasword(char *list, char *word)
{

    unsigned int length = cstring_length(word);

    while (list && list[0])
    {

        unsigned int wordlength = 0;

        while (list[0] && isspace(list[0]))
            list++;

        while (list[wordlength] && !isspace(list[wordlength]))
            wordlength++;

        if (wordlength == length && buffer_match(list, word, length))
            return 1;

        list += wordlength;

    }

    return 0;

}

static unsigned int hasprefix(char *text, char *prefix)
{

    unsigned int length = cstring_length(prefix);

    return cstring_length(text) >= length && buffer_match(text, prefix, length);

}

static unsigned int hassuffix(char *text, char *suffix)
{

    unsigned int length = cstring_length(suffix);
    unsigned int textlength = cstring_length(text);

    return textlength >= length && buffer_match(text + textlength - length, suffix, length);

}

static unsigned int matchattribute(struct matcher *matcher, struct css_part *part, unsigned int node)
{

    char *value = html_getattribute(matcher->document, node, poolstring(matcher->sheet, part->name));
    char *expected = poolstring(matcher->sheet, part->value);

    if (!value)
        return 0;

    switch (part->op)
    {

    case OP_EXISTS:
        return 1;

    case OP_EQUAL:
        return cstring_match(value, expected);

    case OP_INCLUDES:
        return hasword(value, expected);

    case OP_DASH:
        return cstring_match(value, expected) || (hasprefix(value, expected) && value[cstring_length(expected)] == '-');

    case OP_PREFIX:
        return expected[0] && hasprefix(value, expected);

    case OP_SUFFIX:
        return expected[0] && hassuffix(value, expected);

    case OP_SUBSTRING:
        return expected[0] && contains(value, cstring_length(value), expected);

    }

    return 0;

}

static unsigned int elementindex(struct html_document *document, unsigned int node, unsigned int sametype)
{

    unsigned int index = 1;
    unsigned int prev = prevelement(document, node);

    while (prev)
    {

        if (!sametype || (document->nodes[prev].tag == document->nodes[node].tag && cstring_match(html_getstring(document, document->nodes[prev].name), html_getstring(document, document->nodes[node].name))))
            index++;

        prev = prevelement(document, prev);

    }

    return index;

}

static unsigned int islasttype(struct html_document *document, unsigned int node)
{

    unsigned int next = nextelement(document, node);

    while (next)
    {

        if (cstring_match(html_getstring(document, document->nodes[next].name), html_getstring(document, document->nodes[node].name)))
            return 0;

        next = nextelement(document, next);

    }

    return 1;

}

static unsigned int matchnth(int a, int b, int index)
{

    int d = index - b;

    if (!a)
        return d == 0;

    return d == 0 || ((d > 0) == (a > 0) && d % a == 0);

}

static unsigned int matchpart(struct matcher *matcher, struct css_part *part, unsigned int node)
{

    struct html_document *document = matcher->document;
    struct html_node *element = &document->nodes[node];
    unsigned int child;

    switch (part->type)
    {

    case PART_TAG:
        return cstring_match(html_getstring(document, element->name), poolstring(matcher->sheet, part->name));

    case PART_UNIVERSAL:
        return 1;

    case PART_ID:
        return cstring_match(html_getattribute(document, node, "id") ? html_getattribute(document, node, "id") : "", poolstring(matcher->sheet, part->name));

    case PART_CLASS:
        return hasword(html_getattribute(document, node, "class"), poolstring(matcher->sheet, part->name));

    case PART_ATTRIBUTE:
        return matchattribute(matcher, part, node);

    case PART_FIRSTCHILD:
        return !prevelement(document, node);

    case PART_LASTCHILD:
        return !nextelement(document, node);

    case PART_FIRSTOFTYPE:
        return elementindex(document, node, 1) == 1;

    case PART_LASTOFTYPE:
        return islasttype(document, node);

    case PART_LINK:
        return (element->tag == HTML_TAG_A || cstring_match(html_getstring(document, element->name), "area")) && html_getattribute(document, node, "href");

    case PART_ROOT:
        return node == document->html;

    case PART_NTHCHILD:
        return matchnth((int)part->name, (int)part->value, elementindex(document, node, 0));

    case PART_CHECKED:
        return html_getattribute(document, node, "checked") || html_getattribute(document, node, "selected");

    case PART_DISABLED:
        return html_getattribute(document, node, "disabled") != 0;

    case PART_EMPTY:
        for (child = element->first; child; child = document->nodes[child].next)
        {

            if (document->nodes[child].type == HTML_NODE_ELEMENT || document->nodes[child].length)
                return 0;

        }

        return 1;

    }

    return 0;

}

static unsigned int matchcompound(struct matcher *matcher, unsigned int start, unsigned int end, unsigned int node)
{

    unsigned int i;

    for (i = start; i < end; i++)
    {

        struct css_part *part = &matcher->sheet->parts[i];

        if (matchpart(matcher, part, node) == part->negate)
            return 0;

    }

    return 1;

}

static unsigned int matchselector(struct matcher *matcher, unsigned int first, unsigned int end, unsigned int node)
{

    struct css_part *parts = matcher->sheet->parts;
    unsigned int start = end - 1;
    unsigned int other;

    while (start > first && parts[start].combinator == COMBINATOR_NONE)
        start--;

    if (!matchcompound(matcher, start, end, node))
        return 0;

    switch (parts[start].combinator)
    {

    case COMBINATOR_DESCENDANT:
        for (other = parentelement(matcher->document, node); other; other = parentelement(matcher->document, other))
        {

            if (matchselector(matcher, first, start, other))
                return 1;

        }

        return 0;

    case COMBINATOR_CHILD:
        other = parentelement(matcher->document, node);

        return other && matchselector(matcher, first, start, other);

    case COMBINATOR_ADJACENT:
        other = prevelement(matcher->document, node);

        return other && matchselector(matcher, first, start, other);

    case COMBINATOR_SIBLING:
        for (other = prevelement(matcher->document, node); other; other = prevelement(matcher->document, other))
        {

            if (matchselector(matcher, first, start, other))
                return 1;

        }

        return 0;

    }

    return 1;

}

static unsigned int matchkey(struct css_sheet *sheet, struct css_selector *selector, char *name, char *id, char *classes)
{

    switch (selector->keytype)
    {

    case KEY_TAG:
        return cstring_match(name, poolstring(sheet, selector->key));

    case KEY_ID:
        return id && cstring_match(id, poolstring(sheet, selector->key));

    case KEY_CLASS:
        return hasword(classes, poolstring(sheet, selector->key));

    }

    return 1;

}

static unsigned int before(struct match *a, struct match *b)
{

    if (a->origin != b->origin)
        return a->origin < b->origin;

    if (a->specificity != b->specificity)
        return a->specificity < b->specificity;

    return a->rule < b->rule;

}

static unsigned int collect(struct css_sheet *sheet, struct html_document *document, unsigned int node, unsigned int width, struct match matches[MATCHES])
{

    struct matcher matcher;
    char *name = html_getstring(document, document->nodes[node].name);
    char *id = html_getattribute(document, node, "id");
    char *classes = html_getattribute(document, node, "class");
    unsigned int count = 0;
    unsigned int i;

    matcher.sheet = sheet;
    matcher.document = document;

    for (i = 0; i < sheet->selectorcount && count < MATCHES; i++)
    {

        struct css_selector *selector = &sheet->selectors[i];
        struct css_rule *rule = &sheet->rules[selector->rule];

        if ((int)width < rule->minwidth || (int)width > rule->maxwidth)
            continue;

        if (matchkey(sheet, selector, name, id, classes) && matchselector(&matcher, selector->part, selector->part + selector->count, node))
        {

            unsigned int j = count;

            matches[count].origin = rule->origin;
            matches[count].specificity = selector->specificity;
            matches[count].rule = selector->rule;

            while (j > 0 && before(&matches[count], &matches[j - 1]))
                j--;

            if (j < count)
            {

                struct match match;

                buffer_copy(&match, &matches[count], sizeof (struct match));
                buffer_copy(&matches[j + 1], &matches[j], sizeof (struct match) * (count - j));
                buffer_copy(&matches[j], &match, sizeof (struct match));

            }

            count++;

        }

    }

    return count;

}

static void setcustom(struct context *context, unsigned int name, unsigned int value)
{

    struct css_sheet *sheet = context->sheet;
    unsigned int start = sheet->customstart[context->node];
    unsigned int i;

    for (i = start; i < start + sheet->customcount[context->node]; i++)
    {

        if (cstring_match(poolstring(sheet, sheet->customs[i].name), poolstring(sheet, name)))
        {

            sheet->customs[i].value = value;

            return;

        }

    }

    if (sheet->customtotal >= CSS_CUSTOMS || sheet->customtotal != start + sheet->customcount[context->node])
    {

        sheet->full = 1;

        return;

    }

    sheet->customs[sheet->customtotal].name = name;
    sheet->customs[sheet->customtotal].value = value;
    sheet->customtotal++;
    sheet->customcount[context->node]++;

}

static char *findcustom(struct context *context, char *name, unsigned int length)
{

    struct css_sheet *sheet = context->sheet;
    unsigned int node = context->node;

    while (node)
    {

        unsigned int start = sheet->customstart[node];
        unsigned int i;

        for (i = start; i < start + sheet->customcount[node]; i++)
        {

            if (matchword(name, length, poolstring(sheet, sheet->customs[i].name)))
                return poolstring(sheet, sheet->customs[i].value);

        }

        node = parentelement(context->document, node);

    }

    return 0;

}

static void expand(struct context *context, struct expansion *expansion, char *text, unsigned int length, unsigned int depth)
{

    unsigned int i = 0;

    while (i < length && expansion->valid)
    {

        if (i + 4 < length && buffer_match(text + i, "var(", 4))
        {

            unsigned int close = findclose(text, i + 3, length);
            unsigned int start = skipspace(text, i + 4, close);
            unsigned int comma = findstop(text, start, close, ",");
            unsigned int end = comma;
            char *value;

            while (end > start && isspace(text[end - 1]))
                end--;

            value = findcustom(context, text + start, end - start);

            if (value && value[0] && depth < EXPANDDEPTH)
                expand(context, expansion, value, cstring_length(value), depth + 1);
            else if (comma < close && depth < EXPANDDEPTH)
                expand(context, expansion, text + skipspace(text, comma + 1, close), close - skipspace(text, comma + 1, close), depth + 1);
            else
                expansion->valid = 0;

            i = close + 1;

            continue;

        }

        if (expansion->length + 1 >= EXPANDSIZE)
        {

            expansion->valid = 0;

            return;

        }

        expansion->text[expansion->length++] = text[i++];

    }

}

static void applyvalue(struct context *context, unsigned int property, char *value)
{

    struct expansion expansion;

    if (!contains(value, cstring_length(value), "var("))
    {

        apply(context, property, value);

        return;

    }

    expansion.length = 0;
    expansion.valid = 1;

    expand(context, &expansion, value, cstring_length(value), 0);

    if (expansion.valid)
    {

        expansion.text[expansion.length] = '\0';

        apply(context, property, expansion.text);

    }

}

static void applydeclarations(struct context *context, struct css_sheet *sheet, unsigned int start, unsigned int count, unsigned int important, unsigned int phase)
{

    unsigned int i;

    for (i = start; i < start + count; i++)
    {

        struct css_declaration *declaration = &sheet->declarations[i];

        if (declaration->important != important || phaseof(declaration->property) != phase)
            continue;

        if (phase == PHASE_CUSTOM)
            setcustom(context, declaration->name, declaration->value);
        else
            applyvalue(context, declaration->property, poolstring(sheet, declaration->value));

    }

}

static void applyrules(struct context *context, struct css_sheet *sheet, struct match *matches, unsigned int count, unsigned int origin, unsigned int important, unsigned int phase)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct css_rule *rule = &sheet->rules[matches[i].rule];

        if (matches[i].origin == origin)
            applydeclarations(context, sheet, rule->declaration, rule->declarations, important, phase);

    }

}

static void applyhint(struct context *context, unsigned int property, char *value, char *suffix)
{

    char buffer[VALUESIZE];
    unsigned int length = cstring_length(value);
    unsigned int i;

    if (length + cstring_length(suffix) + 1 > VALUESIZE)
        return;

    for (i = 0; i < length; i++)
        buffer[i] = lower(value[i]);

    buffer[length] = '\0';

    if (length && cstring_isdigit(buffer[length - 1]))
        length += cstring_write(buffer, VALUESIZE, suffix, length);

    buffer[length] = '\0';

    apply(context, property, buffer);

}

static unsigned int findtable(struct html_document *document, unsigned int node)
{

    while (node && document->nodes[node].tag != HTML_TAG_TABLE)
        node = parentelement(document, node);

    return node;

}

static void applyhints(struct context *context, struct html_document *document, unsigned int node, unsigned int fonts)
{

    struct html_node *element = &document->nodes[node];
    char *value;

    if (fonts)
    {

        if (element->tag == HTML_TAG_FONT && (value = html_getattribute(document, node, "size")))
        {

            static unsigned int sizes[FONTSIZES] = {10, 10, 13, 16, 18, 24, 32, 48};
            int size = (value[0] == '+' || value[0] == '-') ? 3 : 0;
            int number = (int)cstring_read_value(value + (size ? 1 : 0), cstring_length(value + (size ? 1 : 0)), 10);

            size = (value[0] == '-') ? size - number : size + number;
            size = (size < 1) ? 1 : (size > 7) ? 7 : size;
            context->style->fontsize = sizes[size];

        }

        return;

    }

    if ((value = html_getattribute(document, node, "bgcolor")))
        applyhint(context, PROP_BACKGROUNDCOLOR, value, "");

    if (element->tag == HTML_TAG_FONT && (value = html_getattribute(document, node, "color")))
        applyhint(context, PROP_COLOR, value, "");

    if (node == document->body && (value = html_getattribute(document, node, "text")))
        applyhint(context, PROP_COLOR, value, "");

    if (element->tag != HTML_TAG_FONT && (value = html_getattribute(document, node, "width")))
        applyhint(context, PROP_WIDTH, value, "px");

    if (element->tag != HTML_TAG_FONT && (value = html_getattribute(document, node, "height")))
        applyhint(context, PROP_HEIGHT, value, "px");

    if ((value = html_getattribute(document, node, "align")))
    {

        if (element->tag == HTML_TAG_TABLE || element->tag == HTML_TAG_IMG)
        {

            if (cstring_match(value, "center"))
                apply(context, PROP_MARGIN, "0 auto");

        }

        else
        {

            applyhint(context, PROP_TEXTALIGN, value, "");

        }

    }

    if (element->tag == HTML_TAG_TD || element->tag == HTML_TAG_TH)
    {

        unsigned int table = findtable(document, node);

        if (html_getattribute(document, node, "nowrap"))
            apply(context, PROP_WHITESPACE, "nowrap");

        if (table && (value = html_getattribute(document, table, "cellpadding")))
            applyhint(context, PROP_PADDING, value, "px");

        if (table && (value = html_getattribute(document, table, "border")) && cstring_read_value(value, cstring_length(value), 10))
            apply(context, PROP_BORDER, "1px solid gray");

    }

    if (element->tag == HTML_TAG_TABLE && (value = html_getattribute(document, node, "border")) && cstring_read_value(value, cstring_length(value), 10))
        apply(context, PROP_BORDER, "1px outset gray");

}

static void inherit(struct css_style *style, struct css_style *parent)
{

    initstyle(style);

    style->color = parent->color;
    style->fontsize = parent->fontsize;
    style->lineheight = parent->lineheight;
    style->lineheightfactor = parent->lineheightfactor;
    style->bold = parent->bold;
    style->italic = parent->italic;
    style->align = parent->align;
    style->whitespace = parent->whitespace;
    style->liststyle = parent->liststyle;
    style->decoration = parent->decoration;
    style->hidden = parent->hidden;

}

static void finish(struct css_style *style)
{

    unsigned int side;

    if (style->lineheightfactor)
        style->lineheight = style->fontsize * style->lineheightfactor / 1000;

    for (side = 0; side < CSS_SIDES; side++)
    {

        if (!style->borderstyle[side])
            style->borderwidth[side] = 0;

        if (style->bordercolor[side] == CURRENTCOLOR)
            style->bordercolor[side] = style->color;

    }

}

static void computeelement(struct css_sheet *sheet, struct html_document *document, struct context *context, unsigned int node)
{

    struct match matches[MATCHES];
    unsigned int count = collect(sheet, document, node, context->width, matches);
    char *styleattr = html_getattribute(document, node, "style");
    unsigned int declarationmark = sheet->declarationcount;
    unsigned int phase;

    sheet->customstart[node] = sheet->customtotal;
    sheet->customcount[node] = 0;

    if (styleattr)
        parsedeclarations(sheet, styleattr, 0, cstring_length(styleattr));

    for (phase = 0; phase < PHASES; phase++)
    {

        applyrules(context, sheet, matches, count, CSS_ORIGIN_AGENT, 0, phase);

        if (phase != PHASE_CUSTOM)
            applyhints(context, document, node, phase == PHASE_FONT);

        applyrules(context, sheet, matches, count, CSS_ORIGIN_AUTHOR, 0, phase);
        applydeclarations(context, sheet, declarationmark, sheet->declarationcount - declarationmark, 0, phase);
        applyrules(context, sheet, matches, count, CSS_ORIGIN_AUTHOR, 1, phase);
        applydeclarations(context, sheet, declarationmark, sheet->declarationcount - declarationmark, 1, phase);

    }

    sheet->declarationcount = declarationmark;

    finish(context->style);

}

static int medialength(char *text, unsigned int length)
{

    struct number number = parsenumber(text, length);

    if (!number.length)
        return -1;

    if (matchword(text + number.length, length - number.length, "em") || matchword(text + number.length, length - number.length, "rem"))
        return roundmilli(number.milli * 16);

    return roundmilli(number.milli);

}

static unsigned int evaluatequery(struct parser *parser, char *text, unsigned int start, unsigned int end)
{

    unsigned int offset = skipspace(text, start, end);
    int minwidth = 0;
    int maxwidth = NOLIMIT;

    while (offset < end)
    {

        unsigned int stop;

        if (text[offset] == '(')
        {

            unsigned int close = findclose(text, offset, end);
            unsigned int colon = findstop(text, offset + 1, close, ":");
            unsigned int namestart = skipspace(text, offset + 1, colon);
            unsigned int nameend = namestart;
            unsigned int valuestart = skipspace(text, colon + 1, close);
            unsigned int valueend = close;
            char name[32];
            unsigned int i;

            while (nameend < colon && !isspace(text[nameend]) && nameend - namestart < 31)
                nameend++;

            while (valueend > valuestart && isspace(text[valueend - 1]))
                valueend--;

            for (i = namestart; i < nameend; i++)
                name[i - namestart] = lower(text[i]);

            name[nameend - namestart] = '\0';

            if (cstring_match(name, "min-width"))
                minwidth = medialength(text + valuestart, valueend - valuestart);
            else if (cstring_match(name, "max-width"))
                maxwidth = medialength(text + valuestart, valueend - valuestart);
            else if (cstring_match(name, "prefers-color-scheme") && valueend > valuestart && lower(text[valuestart]) == 'd')
                return 0;
            else if (cstring_match(name, "-webkit-min-device-pixel-ratio") || cstring_match(name, "min-resolution"))
                return 0;

            if (minwidth < 0 || maxwidth < 0)
                return 0;

            offset = skipspace(text, close + 1, end);

            continue;

        }

        stop = readident(text, offset, end);

        if (stop == offset)
            return 0;

        if (!(matchword(text + offset, stop - offset, "all") || matchword(text + offset, stop - offset, "screen") || matchword(text + offset, stop - offset, "only") || matchword(text + offset, stop - offset, "and")))
            return 0;

        offset = skipspace(text, stop, end);

    }

    parser->minwidth = (minwidth > parser->minwidth) ? minwidth : parser->minwidth;
    parser->maxwidth = (maxwidth < parser->maxwidth) ? maxwidth : parser->maxwidth;

    return 1;

}

static unsigned int evaluatemedia(struct parser *parser, char *text, unsigned int start, unsigned int end)
{

    if (skipspace(text, start, end) == end)
        return 1;

    while (start < end)
    {

        unsigned int stop = findstop(text, start, end, ",");

        if (evaluatequery(parser, text, start, stop))
            return 1;

        start = stop + 1;

    }

    return 0;

}

static void parseblock(struct parser *parser, unsigned int start, unsigned int end);

static void parseatrule(struct parser *parser, unsigned int end)
{

    char *text = parser->text;
    unsigned int namestart = parser->offset + 1;
    unsigned int nameend = readident(text, namestart, end);
    unsigned int stop = findstop(text, nameend, end, ";{");
    unsigned int close;

    if (stop >= end || text[stop] == ';')
    {

        parser->offset = stop + 1;

        return;

    }

    close = findclose(text, stop, end);
    parser->offset = close + 1;

    if (matchword(text + namestart, nameend - namestart, "media"))
    {

        int minwidth = parser->minwidth;
        int maxwidth = parser->maxwidth;

        if (evaluatemedia(parser, text, nameend, stop))
            parseblock(parser, stop + 1, close);

        parser->minwidth = minwidth;
        parser->maxwidth = maxwidth;

    }

    else if (matchword(text + namestart, nameend - namestart, "supports"))
    {

        unsigned int condition = skipspace(text, nameend, stop);

        if (!(stop - condition >= 3 && matchword(text + condition, 3, "not")))
            parseblock(parser, stop + 1, close);

    }

    else if (matchword(text + namestart, nameend - namestart, "layer"))
    {

        parseblock(parser, stop + 1, close);

    }

}

static void parseblock(struct parser *parser, unsigned int start, unsigned int end)
{

    char *text = parser->text;

    parser->offset = start;

    while (parser->offset < end)
    {

        unsigned int offset = skipspace(text, parser->offset, end);
        unsigned int stop;
        unsigned int close;

        parser->offset = offset;

        if (offset >= end)
            break;

        if (offset + 4 <= end && buffer_match(text + offset, "<!--", 4))
        {

            parser->offset += 4;

            continue;

        }

        if (offset + 3 <= end && buffer_match(text + offset, "-->", 3))
        {

            parser->offset += 3;

            continue;

        }

        if (text[offset] == '}' || text[offset] == ';')
        {

            parser->offset++;

            continue;

        }

        if (text[offset] == '@')
        {

            parseatrule(parser, end);

            continue;

        }

        stop = findstop(text, offset, end, "{;");

        if (stop >= end)
            break;

        if (text[stop] == ';')
        {

            parser->offset = stop + 1;

            continue;

        }

        close = findclose(text, stop, end);

        parserule(parser, offset, stop, stop + 1, close);

        parser->offset = close + 1;

    }

}

void css_parse(struct css_sheet *sheet, char *text, unsigned int count, unsigned int origin)
{

    struct parser parser;

    parser.sheet = sheet;
    parser.text = text;
    parser.offset = 0;
    parser.origin = origin;
    parser.minwidth = 0;
    parser.maxwidth = NOLIMIT;

    parseblock(&parser, 0, count);

}

void css_reset(struct css_sheet *sheet)
{

    sheet->partcount = 0;
    sheet->selectorcount = 0;
    sheet->declarationcount = 0;
    sheet->rulecount = 0;
    sheet->poolcount = 1;
    sheet->pool[0] = '\0';
    sheet->full = 0;

    css_parse(sheet, agent, cstring_length(agent), CSS_ORIGIN_AGENT);

}

void css_compute(struct css_sheet *sheet, struct html_document *document, struct css_style *styles, unsigned int width, unsigned int height)
{

    struct css_style root;
    struct context context;
    unsigned int full = sheet->full;
    unsigned int poolmark = sheet->poolcount;
    unsigned int node;

    initstyle(&root);

    sheet->customtotal = 0;
    context.sheet = sheet;
    context.document = document;
    context.rootsize = 16;
    context.width = width;
    context.height = height;

    for (node = 1; node < document->nodecount; node++)
    {

        unsigned int parent = parentelement(document, node);

        if (document->nodes[node].type != HTML_NODE_ELEMENT)
            continue;

        context.node = node;
        context.style = &styles[node];
        context.parent = (parent) ? &styles[parent] : &root;

        inherit(context.style, context.parent);
        computeelement(sheet, document, &context, node);

        if (node == document->html)
            context.rootsize = context.style->fontsize;

    }

    sheet->poolcount = poolmark;
    sheet->full = full;

}
