#include <fudge.h>
#include "html.h"
#include "css.h"

#define PROPERTIES                      88
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
#define PROP_BACKGROUNDIMAGE            55
#define PROP_BORDERRADIUS               56
#define PROP_TEXTTRANSFORM              57
#define PROP_GRIDTEMPLATECOLUMNS        58
#define PROP_GAP                        59
#define PROP_ROWGAP                     60
#define PROP_COLUMNGAP                  61
#define PROP_GRIDGAP                    62
#define PROP_FILL                       63
#define PROP_JUSTIFYCONTENT             64
#define PROP_ALIGNITEMS                 65
#define PROP_ALIGNSELF                  66
#define PROP_FLEXGROW                   67
#define PROP_FLEXSHRINK                 68
#define PROP_FLEXBASIS                  69
#define PROP_FLEX                       70
#define PROP_FLEXWRAP                   71
#define PROP_FLEXFLOW                   72
#define PROP_FLOAT                      73
#define PROP_CLEAR                      74
#define PROP_OVERFLOW                   75
#define PROP_OVERFLOWX                  76
#define PROP_OVERFLOWY                  77
#define PROP_TOP                        78
#define PROP_OPACITY                    82
#define PROP_MAXHEIGHT                  83
#define PROP_FONTFAMILY                 84
#define PROP_BORDERSPACING              85
#define PROP_BORDERCOLLAPSE             86
#define PROP_ORDER                      87
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
#define PART_IS                         17
#define PART_ALTERNATIVE                18
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
#define GRADIENTARGS                    8
#define GRIDTRACKS                      12

struct parser
{

    struct css_sheet *sheet;
    char *text;
    unsigned int offset;
    unsigned int origin;
    unsigned int scheme;
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

struct quantity
{

    int milli;
    int percent;
    unsigned int length;

};

struct evaluator
{

    struct context *context;
    char *text;
    unsigned int offset;
    unsigned int count;
    unsigned int fontsize;
    unsigned int valid;
    unsigned int depth;

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
    unsigned int scheme;

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
    "max-width", "min-height", "box-sizing", "position", "flex-direction",
    "background-image", "border-radius", "text-transform", "grid-template-columns", "gap",
    "row-gap", "column-gap", "grid-gap", "fill",
    "justify-content", "align-items", "align-self", "flex-grow", "flex-shrink", "flex-basis", "flex", "flex-wrap", "flex-flow",
    "float", "clear", "overflow", "overflow-x", "overflow-y",
    "top", "right", "bottom", "left", "opacity", "max-height", "font-family", "border-spacing", "border-collapse", "order"
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
    "head,script,style,title,meta,link,base,template,option,datalist,param,area,map,noscript,[hidden],input[type=hidden]{display:none}"
    "details:not([open])>:not(summary){display:none}"
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
    "pre,code,kbd,samp,tt,listing,xmp,plaintext,textarea{font-family:monospace}"
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
    "button{display:inline-block;padding:1px 6px;border:2px outset #c0c0c0;background-color:#e0e0e0;color:#000}"
    "input,select,textarea{display:inline-block;border:2px inset #c0c0c0;padding:1px 2px;background-color:#fff;color:#000}"
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

static unsigned int parsealternative(struct selectorparser *selectorparser, unsigned int start, unsigned int end)
{

    struct css_sheet *sheet = selectorparser->sheet;
    unsigned int alternative = sheet->partcount;
    unsigned int offset = skipspace(selectorparser->text, start, end);

    while (end > offset && isspace(selectorparser->text[end - 1]))
        end--;

    if (offset >= end || !addpart(sheet, PART_ALTERNATIVE, COMBINATOR_NONE))
        return 0;

    while (offset < end)
    {

        offset = parsesimple(selectorparser, offset, end, COMBINATOR_NONE);

        if (!offset)
            return 0;

    }

    sheet->parts[alternative].name = sheet->partcount - alternative - 1;

    return 1;

}

static unsigned int parselist(struct selectorparser *selectorparser, unsigned int start, unsigned int end, unsigned int combinator, unsigned int negate, unsigned int weighted)
{

    struct css_sheet *sheet = selectorparser->sheet;
    unsigned int list = sheet->partcount;
    unsigned int specificity = selectorparser->specificity;
    unsigned int highest = 0;
    struct css_part *part = addpart(sheet, PART_IS, combinator);

    if (!part)
        return 0;

    part->negate = negate;

    while (start <= end)
    {

        unsigned int stop = findstop(selectorparser->text, start, end, ",");
        unsigned int partmark = sheet->partcount;
        unsigned int poolmark = sheet->poolcount;

        selectorparser->specificity = 0;

        if (parsealternative(selectorparser, start, stop))
        {

            if (selectorparser->specificity > highest)
                highest = selectorparser->specificity;

        }

        else
        {

            /* :is and :where forgive alternatives they can not parse, :not does not */
            if (negate || sheet->full)
                return 0;

            sheet->partcount = partmark;
            sheet->poolcount = poolmark;

        }

        start = stop + 1;

    }

    sheet->parts[list].name = sheet->partcount - list - 1;
    selectorparser->specificity = specificity + ((weighted) ? highest : 0);

    return 1;

}

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
            return (parselist(selectorparser, stop + 1, close, combinator, 1, 1)) ? close + 1 : 0;

        if (cstring_match(name, "is") || cstring_match(name, "matches") || cstring_match(name, "where"))
            return (parselist(selectorparser, stop + 1, close, combinator, 0, !cstring_match(name, "where"))) ? close + 1 : 0;

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

        if (part->type == PART_IS)
            i += part->name;

        if (part->negate || part->type == PART_IS)
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

    if (sheet->selectorcount >= CSS_SELECTORS)
        sheet->full = 1;

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
    rule->scheme = parser->scheme;
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
    result.length.offset = 0;
    result.valid = valid;

    return result;

}

static struct quantity quantity(int milli, int percent, unsigned int length)
{

    struct quantity result;

    result.milli = milli;
    result.percent = percent;
    result.length = length;

    return result;

}

static int multiply(int a, int b)
{

    return a / 1000 * b + a % 1000 * b / 1000;

}

static int divide(int a, int b)
{

    if (!b)
        return 0;

    return a / b * 1000 + a % b * 1000 / b;

}

static void skipblank(struct evaluator *evaluator)
{

    while (evaluator->offset < evaluator->count && evaluator->text[evaluator->offset] == ' ')
        evaluator->offset++;

}

static unsigned int nextis(struct evaluator *evaluator, char c)
{

    skipblank(evaluator);

    if (evaluator->offset < evaluator->count && evaluator->text[evaluator->offset] == c)
    {

        evaluator->offset++;

        return 1;

    }

    return 0;

}

static struct quantity evaluatesum(struct evaluator *evaluator);

static struct quantity evaluatefunction(struct evaluator *evaluator, unsigned int kind)
{

    struct quantity result = evaluatesum(evaluator);
    unsigned int count = 1;

    while (evaluator->valid && nextis(evaluator, ','))
    {

        struct quantity next = evaluatesum(evaluator);

        if (next.length != result.length || next.percent || result.percent)
            evaluator->valid = 0;

        if (kind == 1 && next.milli < result.milli)
            result.milli = next.milli;
        else if (kind == 2 && next.milli > result.milli)
            result.milli = next.milli;
        else if (kind == 3 && count == 1)
            result.milli = (next.milli > result.milli) ? next.milli : result.milli;
        else if (kind == 3 && count == 2 && next.milli < result.milli)
            result.milli = next.milli;

        count++;

    }

    if (kind == 3 && count != 3)
        evaluator->valid = 0;

    if (!nextis(evaluator, ')'))
        evaluator->valid = 0;

    return result;

}

static struct quantity evaluatevalue(struct evaluator *evaluator)
{

    char *text;
    unsigned int remaining;
    struct number number;
    unsigned int unit;

    skipblank(evaluator);

    text = evaluator->text + evaluator->offset;
    remaining = evaluator->count - evaluator->offset;

    if (evaluator->depth > 8 || !remaining)
    {

        evaluator->valid = 0;

        return quantity(0, 0, 0);

    }

    if (text[0] == '(' || (remaining > 5 && buffer_match(text, "calc(", 5)))
    {

        struct quantity result;

        evaluator->offset += (text[0] == '(') ? 1 : 5;
        evaluator->depth++;
        result = evaluatesum(evaluator);
        evaluator->depth--;

        if (!nextis(evaluator, ')'))
            evaluator->valid = 0;

        return result;

    }

    if ((remaining > 4 && (buffer_match(text, "min(", 4) || buffer_match(text, "max(", 4))) || (remaining > 6 && buffer_match(text, "clamp(", 6)))
    {

        unsigned int kind = (text[1] == 'i') ? 1 : (text[1] == 'a') ? 2 : 3;
        struct quantity result;

        evaluator->offset += (kind == 3) ? 6 : 4;
        evaluator->depth++;
        result = evaluatefunction(evaluator, kind);
        evaluator->depth--;

        return result;

    }

    number = parsenumber(text, remaining);

    if (!number.length)
    {

        evaluator->valid = 0;

        return quantity(0, 0, 0);

    }

    evaluator->offset += number.length;
    text += number.length;
    remaining -= number.length;

    for (unit = 0; unit < remaining && cstring_isalpha(text[unit]); unit++);

    evaluator->offset += unit;

    if (!unit && remaining && text[0] == '%')
    {

        evaluator->offset++;

        return quantity(0, number.milli, 1);

    }

    if (!unit)
        return quantity(number.milli, 0, 0);

    if (matchword(text, unit, "px"))
        return quantity(number.milli, 0, 1);

    if (matchword(text, unit, "em"))
        return quantity(number.milli * (int)evaluator->fontsize, 0, 1);

    if (matchword(text, unit, "rem"))
        return quantity(number.milli * (int)evaluator->context->rootsize, 0, 1);

    if (matchword(text, unit, "vw"))
        return quantity(number.milli / 100 * (int)evaluator->context->width, 0, 1);

    if (matchword(text, unit, "vh"))
        return quantity(number.milli / 100 * (int)evaluator->context->height, 0, 1);

    if (matchword(text, unit, "pt"))
        return quantity(number.milli * 4 / 3, 0, 1);

    evaluator->valid = 0;

    return quantity(0, 0, 0);

}

static struct quantity evaluateproduct(struct evaluator *evaluator)
{

    struct quantity result = evaluatevalue(evaluator);

    while (evaluator->valid)
    {

        struct quantity next;

        if (nextis(evaluator, '*'))
        {

            next = evaluatevalue(evaluator);

            if (result.length && next.length)
                evaluator->valid = 0;

            if (next.length)
            {

                result.percent = multiply(next.percent, result.milli);
                result.milli = multiply(next.milli, result.milli);

            }

            else
            {

                result.percent = multiply(result.percent, next.milli);
                result.milli = multiply(result.milli, next.milli);

            }

            result.length = result.length || next.length;

        }

        else if (nextis(evaluator, '/'))
        {

            next = evaluatevalue(evaluator);

            if (next.length)
                evaluator->valid = 0;

            result.percent = divide(result.percent, next.milli);
            result.milli = divide(result.milli, next.milli);

        }

        else
        {

            break;

        }

    }

    return result;

}

static struct quantity evaluatesum(struct evaluator *evaluator)
{

    struct quantity result = evaluateproduct(evaluator);

    while (evaluator->valid)
    {

        struct quantity next;
        int sign;

        if (nextis(evaluator, '+'))
            sign = 1;
        else if (nextis(evaluator, '-'))
            sign = -1;
        else
            break;

        next = evaluateproduct(evaluator);

        if (next.length != result.length && (next.milli || next.percent || result.milli || result.percent))
            evaluator->valid = 0;

        result.milli += sign * next.milli;
        result.percent += sign * next.percent;
        result.length = result.length || next.length;

    }

    return result;

}

static struct lengthvalue evaluatelength(struct context *context, char *text, unsigned int length, unsigned int fontsize)
{

    struct evaluator evaluator;
    struct quantity result;

    evaluator.context = context;
    evaluator.text = text;
    evaluator.offset = 0;
    evaluator.count = length;
    evaluator.fontsize = fontsize;
    evaluator.valid = 1;
    evaluator.depth = 0;

    result = evaluatevalue(&evaluator);

    skipblank(&evaluator);

    if (!evaluator.valid || evaluator.offset != length || (!result.length && result.milli))
        return lengthvalue(0, CSS_UNIT_PX, 0);

    if (result.percent)
    {

        struct lengthvalue value = lengthvalue(result.percent, CSS_UNIT_PERCENT, 1);

        value.length.offset = roundmilli(result.milli);

        return value;

    }

    return lengthvalue(roundmilli(result.milli), CSS_UNIT_PX, 1);

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

    if (matchword(text, length, "fit-content") || matchword(text, length, "max-content"))
        return lengthvalue(0, CSS_UNIT_FIT, 1);

    if (length > 4 && (buffer_match(text, "calc(", 5) || buffer_match(text, "min(", 4) || buffer_match(text, "max(", 4) || buffer_match(text, "clamp(", 6)))
        return evaluatelength(context, text, length, fontsize);

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
    to->offset = from->offset;

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
        return colorvalue(CSS_CURRENTCOLOR, 1);

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

    if (size && !matchword(text, length, "smaller") && !matchword(text, length, "larger"))
        context->style->keyword = 1;

    if (!size)
    {

        struct lengthvalue value = parselength(context, text, length, context->parent->fontsize);

        if (!value.valid || value.length.unit == CSS_UNIT_AUTO || value.length.unit == CSS_UNIT_NONE)
            return 0;

        size = (value.length.unit == CSS_UNIT_PERCENT) ? (unsigned int)roundmilli(value.length.value * (int)context->parent->fontsize / 100) : (unsigned int)value.length.value;

        /* only sizes relative to a keyword keep the smaller default of monospace text */
        if (value.length.unit != CSS_UNIT_PERCENT && !(length > 2 && matchword(text + length - 2, 2, "em") && text[length - 3] != 'r'))
            context->style->keyword = 0;

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
        unsigned int length = buffer_findbyte(text, spans[size].length, '/');
        struct number number = parsenumber(text, length);

        /* a bare number before the size is a weight */
        if (number.length && number.length == length)
            continue;

        if (cstring_isdigit(text[0]) || text[0] == '.' || fontkeyword(context, text, length))
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
        unsigned int next = size + 1;

        if (!setfontsize(context, text, slash))
            return;

        context->style->monospace = matchword(value + spans[count - 1].start, spans[count - 1].length, "monospace") && buffer_findbyte(value, cstring_length(value), ',') == cstring_length(value);

        if (slash == length && next < count && value[spans[next].start] == '/')
        {

            text = value + spans[next].start;
            length = spans[next].length;
            slash = 0;
            next++;

        }

        if (slash + 1 < length)
            setlineheight(context, text + slash + 1, length - slash - 1);
        else if (slash < length && next < count)
            setlineheight(context, value + spans[next].start, spans[next].length);

    }

}

static unsigned int parseflexalign(char *text, unsigned int length)
{

    if (matchword(text, length, "flex-start") || matchword(text, length, "start") || matchword(text, length, "self-start") || matchword(text, length, "left") || matchword(text, length, "baseline") || matchword(text, length, "first baseline") || matchword(text, length, "last baseline"))
        return CSS_FLEX_START;

    if (matchword(text, length, "flex-end") || matchword(text, length, "end") || matchword(text, length, "self-end") || matchword(text, length, "right"))
        return CSS_FLEX_END;

    if (matchword(text, length, "center"))
        return CSS_FLEX_CENTER;

    if (matchword(text, length, "stretch"))
        return CSS_FLEX_STRETCH;

    if (matchword(text, length, "space-between"))
        return CSS_FLEX_BETWEEN;

    if (matchword(text, length, "space-around"))
        return CSS_FLEX_AROUND;

    if (matchword(text, length, "space-evenly"))
        return CSS_FLEX_EVENLY;

    /* normal is stretch for align-items and flex-start for justify-content */
    if (matchword(text, length, "auto") || matchword(text, length, "normal"))
        return CSS_FLEX_AUTO;

    return 0xFF;

}

static unsigned int setbasis(struct context *context, char *text, unsigned int length)
{

    struct lengthvalue value;

    if (matchword(text, length, "auto") || matchword(text, length, "content"))
    {

        context->style->basis.unit = CSS_UNIT_AUTO;
        context->style->basis.value = 0;

        return 1;

    }

    value = parselength(context, text, length, context->style->fontsize);

    if (!value.valid || value.length.unit == CSS_UNIT_AUTO || value.length.value < 0)
        return 0;

    copylength(&context->style->basis, &value.length);

    return 1;

}

static void setflex(struct context *context, char *value)
{

    struct css_style *style = context->style;
    struct span spans[FONTTOKENS];
    unsigned int count = countspans(value, spans, FONTTOKENS);
    unsigned int numbers = 0;
    unsigned int based = 0;
    unsigned int i;

    if (count == 1 && matchword(value + spans[0].start, spans[0].length, "none"))
    {

        style->grow = 0;
        style->shrink = 0;
        style->basis.unit = CSS_UNIT_AUTO;

        return;

    }

    if (count == 1 && matchword(value + spans[0].start, spans[0].length, "auto"))
    {

        style->grow = 100;
        style->shrink = 100;
        style->basis.unit = CSS_UNIT_AUTO;

        return;

    }

    if (count == 1 && matchword(value + spans[0].start, spans[0].length, "initial"))
    {

        style->grow = 0;
        style->shrink = 100;
        style->basis.unit = CSS_UNIT_AUTO;

        return;

    }

    style->grow = 100;
    style->shrink = 100;

    for (i = 0; i < count; i++)
    {

        char *text = value + spans[i].start;
        struct number number = parsenumber(text, spans[i].length);

        if (number.length == spans[i].length && number.milli >= 0 && numbers < 2)
        {

            unsigned int factor = (number.milli > 655000) ? 65500 : number.milli / 10;

            if (numbers++)
                style->shrink = factor;
            else
                style->grow = factor;

        }

        else if (setbasis(context, text, spans[i].length))
        {

            based = 1;

        }

    }

    /* a lone number means a basis of zero, so the space is shared by grow factors alone */
    if (!based)
    {

        style->basis.unit = CSS_UNIT_PX;
        style->basis.value = 0;

    }

}

static void setflexflow(struct context *context, char *value)
{

    struct span spans[FONTTOKENS];
    unsigned int count = countspans(value, spans, FONTTOKENS);
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char *text = value + spans[i].start;
        unsigned int length = spans[i].length;

        if (matchword(text, length, "row") || matchword(text, length, "row-reverse"))
            context->style->column = 0;
        else if (matchword(text, length, "column") || matchword(text, length, "column-reverse"))
            context->style->column = 1;

        if (matchword(text, length, "row") || matchword(text, length, "column"))
            context->style->reverse = 0;
        else if (matchword(text, length, "row-reverse") || matchword(text, length, "column-reverse"))
            context->style->reverse = 1;
        else if (matchword(text, length, "nowrap"))
            context->style->wrap = 0;
        else if (matchword(text, length, "wrap") || matchword(text, length, "wrap-reverse"))
            context->style->wrap = 1;

    }

}

static unsigned int parsedisplay(char *text, unsigned int length)
{

    if (matchword(text, length, "block") || matchword(text, length, "flow-root") || matchword(text, length, "table-caption"))
        return CSS_DISPLAY_BLOCK;

    if (matchword(text, length, "grid"))
        return CSS_DISPLAY_GRID;

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
    unsigned int color = CSS_CURRENTCOLOR;
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
        copylength(length, &parsed.length);

}

static unsigned int isgradient(char *text, unsigned int length)
{

    if (length > 8 && buffer_match(text, "-webkit-", 8))
    {

        text += 8;
        length -= 8;

    }

    if (length > 10 && buffer_match(text, "repeating-", 10))
    {

        text += 10;
        length -= 10;

    }

    return (length > 16 && buffer_match(text, "linear-gradient(", 16)) || (length > 16 && buffer_match(text, "radial-gradient(", 16));

}

static int parsepercent(char *text, unsigned int length, int fallback)
{

    struct number number = parsenumber(text, length);

    if (matchword(text, length, "left") || matchword(text, length, "top"))
        return 0;

    if (matchword(text, length, "center"))
        return 500;

    if (matchword(text, length, "right") || matchword(text, length, "bottom"))
        return 1000;

    if (number.length && number.length + 1 == length && text[number.length] == '%')
        return number.milli / 100;

    if (number.length == length && !number.milli)
        return 0;

    return fallback;

}

static int parseangle(char *text, unsigned int length)
{

    struct number number = parsenumber(text, length);
    unsigned int horizontal = contains(text, length, "left") ? 270 : contains(text, length, "right") ? 90 : 0;
    unsigned int vertical = contains(text, length, "top") ? 1 : contains(text, length, "bottom") ? 2 : 0;

    if (number.length && matchword(text + number.length, length - number.length, "deg"))
        return roundmilli(number.milli);

    if (number.length && matchword(text + number.length, length - number.length, "turn"))
        return roundmilli(number.milli * 360);

    if (length < 3 || !buffer_match(text, "to ", 3))
        return -1;

    if (!vertical)
        return horizontal;

    if (!horizontal)
        return (vertical == 1) ? 0 : 180;

    if (vertical == 1)
        return (horizontal == 90) ? 45 : 315;

    return (horizontal == 90) ? 135 : 225;

}

static void parseshape(struct css_gradient *gradient, char *text)
{

    struct span spans[GRADIENTARGS];
    unsigned int count = countspans(text, spans, GRADIENTARGS);
    unsigned int at = count;
    unsigned int sizes = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char *token = text + spans[i].start;
        unsigned int length = spans[i].length;

        if (matchword(token, length, "at"))
        {

            at = i;

            break;

        }

        if (parsepercent(token, length, -1) >= 0 && sizes < 2)
        {

            if (sizes == 0)
                gradient->width = gradient->height = parsepercent(token, length, -1);
            else
                gradient->height = parsepercent(token, length, -1);

            sizes++;

        }

    }

    if (at + 1 < count)
        gradient->x = parsepercent(text + spans[at + 1].start, spans[at + 1].length, 500);

    if (at + 2 < count)
        gradient->y = parsepercent(text + spans[at + 2].start, spans[at + 2].length, 500);

}

static void addstop(struct css_gradient *gradient, unsigned int color, int position)
{

    if (gradient->count < CSS_STOPS)
    {

        gradient->stops[gradient->count].color = color;
        gradient->stops[gradient->count].position = position;
        gradient->count++;

    }

}

static unsigned int parsestop(struct context *context, struct css_gradient *gradient, char *text, unsigned int length)
{

    struct span spans[3];
    char token[EXPANDSIZE];
    struct colorvalue color;
    unsigned int count;
    unsigned int i;

    if (length >= EXPANDSIZE)
        return 0;

    buffer_copy(token, text, length);

    token[length] = '\0';
    count = countspans(token, spans, 3);
    color = (count) ? parsecolor(token + spans[0].start, spans[0].length) : colorvalue(0, 0);

    if (!color.valid)
        return 0;

    if (color.value == CSS_CURRENTCOLOR)
        color.value = context->style->color;

    if (count == 1)
        addstop(gradient, color.value, -1);

    for (i = 1; i < count; i++)
        addstop(gradient, color.value, parsepercent(token + spans[i].start, spans[i].length, -1));

    return 1;

}

static void spreadstops(struct css_gradient *gradient)
{

    unsigned int i;

    if (gradient->stops[0].position < 0)
        gradient->stops[0].position = 0;

    if (gradient->stops[gradient->count - 1].position < 0)
        gradient->stops[gradient->count - 1].position = 1000;

    for (i = 1; i < gradient->count; i++)
    {

        if (gradient->stops[i].position < 0)
        {

            unsigned int next = i;

            while (gradient->stops[next].position < 0)
                next++;

            gradient->stops[i].position = gradient->stops[i - 1].position + (gradient->stops[next].position - gradient->stops[i - 1].position) / (int)(next - i + 1);

        }

        if (gradient->stops[i].position < gradient->stops[i - 1].position)
            gradient->stops[i].position = gradient->stops[i - 1].position;

    }

}

static unsigned int parsegradient(struct context *context, char *text, unsigned int length)
{

    struct css_sheet *sheet = context->sheet;
    struct css_gradient *gradient;
    unsigned int open = buffer_findbyte(text, length, '(');
    unsigned int close = findclose(text, open, length);
    unsigned int start = open + 1;
    unsigned int first = 1;

    if (sheet->gradientcount >= CSS_GRADIENTS || close >= length)
        return 0;

    gradient = &sheet->gradients[sheet->gradientcount];
    gradient->radial = contains(text, open, "radial");
    gradient->count = 0;
    gradient->angle = 180;
    gradient->x = 500;
    gradient->y = 500;
    gradient->width = 707;
    gradient->height = 707;

    while (start < close)
    {

        unsigned int stop = findstop(text, start, close, ",");

        start = skipspace(text, start, stop);

        if (first && !gradient->radial && parseangle(text + start, stop - start) >= 0)
        {

            gradient->angle = parseangle(text + start, stop - start);

        }

        else if (!parsestop(context, gradient, text + start, stop - start))
        {

            if (!first || !gradient->radial)
                return 0;

            if (stop - start < EXPANDSIZE)
            {

                char shape[EXPANDSIZE];

                buffer_copy(shape, text + start, stop - start);

                shape[stop - start] = '\0';

                parseshape(gradient, shape);

            }

        }

        first = 0;
        start = stop + 1;

    }

    if (gradient->count < 2)
        return 0;

    spreadstops(gradient);

    return ++sheet->gradientcount;

}

static void setbackground(struct context *context, char *value, unsigned int image)
{

    struct span spans[8];
    unsigned int count = countspans(value, spans, 8);
    unsigned int color = 0;
    unsigned int gradient = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char *text = value + spans[i].start;
        struct colorvalue parsed = parsecolor(text, spans[i].length);

        if (!gradient && isgradient(text, spans[i].length))
            gradient = parsegradient(context, text, spans[i].length);
        else if (parsed.valid && !image)
            color = (parsed.value == CSS_CURRENTCOLOR) ? context->style->color : parsed.value;

    }

    if (!image)
        context->style->background = color;

    context->style->gradient = gradient;

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

static unsigned int isfraction(char *text, unsigned int length)
{

    if (length > 7 && buffer_match(text, "minmax(", 7))
        return length > 4 && buffer_match(text + length - 3, "fr)", 3);

    return length > 2 && buffer_match(text + length - 2, "fr", 2);

}

static int gaplength(struct context *context, char *text, unsigned int length)
{

    struct lengthvalue gap = parselength(context, text, length, context->style->fontsize);

    if (matchword(text, length, "normal"))
        return 0;

    return (gap.valid && gap.length.unit == CSS_UNIT_PX && gap.length.value > 0) ? gap.length.value : 0;

}

static void setcolumns(struct context *context, char *value)
{

    struct span spans[GRIDTRACKS];
    unsigned int count = countspans(value, spans, GRIDTRACKS);
    unsigned int columns = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char *text = value + spans[i].start;
        unsigned int length = spans[i].length;

        if (length > 7 && buffer_match(text, "repeat(", 7))
        {

            unsigned int comma = buffer_findbyte(text, length, ',');
            unsigned int start = comma + 1;

            while (start < length && text[start] == ' ')
                start++;

            if (comma < length && (buffer_match(text + 7, "auto-fill", 9) || buffer_match(text + 7, "auto-fit", 8)) && buffer_match(text + start, "minmax(", 7))
            {

                context->style->columns = CSS_COLUMNS_AUTOFILL;
                context->style->trackmin = gaplength(context, text + start + 7, buffer_findbyte(text + start + 7, length - start - 7, ','));

                return;

            }

            if (comma < length && isfraction(text + start, length - start - 1))
            {

                columns += cstring_read_value(text + 7, comma - 7, 10);

                continue;

            }

            columns = 0;

            break;

        }

        if (!isfraction(text, length))
        {

            columns = 0;

            break;

        }

        columns++;

    }

    context->style->columns = (columns > 32) ? 32 : columns;

}

static void setgap(struct context *context, char *value, unsigned int rows, unsigned int columns)
{

    struct span spans[2];
    unsigned int count = countspans(value, spans, 2);

    if (!count)
        return;

    if (rows)
        context->style->rowgap = gaplength(context, value + spans[0].start, spans[0].length);

    if (columns)
        context->style->columngap = gaplength(context, value + spans[(rows && count > 1) ? 1 : 0].start, spans[(rows && count > 1) ? 1 : 0].length);

}

static void copyproperty(struct css_style *style, struct css_style *from, unsigned int property)
{

    unsigned int side;

    switch (property)
    {

    case PROP_DISPLAY:
        style->display = from->display;
        style->flexbox = from->flexbox;

        break;

    case PROP_COLOR:
        style->color = from->color;

        break;

    case PROP_FILL:
        style->fill = from->fill;

        break;

    case PROP_BACKGROUND:
        style->background = from->background;
        style->gradient = from->gradient;

        break;

    case PROP_BACKGROUNDCOLOR:
        style->background = from->background;

        break;

    case PROP_BACKGROUNDIMAGE:
        style->gradient = from->gradient;

        break;

    case PROP_BORDERRADIUS:
        copylength(&style->radius, &from->radius);

        break;

    case PROP_TEXTTRANSFORM:
        style->transform = from->transform;

        break;

    case PROP_GRIDTEMPLATECOLUMNS:
        style->columns = from->columns;
        style->trackmin = from->trackmin;

        break;

    case PROP_GAP:
    case PROP_GRIDGAP:
        style->rowgap = from->rowgap;
        style->columngap = from->columngap;

        break;

    case PROP_ROWGAP:
        style->rowgap = from->rowgap;

        break;

    case PROP_COLUMNGAP:
        style->columngap = from->columngap;

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
    case PROP_OPACITY:
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

    case PROP_MAXHEIGHT:
        copylength(&style->maxheight, &from->maxheight);

        break;

    case PROP_BOXSIZING:
        style->borderbox = from->borderbox;

        break;

    case PROP_POSITION:
        style->position = from->position;

        break;

    case PROP_FONTFAMILY:
        style->monospace = from->monospace;

        break;

    case PROP_BORDERSPACING:
        style->spacing = from->spacing;

        break;

    case PROP_BORDERCOLLAPSE:
        style->collapse = from->collapse;

        break;

    case PROP_FLOAT:
        style->floating = from->floating;

        break;

    case PROP_CLEAR:
        style->clear = from->clear;

        break;

    case PROP_OVERFLOW:
    case PROP_OVERFLOWX:
    case PROP_OVERFLOWY:
        style->overflow = from->overflow;

        break;

    case PROP_FLEXDIRECTION:
        style->column = from->column;
        style->reverse = from->reverse;

        break;

    case PROP_ORDER:
        style->order = from->order;

        break;

    case PROP_JUSTIFYCONTENT:
        style->justify = from->justify;

        break;

    case PROP_ALIGNITEMS:
        style->alignitems = from->alignitems;

        break;

    case PROP_ALIGNSELF:
        style->alignself = from->alignself;

        break;

    case PROP_FLEXGROW:
    case PROP_FLEXSHRINK:
    case PROP_FLEXBASIS:
    case PROP_FLEX:
        style->grow = from->grow;
        style->shrink = from->shrink;
        copylength(&style->basis, &from->basis);

        break;

    case PROP_FLEXWRAP:
    case PROP_FLEXFLOW:
        style->wrap = from->wrap;
        style->column = from->column;
        style->reverse = from->reverse;

        break;

    default:
        for (side = 0; side < CSS_SIDES; side++)
        {

            if (property == PROP_MARGIN || property == PROP_MARGINTOP + side)
                copylength(&style->margin[side], &from->margin[side]);

            if (property == PROP_PADDING || property == PROP_PADDINGTOP + side)
                copylength(&style->padding[side], &from->padding[side]);

            if (property == PROP_TOP + side)
                copylength(&style->inset[side], &from->inset[side]);

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
    style->keyword = 1;
    style->liststyle = CSS_LIST_DISC;
    style->width.unit = CSS_UNIT_AUTO;
    style->height.unit = CSS_UNIT_AUTO;
    style->maxwidth.unit = CSS_UNIT_NONE;
    style->maxheight.unit = CSS_UNIT_NONE;
    style->basis.unit = CSS_UNIT_AUTO;
    style->shrink = 100;
    style->alignitems = CSS_FLEX_STRETCH;
    style->alignself = CSS_FLEX_AUTO;

    for (side = 0; side < CSS_SIDES; side++)
    {

        style->borderwidth[side] = 3;
        style->bordercolor[side] = CSS_CURRENTCOLOR;
        style->inset[side].unit = CSS_UNIT_AUTO;

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
        {

            style->display = parsed;
            style->flexbox = matchword(value, length, "flex") || matchword(value, length, "inline-flex");

        }

        break;

    case PROP_COLOR:
        color = parsecolor(value, length);

        if (color.valid)
            style->color = (color.value == CSS_CURRENTCOLOR) ? context->parent->color : color.value;

        break;

    case PROP_FILL:
        color = parsecolor(value, length);

        if (matchword(value, length, "none"))
            style->fill = 0x00FFFFFF;
        else if (color.valid)
            style->fill = (color.value) ? color.value : 0x00FFFFFF;

        break;

    case PROP_BACKGROUND:
        setbackground(context, value, 0);

        break;

    case PROP_BACKGROUNDIMAGE:
        setbackground(context, value, 1);

        break;

    case PROP_GRIDTEMPLATECOLUMNS:
        setcolumns(context, value);

        break;

    case PROP_GAP:
    case PROP_GRIDGAP:
        setgap(context, value, 1, 1);

        break;

    case PROP_ROWGAP:
        setgap(context, value, 1, 0);

        break;

    case PROP_COLUMNGAP:
        setgap(context, value, 0, 1);

        break;

    case PROP_TEXTTRANSFORM:
        if (matchword(value, length, "uppercase"))
            style->transform = CSS_TRANSFORM_UPPERCASE;
        else if (matchword(value, length, "lowercase"))
            style->transform = CSS_TRANSFORM_LOWERCASE;
        else if (matchword(value, length, "none") || matchword(value, length, "capitalize"))
            style->transform = CSS_TRANSFORM_NONE;

        break;

    case PROP_BORDERRADIUS:
        {

            struct span spans[CSS_SIDES];
            struct lengthvalue radius;

            if (!countspans(value, spans, CSS_SIDES))
                break;

            radius = parselength(context, value + spans[0].start, spans[0].length, style->fontsize);

            if (radius.valid && radius.length.unit <= CSS_UNIT_PERCENT && radius.length.value >= 0)
                copylength(&style->radius, &radius.length);

        }

        break;

    case PROP_BACKGROUNDCOLOR:
        color = parsecolor(value, length);

        if (color.valid)
            style->background = (color.value == CSS_CURRENTCOLOR) ? style->color : color.value;

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

    case PROP_OPACITY:
        if (parsenumber(value, length).length && !parsenumber(value, length).milli)
            style->hidden = 1;

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

    case PROP_MAXHEIGHT:
        setlength(context, &style->maxheight, value, 1);

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

    case PROP_FONTFAMILY:
        style->monospace = matchword(value, length, "monospace");

        break;

    case PROP_BORDERSPACING:
        {

            struct span spans[FONTTOKENS];
            struct lengthvalue spacing;

            if (!countspans(value, spans, FONTTOKENS))
                break;

            spacing = parselength(context, value + spans[0].start, spans[0].length, style->fontsize);

            if (spacing.valid && spacing.length.unit == CSS_UNIT_PX && spacing.length.value >= 0)
                style->spacing = (spacing.length.value > 255) ? 255 : spacing.length.value;

        }

        break;

    case PROP_BORDERCOLLAPSE:
        style->collapse = matchword(value, length, "collapse");

        break;

    case PROP_FLOAT:
        if (matchword(value, length, "none"))
            style->floating = CSS_FLOAT_NONE;
        else if (matchword(value, length, "left") || matchword(value, length, "inline-start"))
            style->floating = CSS_FLOAT_LEFT;
        else if (matchword(value, length, "right") || matchword(value, length, "inline-end"))
            style->floating = CSS_FLOAT_RIGHT;

        break;

    case PROP_CLEAR:
        if (matchword(value, length, "none"))
            style->clear = 0;
        else if (matchword(value, length, "left") || matchword(value, length, "inline-start"))
            style->clear = CSS_CLEAR_LEFT;
        else if (matchword(value, length, "right") || matchword(value, length, "inline-end"))
            style->clear = CSS_CLEAR_RIGHT;
        else if (matchword(value, length, "both") || matchword(value, length, "all"))
            style->clear = CSS_CLEAR_LEFT | CSS_CLEAR_RIGHT;

        break;

    case PROP_OVERFLOW:
    case PROP_OVERFLOWX:
    case PROP_OVERFLOWY:
        /* nothing scrolls inside a page, so every value but visible clips */
        if (contains(value, length, "hidden") || contains(value, length, "clip") || contains(value, length, "auto") || contains(value, length, "scroll"))
            style->overflow = 1;
        else if (matchword(value, length, "visible"))
            style->overflow = 0;

        break;

    case PROP_FLEXDIRECTION:
        style->column = matchword(value, length, "column") || matchword(value, length, "column-reverse");
        style->reverse = matchword(value, length, "row-reverse") || matchword(value, length, "column-reverse");

        break;

    case PROP_ORDER:
        {

            struct number number = parsenumber(value, length);

            if (number.length == length && number.milli >= -32000000 && number.milli <= 32000000)
                style->order = number.milli / 1000;

        }

        break;

    case PROP_JUSTIFYCONTENT:
        parsed = parseflexalign(value, length);

        if (parsed != 0xFF && parsed != CSS_FLEX_STRETCH)
            style->justify = (parsed == CSS_FLEX_AUTO) ? CSS_FLEX_START : parsed;

        break;

    case PROP_ALIGNITEMS:
        parsed = parseflexalign(value, length);

        if (parsed != 0xFF && parsed != CSS_FLEX_BETWEEN && parsed != CSS_FLEX_AROUND && parsed != CSS_FLEX_EVENLY)
            style->alignitems = (parsed == CSS_FLEX_AUTO) ? CSS_FLEX_STRETCH : parsed;

        break;

    case PROP_ALIGNSELF:
        parsed = parseflexalign(value, length);

        if (parsed != 0xFF && parsed != CSS_FLEX_BETWEEN && parsed != CSS_FLEX_AROUND && parsed != CSS_FLEX_EVENLY)
            style->alignself = parsed;

        break;

    case PROP_FLEXGROW:
    case PROP_FLEXSHRINK:
        {

            struct number number = parsenumber(value, length);

            if (number.length == length && number.milli >= 0)
            {

                unsigned int factor = (number.milli > 655000) ? 65500 : number.milli / 10;

                if (property == PROP_FLEXGROW)
                    style->grow = factor;
                else
                    style->shrink = factor;

            }

        }

        break;

    case PROP_FLEXBASIS:
        setbasis(context, value, length);

        break;

    case PROP_FLEX:
        setflex(context, value);

        break;

    case PROP_FLEXWRAP:
        style->wrap = matchword(value, length, "wrap") || matchword(value, length, "wrap-reverse");

        break;

    case PROP_FLEXFLOW:
        setflexflow(context, value);

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

        else if (property >= PROP_TOP && property < PROP_TOP + CSS_SIDES)
        {

            struct lengthvalue parsedlength = parselength(context, value, length, style->fontsize);

            if (parsedlength.valid && parsedlength.length.unit != CSS_UNIT_NONE)
                copylength(&style->inset[property - PROP_TOP], &parsedlength.length);

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

    if (property == PROP_FONTSIZE || property == PROP_FONT || property == PROP_FONTFAMILY)
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

static unsigned int matchcompound(struct matcher *matcher, unsigned int start, unsigned int end, unsigned int node);

static unsigned int matchlist(struct matcher *matcher, unsigned int start, unsigned int end, unsigned int node)
{

    unsigned int i;

    for (i = start; i < end; i += matcher->sheet->parts[i].name + 1)
    {

        if (matchcompound(matcher, i + 1, i + 1 + matcher->sheet->parts[i].name, node))
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
        unsigned int matched = (part->type == PART_IS) ? matchlist(matcher, i + 1, i + 1 + part->name, node) : matchpart(matcher, part, node);

        if (matched == part->negate)
            return 0;

        if (part->type == PART_IS)
            i += part->name;

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

static unsigned int collect(struct css_sheet *sheet, struct html_document *document, unsigned int node, unsigned int width, unsigned int scheme, struct match matches[MATCHES])
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

        if ((int)width < rule->minwidth || (int)width > rule->maxwidth || (rule->scheme && rule->scheme != scheme))
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
                unsigned int k;

                buffer_copy(&match, &matches[count], sizeof (struct match));

                for (k = count; k > j; k--)
                    buffer_copy(&matches[k], &matches[k - 1], sizeof (struct match));

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

static unsigned int isalttext(struct html_document *document, unsigned int node)
{

    char *src = html_getattribute(document, node, "src");
    char *alt = html_getattribute(document, node, "alt");

    /* an image without a source is drawn as its alternative text */
    return document->nodes[node].tag == HTML_TAG_IMG && (!src || !src[0]) && alt && alt[0];

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

    if (element->tag != HTML_TAG_FONT && !isalttext(document, node) && (value = html_getattribute(document, node, "width")))
        applyhint(context, PROP_WIDTH, value, "px");

    if (element->tag != HTML_TAG_FONT && !isalttext(document, node) && (value = html_getattribute(document, node, "height")))
        applyhint(context, PROP_HEIGHT, value, "px");

    if ((value = html_getattribute(document, node, "align")))
    {

        if (element->tag == HTML_TAG_TABLE || element->tag == HTML_TAG_IMG)
        {

            if (cstring_match(value, "center"))
                apply(context, PROP_MARGIN, "0 auto");
            else if (cstring_match(value, "left") || cstring_match(value, "right"))
                applyhint(context, PROP_FLOAT, value, "");

        }

        else
        {

            applyhint(context, PROP_TEXTALIGN, value, "");

        }

    }

    if (element->tag == HTML_TAG_BR && (value = html_getattribute(document, node, "clear")))
        applyhint(context, PROP_CLEAR, value, "");

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
    style->fill = parent->fill;
    style->fontsize = parent->fontsize;
    style->monospace = parent->monospace;
    style->keyword = parent->keyword;
    style->spacing = parent->spacing;
    style->collapse = parent->collapse;
    style->lineheight = parent->lineheight;
    style->lineheightfactor = parent->lineheightfactor;
    style->bold = parent->bold;
    style->italic = parent->italic;
    style->align = parent->align;
    style->whitespace = parent->whitespace;
    style->liststyle = parent->liststyle;
    style->decoration = parent->decoration;
    style->hidden = parent->hidden;
    style->transform = parent->transform;

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

        if (style->bordercolor[side] == CSS_CURRENTCOLOR)
            style->bordercolor[side] = style->color;

    }

}

static unsigned int isatomic(unsigned int tag)
{

    return tag == HTML_TAG_IMG || tag == HTML_TAG_INPUT || tag == HTML_TAG_SELECT || tag == HTML_TAG_TEXTAREA || tag == HTML_TAG_BUTTON;

}

static void computeelement(struct css_sheet *sheet, struct html_document *document, struct context *context, unsigned int node)
{

    struct match matches[MATCHES];
    unsigned int count = collect(sheet, document, node, context->width, context->scheme, matches);
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

        if (phase == PHASE_FONT && context->style->keyword && context->style->monospace != context->parent->monospace)
            context->style->fontsize = (context->style->monospace) ? context->style->fontsize * 13 / 16 : context->style->fontsize * 16 / 13;

    }

    sheet->declarationcount = declarationmark;

    if (context->style->display == CSS_DISPLAY_INLINE && isatomic(document->nodes[node].tag))
        context->style->display = CSS_DISPLAY_INLINEBLOCK;

    if (context->style->floating && (context->style->display == CSS_DISPLAY_INLINE || context->style->display == CSS_DISPLAY_INLINEBLOCK))
        context->style->display = CSS_DISPLAY_BLOCK;

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
    unsigned int scheme = parser->scheme;

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
            else if (cstring_match(name, "prefers-color-scheme") && valueend > valuestart)
            {

                unsigned int wanted = (lower(text[valuestart]) == 'd') ? CSS_SCHEME_DARK : CSS_SCHEME_LIGHT;

                if (scheme && scheme != wanted)
                    return 0;

                scheme = wanted;

            }
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

    parser->scheme = scheme;
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

        unsigned int scheme = parser->scheme;
        int minwidth = parser->minwidth;
        int maxwidth = parser->maxwidth;

        if (evaluatemedia(parser, text, nameend, stop))
            parseblock(parser, stop + 1, close);

        parser->scheme = scheme;
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

struct css_color css_parsecolor(char *text, unsigned int length)
{

    struct css_color result;
    struct colorvalue color;
    char lowered[64];
    unsigned int i;

    result.value = 0;
    result.valid = 0;

    if (length >= 64)
        return result;

    for (i = 0; i < length; i++)
        lowered[i] = (text[i] >= 'A' && text[i] <= 'Z') ? text[i] + 32 : text[i];

    color = parsecolor(lowered, length);
    result.value = color.value;
    result.valid = color.valid;

    return result;

}

void css_parse(struct css_sheet *sheet, char *text, unsigned int count, unsigned int origin)
{

    struct parser parser;

    parser.sheet = sheet;
    parser.text = text;
    parser.offset = 0;
    parser.origin = origin;
    parser.scheme = CSS_SCHEME_ANY;
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

void css_compute(struct css_sheet *sheet, struct html_document *document, struct css_style *styles, unsigned int width, unsigned int height, unsigned int scheme)
{

    struct css_style root;
    struct context context;
    unsigned int full = sheet->full;
    unsigned int poolmark = sheet->poolcount;
    unsigned int node;

    initstyle(&root);

    sheet->customtotal = 0;
    sheet->gradientcount = 0;
    context.sheet = sheet;
    context.document = document;
    context.rootsize = 16;
    context.width = width;
    context.height = height;
    context.scheme = scheme;

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
