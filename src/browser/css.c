#include <fudge.h>
#include "html.h"
#include "css.h"

#define PROPERTIES                      123
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
#define PROP_ZINDEX                     88
#define PROP_CONTENT                    89
#define PROP_BOXSHADOW                  90
#define PROP_BACKGROUNDCLIP             91
#define PROP_WEBKITBACKGROUNDCLIP       92
#define PROP_VERTICALALIGN              93
#define PROP_LETTERSPACING              94
#define PROP_TOPLEFTRADIUS              95
#define PROP_TOPRIGHTRADIUS             96
#define PROP_BOTTOMRIGHTRADIUS          97
#define PROP_BOTTOMLEFTRADIUS           98
#define PROP_INSET                      99
#define PROP_MARGININLINE               100
#define PROP_MARGINBLOCK                101
#define PROP_PADDINGINLINE              102
#define PROP_PADDINGBLOCK               103
#define PROP_INSETINLINE                104
#define PROP_INSETBLOCK                 105
#define PROP_BORDERINLINE               106
#define PROP_BORDERBLOCK                107
#define PROP_GRIDTEMPLATEROWS           108
#define PROP_GRIDTEMPLATEAREAS          109
#define PROP_GRIDAUTOROWS               110
#define PROP_GRIDCOLUMN                 111
#define PROP_GRIDROW                    112
#define PROP_GRIDCOLUMNSTART            113
#define PROP_GRIDCOLUMNEND              114
#define PROP_GRIDROWSTART               115
#define PROP_GRIDROWEND                 116
#define PROP_GRIDAREA                   117
#define PROP_WORDBREAK                  118
#define PROP_OVERFLOWWRAP               119
#define PROP_WORDWRAP                   120
#define PROP_ASPECTRATIO                121
#define PROP_TRANSFORM                  122
#define ALIASES                         30
#define PROP_CUSTOM                     255
#define POOL_EXACT                      0
#define POOL_LOWER                      1
#define POOL_RAW                        2
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
#define PART_HAS                        19
#define HAS_DESCENDANT                  0
#define HAS_CHILD                       1
#define HAS_NEXT                        2
#define HAS_LATER                       3
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
#define COLORS                          148
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
#define CONTENTSIZE                     128

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
    unsigned int pseudo;

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

struct channel
{

    int milli;
    unsigned int length;
    unsigned int percent;

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

struct content
{

    char text[CONTENTSIZE];
    unsigned int length;
    unsigned int valid;

};

struct context
{

    struct css_sheet *sheet;
    struct html_document *document;
    unsigned int node;
    struct css_style *style;
    struct css_style *parent;
    struct content *content;
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
    "top", "right", "bottom", "left", "opacity", "max-height", "font-family", "border-spacing", "border-collapse", "order", "z-index", "content", "box-shadow", "background-clip", "-webkit-background-clip", "vertical-align", "letter-spacing", "border-top-left-radius", "border-top-right-radius", "border-bottom-right-radius", "border-bottom-left-radius",
    "inset", "margin-inline", "margin-block", "padding-inline", "padding-block", "inset-inline", "inset-block", "border-inline", "border-block",
    "grid-template-rows", "grid-template-areas", "grid-auto-rows", "grid-column", "grid-row", "grid-column-start", "grid-column-end", "grid-row-start", "grid-row-end", "grid-area", "word-break", "overflow-wrap", "word-wrap", "aspect-ratio", "transform"
};
static char *aliases[ALIASES][2] = {
    {"margin-inline-start", "margin-left"}, {"margin-inline-end", "margin-right"}, {"margin-block-start", "margin-top"}, {"margin-block-end", "margin-bottom"},
    {"padding-inline-start", "padding-left"}, {"padding-inline-end", "padding-right"}, {"padding-block-start", "padding-top"}, {"padding-block-end", "padding-bottom"},
    {"inset-inline-start", "left"}, {"inset-inline-end", "right"}, {"inset-block-start", "top"}, {"inset-block-end", "bottom"},
    {"border-inline-start", "border-left"}, {"border-inline-end", "border-right"}, {"border-block-start", "border-top"}, {"border-block-end", "border-bottom"},
    {"border-inline-start-width", "border-left-width"}, {"border-inline-end-width", "border-right-width"},
    {"border-start-start-radius", "border-top-left-radius"}, {"border-start-end-radius", "border-top-right-radius"}, {"border-end-start-radius", "border-bottom-left-radius"}, {"border-end-end-radius", "border-bottom-right-radius"},
    {"inline-size", "width"}, {"block-size", "height"}, {"min-inline-size", "min-width"}, {"max-inline-size", "max-width"}, {"min-block-size", "min-height"}, {"max-block-size", "max-height"},
    {"grid-row-gap", "row-gap"}, {"grid-column-gap", "column-gap"}
};
static struct namedcolor colors[COLORS] = {
    {"aliceblue", 0xF0F8FF}, {"antiquewhite", 0xFAEBD7}, {"aqua", 0x00FFFF}, {"aquamarine", 0x7FFFD4}, {"azure", 0xF0FFFF}, {"beige", 0xF5F5DC},
    {"bisque", 0xFFE4C4}, {"black", 0x000000}, {"blanchedalmond", 0xFFEBCD}, {"blue", 0x0000FF}, {"blueviolet", 0x8A2BE2}, {"brown", 0xA52A2A},
    {"burlywood", 0xDEB887}, {"cadetblue", 0x5F9EA0}, {"chartreuse", 0x7FFF00}, {"chocolate", 0xD2691E}, {"coral", 0xFF7F50}, {"cornflowerblue", 0x6495ED},
    {"cornsilk", 0xFFF8DC}, {"crimson", 0xDC143C}, {"cyan", 0x00FFFF}, {"darkblue", 0x00008B}, {"darkcyan", 0x008B8B}, {"darkgoldenrod", 0xB8860B},
    {"darkgray", 0xA9A9A9}, {"darkgreen", 0x006400}, {"darkgrey", 0xA9A9A9}, {"darkkhaki", 0xBDB76B}, {"darkmagenta", 0x8B008B}, {"darkolivegreen", 0x556B2F},
    {"darkorange", 0xFF8C00}, {"darkorchid", 0x9932CC}, {"darkred", 0x8B0000}, {"darksalmon", 0xE9967A}, {"darkseagreen", 0x8FBC8F}, {"darkslateblue", 0x483D8B},
    {"darkslategray", 0x2F4F4F}, {"darkslategrey", 0x2F4F4F}, {"darkturquoise", 0x00CED1}, {"darkviolet", 0x9400D3}, {"deeppink", 0xFF1493}, {"deepskyblue", 0x00BFFF},
    {"dimgray", 0x696969}, {"dimgrey", 0x696969}, {"dodgerblue", 0x1E90FF}, {"firebrick", 0xB22222}, {"floralwhite", 0xFFFAF0}, {"forestgreen", 0x228B22},
    {"fuchsia", 0xFF00FF}, {"gainsboro", 0xDCDCDC}, {"ghostwhite", 0xF8F8FF}, {"gold", 0xFFD700}, {"goldenrod", 0xDAA520}, {"gray", 0x808080},
    {"green", 0x008000}, {"greenyellow", 0xADFF2F}, {"grey", 0x808080}, {"honeydew", 0xF0FFF0}, {"hotpink", 0xFF69B4}, {"indianred", 0xCD5C5C},
    {"indigo", 0x4B0082}, {"ivory", 0xFFFFF0}, {"khaki", 0xF0E68C}, {"lavender", 0xE6E6FA}, {"lavenderblush", 0xFFF0F5}, {"lawngreen", 0x7CFC00},
    {"lemonchiffon", 0xFFFACD}, {"lightblue", 0xADD8E6}, {"lightcoral", 0xF08080}, {"lightcyan", 0xE0FFFF}, {"lightgoldenrodyellow", 0xFAFAD2}, {"lightgray", 0xD3D3D3},
    {"lightgreen", 0x90EE90}, {"lightgrey", 0xD3D3D3}, {"lightpink", 0xFFB6C1}, {"lightsalmon", 0xFFA07A}, {"lightseagreen", 0x20B2AA}, {"lightskyblue", 0x87CEFA},
    {"lightslategray", 0x778899}, {"lightslategrey", 0x778899}, {"lightsteelblue", 0xB0C4DE}, {"lightyellow", 0xFFFFE0}, {"lime", 0x00FF00}, {"limegreen", 0x32CD32},
    {"linen", 0xFAF0E6}, {"magenta", 0xFF00FF}, {"maroon", 0x800000}, {"mediumaquamarine", 0x66CDAA}, {"mediumblue", 0x0000CD}, {"mediumorchid", 0xBA55D3},
    {"mediumpurple", 0x9370DB}, {"mediumseagreen", 0x3CB371}, {"mediumslateblue", 0x7B68EE}, {"mediumspringgreen", 0x00FA9A}, {"mediumturquoise", 0x48D1CC}, {"mediumvioletred", 0xC71585},
    {"midnightblue", 0x191970}, {"mintcream", 0xF5FFFA}, {"mistyrose", 0xFFE4E1}, {"moccasin", 0xFFE4B5}, {"navajowhite", 0xFFDEAD}, {"navy", 0x000080},
    {"oldlace", 0xFDF5E6}, {"olive", 0x808000}, {"olivedrab", 0x6B8E23}, {"orange", 0xFFA500}, {"orangered", 0xFF4500}, {"orchid", 0xDA70D6},
    {"palegoldenrod", 0xEEE8AA}, {"palegreen", 0x98FB98}, {"paleturquoise", 0xAFEEEE}, {"palevioletred", 0xDB7093}, {"papayawhip", 0xFFEFD5}, {"peachpuff", 0xFFDAB9},
    {"peru", 0xCD853F}, {"pink", 0xFFC0CB}, {"plum", 0xDDA0DD}, {"powderblue", 0xB0E0E6}, {"purple", 0x800080}, {"rebeccapurple", 0x663399},
    {"red", 0xFF0000}, {"rosybrown", 0xBC8F8F}, {"royalblue", 0x4169E1}, {"saddlebrown", 0x8B4513}, {"salmon", 0xFA8072}, {"sandybrown", 0xF4A460},
    {"seagreen", 0x2E8B57}, {"seashell", 0xFFF5EE}, {"sienna", 0xA0522D}, {"silver", 0xC0C0C0}, {"skyblue", 0x87CEEB}, {"slateblue", 0x6A5ACD},
    {"slategray", 0x708090}, {"slategrey", 0x708090}, {"snow", 0xFFFAFA}, {"springgreen", 0x00FF7F}, {"steelblue", 0x4682B4}, {"tan", 0xD2B48C},
    {"teal", 0x008080}, {"thistle", 0xD8BFD8}, {"tomato", 0xFF6347}, {"turquoise", 0x40E0D0}, {"violet", 0xEE82EE}, {"wheat", 0xF5DEB3},
    {"white", 0xFFFFFF}, {"whitesmoke", 0xF5F5F5}, {"yellow", 0xFFFF00}, {"yellowgreen", 0x9ACD32}
};
static char *agent =
    "html,address,blockquote,body,center,dd,details,dialog,div,dl,dt,fieldset,figcaption,figure,footer,form,h1,h2,h3,h4,h5,h6,header,hgroup,hr,legend,main,menu,nav,ol,p,pre,search,section,summary,ul,article,aside,caption,listing,plaintext,xmp,optgroup{display:block}"
    "head,script,style,title,meta,link,base,template,option,datalist,param,area,map,noscript,[hidden],input[type=hidden]{display:none}"
    "details:not([open])>:not(summary){display:none}"
    "li{display:list-item}"
    "table{display:table;border-spacing:2px}"
    "tr{display:table-row}"
    "thead,tbody,tfoot{display:table-row-group}"
    "thead,tbody,tfoot,tr{vertical-align:middle}"
    "td,th{display:table-cell;padding:1px;vertical-align:inherit}"
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
    "center{text-align:-webkit-center}"
    "small,sub,sup{font-size:smaller}"
    "sub{vertical-align:sub}"
    "sup{vertical-align:super}"
    "big{font-size:larger}"
    "mark{background-color:yellow}"
    "hr{border:1px inset gray;margin:.5em 0}"
    "fieldset{border:2px groove gray;padding:.35em .75em .625em;margin:0 2px}"
    "fieldset>legend{width:fit-content;padding:0 2px}"
    "button,input,select,textarea{font-size:13.333px}"
    "button,select,input[type=submit],input[type=button],input[type=reset]{box-sizing:border-box}"
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

static unsigned int getmonospace(char *value, unsigned int length)
{

    unsigned int start = length;
    unsigned int end = length;

    while (start && value[start - 1] != ',')
        start--;

    while (start < end && isspace(value[start]))
        start++;

    while (end > start && isspace(value[end - 1]))
        end--;

    if (!matchword(value + start, end - start, "monospace"))
        return 0;

    /* fonts are never downloaded, so a list falling back to monospace gets the mono face but only monospace alone gets its smaller size */
    return (start) ? CSS_MONOSPACE_FACE : CSS_MONOSPACE_GENERIC;

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

static unsigned int pooladd(struct css_sheet *sheet, char *text, unsigned int length, unsigned int mode)
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

        c = (mode == POOL_LOWER) ? lower(text[i]) : text[i];

        /* raw text keeps its escapes, generated content decodes them itself */
        if (c == '\\' && i + 1 < length && mode != POOL_RAW)
            c = text[++i];

        i++;

        if (mode != POOL_EXACT && isspace(c))
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

    /* text runs left to right and top to bottom, so every logical side is one physical side */
    for (i = 0; i < ALIASES; i++)
    {

        if (matchword(text, length, aliases[i][0]))
            return findproperty(aliases[i][1], cstring_length(aliases[i][1]));

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
    sheet->declarations[sheet->declarationcount].name = (property == PROP_CUSTOM) ? pooladd(sheet, name, namelength, POOL_LOWER) : 0;
    sheet->declarations[sheet->declarationcount].value = pooladd(sheet, text + start, (end > start) ? end - start : 0, (property == PROP_CONTENT || property == PROP_CUSTOM) ? POOL_RAW : POOL_LOWER);

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

static unsigned int findcompound(struct css_part *parts, unsigned int first, unsigned int end)
{

    unsigned int start = first;
    unsigned int i;

    /* the last compound starts at the last combinator outside :is, :not and :has, whose own parts can have combinators too */
    for (i = first; i < end; i += (parts[i].type == PART_IS || parts[i].type == PART_HAS) ? parts[i].name + 1 : 1)
    {

        if (parts[i].combinator != COMBINATOR_NONE)
            start = i;

    }

    return start;

}

static unsigned int parsesimple(struct selectorparser *selectorparser, unsigned int offset, unsigned int end, unsigned int combinator);

static unsigned int parsealternative(struct selectorparser *selectorparser, unsigned int start, unsigned int end)
{

    struct css_sheet *sheet = selectorparser->sheet;
    unsigned int alternative = sheet->partcount;
    unsigned int offset = skipspace(selectorparser->text, start, end);
    unsigned int combinator = COMBINATOR_NONE;

    while (end > offset && isspace(selectorparser->text[end - 1]))
        end--;

    if (offset >= end || !addpart(sheet, PART_ALTERNATIVE, COMBINATOR_NONE))
        return 0;

    while (offset < end)
    {

        char c = selectorparser->text[offset];

        if (isspace(c) || c == '>' || c == '+' || c == '~')
        {

            offset = skipspace(selectorparser->text, offset, end);

            if (offset < end && (selectorparser->text[offset] == '>' || selectorparser->text[offset] == '+' || selectorparser->text[offset] == '~'))
            {

                c = selectorparser->text[offset];
                combinator = (c == '>') ? COMBINATOR_CHILD : (c == '+') ? COMBINATOR_ADJACENT : COMBINATOR_SIBLING;
                offset = skipspace(selectorparser->text, offset + 1, end);

            }

            else
            {

                combinator = COMBINATOR_DESCENDANT;

            }

            if (sheet->partcount == alternative + 1)
                return 0;

            continue;

        }

        offset = parsesimple(selectorparser, offset, end, combinator);

        if (!offset)
            return 0;

        combinator = COMBINATOR_NONE;

    }

    if (combinator != COMBINATOR_NONE)
        return 0;

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

        if (cstring_match(name, "has"))
        {

            unsigned int list = sheet->partcount;
            unsigned int inner = skipspace(text, stop + 1, close);
            unsigned int relation = (inner >= close) ? HAS_DESCENDANT : (text[inner] == '>') ? HAS_CHILD : (text[inner] == '+') ? HAS_NEXT : (text[inner] == '~') ? HAS_LATER : HAS_DESCENDANT;

            if (!parselist(selectorparser, (relation == HAS_DESCENDANT) ? stop + 1 : inner + 1, close, combinator, 0, 1))
                return 0;

            sheet->parts[list].type = PART_HAS;
            sheet->parts[list].value = relation;

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

    part->name = pooladd(sheet, text + start, stop - start, POOL_LOWER);
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
            part->value = pooladd(sheet, text + start + 1, (stop > start + 1) ? stop - start - 2 : 0, POOL_EXACT);

        }

        else
        {

            stop = readident(text, start, close);
            part->value = pooladd(sheet, text + start, stop - start, POOL_EXACT);

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

        part->name = pooladd(sheet, text + offset + 1, stop - offset - 1, POOL_EXACT);
        selectorparser->specificity += (c == '#') ? 0x10000 : 0x100;

        return (part->name) ? stop : 0;

    }

    if (isidentchar(c))
    {

        stop = readident(text, offset, end);
        part = addpart(sheet, PART_TAG, combinator);

        if (!part)
            return 0;

        part->name = pooladd(sheet, text + offset, stop - offset, POOL_LOWER);
        selectorparser->specificity += 1;

        return (part->name) ? stop : 0;

    }

    return 0;

}

static void setkey(struct css_sheet *sheet, struct css_selector *selector)
{

    unsigned int start = findcompound(sheet->parts, selector->part, selector->part + selector->count);
    unsigned int i;

    selector->keytype = KEY_NONE;

    for (i = start; i < selector->part + selector->count; i++)
    {

        struct css_part *part = &sheet->parts[i];

        if (part->type == PART_IS || part->type == PART_HAS)
            i += part->name;

        if (part->negate || part->type == PART_IS || part->type == PART_HAS)
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

static unsigned int parsepseudoelement(struct selectorparser *selectorparser, unsigned int offset, unsigned int end)
{

    char *text = selectorparser->text;
    unsigned int start = (offset + 1 < end && text[offset + 1] == ':') ? offset + 2 : offset + 1;
    unsigned int stop = readident(text, start, end);

    if (matchword(text + start, stop - start, "before"))
        selectorparser->pseudo = CSS_PSEUDO_BEFORE;
    else if (matchword(text + start, stop - start, "after"))
        selectorparser->pseudo = CSS_PSEUDO_AFTER;
    else
        return 0;

    selectorparser->specificity += 1;

    return stop;

}

static unsigned int ispseudoelement(char *text, unsigned int offset, unsigned int end)
{

    unsigned int start = (offset + 1 < end && text[offset + 1] == ':') ? offset + 2 : offset + 1;
    unsigned int stop = readident(text, start, end);

    if (text[offset] != ':')
        return 0;

    return matchword(text + start, stop - start, "before") || matchword(text + start, stop - start, "after");

}

static unsigned int isnever(struct css_sheet *sheet, unsigned int start, unsigned int end)
{

    unsigned int i = start;

    /* only a part of the selector itself rules it out, inside :not or :is it may be the part that matches */
    while (i < end)
    {

        struct css_part *part = &sheet->parts[i];

        if (part->type == PART_NEVER)
            return 1;

        i += (part->type == PART_IS || part->type == PART_HAS) ? part->name + 1 : 1;

    }

    return 0;

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
    selectorparser.pseudo = CSS_PSEUDO_NONE;

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

        /* a pseudo element ends the selector, a bare one stands for any element */
        if (ispseudoelement(text, offset, end))
        {

            if (combinator != COMBINATOR_NONE && !addpart(sheet, PART_UNIVERSAL, combinator))
                break;

            offset = parsepseudoelement(&selectorparser, offset, end);
            combinator = COMBINATOR_NONE;

            break;

        }

        offset = parsesimple(&selectorparser, offset, end, combinator);

        if (!offset)
            break;

        combinator = COMBINATOR_NONE;

    }

    if (sheet->selectorcount >= CSS_SELECTORS)
        sheet->full = 1;

    /* a selector that can never match is not kept, so a sheet full of hover and focus rules still fits */
    if (!offset || offset < end || combinator != COMBINATOR_NONE || sheet->selectorcount >= CSS_SELECTORS || isnever(sheet, partstart, sheet->partcount))
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
    selector->pseudo = selectorparser.pseudo;

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

    if (matchword(text, unit, "rem") && evaluator->context)
        return quantity(number.milli * (int)evaluator->context->rootsize, 0, 1);

    if (matchword(text, unit, "vw") && evaluator->context)
        return quantity(number.milli / 100 * (int)evaluator->context->width, 0, 1);

    if (matchword(text, unit, "vh") && evaluator->context)
        return quantity(number.milli / 100 * (int)evaluator->context->height, 0, 1);

    if (matchword(text, unit, "pt"))
        return quantity(number.milli * 4 / 3, 0, 1);

    /* angles only turn up as hues, which are counted in degrees anyway */
    if (matchword(text, unit, "deg"))
        return quantity(number.milli, 0, 0);

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

static struct channel readchannel(char *text, unsigned int length)
{

    struct channel channel;
    struct number number;

    channel.milli = 0;
    channel.length = 0;
    channel.percent = 0;

    /* a calculated channel has no element to measure against, so only plain numbers and percentages work */
    if (length > 4 && (buffer_match(text, "calc(", 5) || buffer_match(text, "min(", 4) || buffer_match(text, "max(", 4) || buffer_match(text, "clamp(", 6)))
    {

        unsigned int close = findclose(text, buffer_findbyte(text, length, '('), length);
        struct evaluator evaluator;
        struct quantity result;

        evaluator.context = 0;
        evaluator.text = text;
        evaluator.offset = 0;
        evaluator.count = (close < length) ? close + 1 : length;
        evaluator.fontsize = 16;
        evaluator.valid = 1;
        evaluator.depth = 0;
        result = evaluatevalue(&evaluator);

        if (!evaluator.valid || (result.length && (result.milli || !result.percent)))
            return channel;

        channel.milli = (result.length) ? result.percent : result.milli;
        channel.percent = result.length;
        channel.length = evaluator.count;

        return channel;

    }

    number = parsenumber(text, length);
    channel.milli = number.milli;
    channel.length = number.length;

    if (number.length && number.length < length && text[number.length] == '%')
    {

        channel.percent = 1;
        channel.length++;

    }

    return channel;

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

        struct channel channel;

        while (i < length && (isspace(text[i]) || text[i] == ',' || text[i] == '/'))
            i++;

        channel = readchannel(text + i, length - i);

        if (!channel.length)
            break;

        i += channel.length;

        if (channel.percent)
            channels[count] = clampbyte(channel.milli * 255 / 100000);
        else
            channels[count] = (count == 3) ? clampbyte(channel.milli * 255 / 1000) : clampbyte(roundmilli(channel.milli));

        count++;

    }

    if (count < 3)
        return colorvalue(0, 0);

    return colorvalue((channels[3] << 24) | (channels[0] << 16) | (channels[1] << 8) | channels[2], 1);

}

static int huechannel(int hue, int chroma)
{

    int sector = (hue * 1000 / 60) % 2000;

    return chroma * (1000 - ((sector > 1000) ? sector - 1000 : 1000 - sector)) / 1000;

}

static struct colorvalue parsehsl(char *text, unsigned int length)
{

    int values[4];
    unsigned int count = 0;
    unsigned int i = 0;
    int chroma;
    int second;
    int light;
    int r;
    int g;
    int b;

    values[3] = 1000;

    while (i < length && text[i] != '(')
        i++;

    i++;

    while (i < length && count < 4)
    {

        struct channel channel;

        while (i < length && (isspace(text[i]) || text[i] == ',' || text[i] == '/'))
            i++;

        channel = readchannel(text + i, length - i);

        if (!channel.length)
            break;

        i += channel.length;

        /* saturation and lightness are percentages even without the sign */
        values[count] = (channel.percent || count == 1 || count == 2) ? channel.milli / 100 : channel.milli;

        /* units after the hue, deg is the only one in use */
        while (i < length && cstring_isalpha(text[i]))
            i++;

        count++;

    }

    if (count < 3)
        return colorvalue(0, 0);

    values[0] = ((values[0] / 1000) % 360 + 360) % 360;
    values[1] = (values[1] < 0) ? 0 : (values[1] > 1000) ? 1000 : values[1];
    values[2] = (values[2] < 0) ? 0 : (values[2] > 1000) ? 1000 : values[2];
    chroma = (1000 - ((values[2] * 2 > 1000) ? values[2] * 2 - 1000 : 1000 - values[2] * 2)) * values[1] / 1000;
    second = huechannel(values[0], chroma);
    light = values[2] - chroma / 2;
    r = (values[0] < 60 || values[0] >= 300) ? chroma : (values[0] < 120 || values[0] >= 240) ? second : 0;
    g = (values[0] >= 240) ? 0 : (values[0] >= 60 && values[0] < 180) ? chroma : second;
    b = (values[0] < 120) ? 0 : (values[0] >= 180 && values[0] < 300) ? chroma : second;

    return colorvalue((clampbyte(values[3] * 255 / 1000) << 24) | (clampbyte((r + light) * 255 / 1000) << 16) | (clampbyte((g + light) * 255 / 1000) << 8) | clampbyte((b + light) * 255 / 1000), 1);

}

static struct colorvalue parsecolor(char *text, unsigned int length);

static struct colorvalue parsemixpart(char *text, unsigned int start, unsigned int end, int *percent)
{

    unsigned int stop;

    start = skipspace(text, start, end);

    while (end > start && isspace(text[end - 1]))
        end--;

    stop = end;

    /* an optional percentage follows the color */
    if (stop > start && text[stop - 1] == '%')
    {

        while (stop > start && !isspace(text[stop - 1]))
            stop--;

        *percent = parsenumber(text + stop, end - stop).milli / 1000;

        while (stop > start && isspace(text[stop - 1]))
            stop--;

    }

    return parsecolor(text + start, stop - start);

}

static unsigned int mixchannel(unsigned int a, unsigned int b, int shift, int weight)
{

    return ((((a >> shift) & 0xFF) * weight + ((b >> shift) & 0xFF) * (100 - weight)) / 100) << shift;

}

static struct colorvalue parsemix(char *text, unsigned int length)
{

    unsigned int open = buffer_findbyte(text, length, '(');
    unsigned int close = findclose(text, open, length);
    unsigned int first = findstop(text, open + 1, close, ",");
    unsigned int second = findstop(text, first + 1, close, ",");
    int p1 = -1;
    int p2 = -1;
    struct colorvalue a;
    struct colorvalue b;
    int weight;

    if (close >= length || second >= close)
        return colorvalue(0, 0);

    /* every color space is mixed as srgb */
    a = parsemixpart(text, first + 1, second, &p1);
    b = parsemixpart(text, second + 1, close, &p2);

    if (!a.valid || !b.valid || a.value == CSS_CURRENTCOLOR || b.value == CSS_CURRENTCOLOR)
        return colorvalue(0, 0);

    weight = (p1 >= 0 && p2 >= 0 && p1 + p2 > 0) ? p1 * 100 / (p1 + p2) : (p1 >= 0) ? p1 : (p2 >= 0) ? 100 - p2 : 50;
    weight = (weight < 0) ? 0 : (weight > 100) ? 100 : weight;

    return colorvalue(mixchannel(a.value, b.value, 24, weight) | mixchannel(a.value, b.value, 16, weight) | mixchannel(a.value, b.value, 8, weight) | mixchannel(a.value, b.value, 0, weight), 1);

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

    if (length > 4 && (buffer_match(text, "hsl(", 4) || buffer_match(text, "hsla(", 5)))
        return parsehsl(text, length);

    if (length > 10 && buffer_match(text, "color-mix(", 10))
        return parsemix(text, length);

    /* pages are drawn light, so the light half is taken */
    if (length > 11 && buffer_match(text, "light-dark(", 11))
        return parsecolor(text + 11, findstop(text, 11, length, ",") - 11);

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

        if (slash == length && next < count && value[spans[next].start] == '/')
        {

            text = value + spans[next].start;
            length = spans[next].length;
            slash = 0;
            next++;

        }

        if (slash + 1 < length)
        {

            setlineheight(context, text + slash + 1, length - slash - 1);

        }

        else if (slash < length && next < count)
        {

            setlineheight(context, value + spans[next].start, spans[next].length);

            next++;

        }

        context->style->monospace = (next < count) ? getmonospace(value + spans[next].start, spans[count - 1].start + spans[count - 1].length - spans[next].start) : 0;

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

    if (matchword(text, length, "auto") || matchword(text, length, "content") || matchword(text, length, "max-content") || matchword(text, length, "fit-content"))
    {

        context->style->basis.unit = CSS_UNIT_AUTO;
        context->style->basis.value = 0;

        return 1;

    }

    if (matchword(text, length, "min-content"))
    {

        context->style->basis.unit = CSS_UNIT_MINCONTENT;
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

static int gaplength(struct context *context, char *text, unsigned int length)
{

    struct lengthvalue gap = parselength(context, text, length, context->style->fontsize);

    if (matchword(text, length, "normal"))
        return 0;

    return (gap.valid && gap.length.unit == CSS_UNIT_PX && gap.length.value > 0) ? gap.length.value : 0;

}

struct tracksize
{

    unsigned int kind;
    int value;
    unsigned int valid;

};

struct linepart
{

    unsigned int start;
    unsigned int end;

};

static unsigned int hashname(char *text, unsigned int length)
{

    unsigned int hash = 2166136261u;
    unsigned int i;

    for (i = 0; i < length; i++)
        hash = (hash ^ (unsigned char)text[i]) * 16777619u;

    return (hash) ? hash : 1;

}

static struct css_grid *getgrid(struct context *context)
{

    struct css_sheet *sheet = context->sheet;
    struct css_grid *grid;

    if (context->style->grid)
        return &sheet->grids[context->style->grid - 1];

    if (sheet->gridcount >= CSS_GRIDS)
        return 0;

    grid = &sheet->grids[sheet->gridcount++];

    buffer_clear(grid, sizeof (struct css_grid));

    context->style->grid = sheet->gridcount;

    return grid;

}

static struct tracksize parsetracksize(struct context *context, char *text, unsigned int length)
{

    struct tracksize size;
    struct lengthvalue parsed;

    size.kind = CSS_TRACK_AUTO;
    size.value = 0;
    size.valid = 1;

    if (matchword(text, length, "auto"))
        return size;

    size.kind = (matchword(text, length, "min-content")) ? CSS_TRACK_MINCONTENT : (matchword(text, length, "max-content")) ? CSS_TRACK_MAXCONTENT : CSS_TRACK_AUTO;

    if (size.kind != CSS_TRACK_AUTO)
        return size;

    if (length > 2 && text[length - 2] == 'f' && text[length - 1] == 'r')
    {

        struct number number = parsenumber(text, length - 2);

        size.kind = CSS_TRACK_FR;
        size.value = number.milli;
        size.valid = number.length == length - 2;

        return size;

    }

    parsed = parselength(context, text, length, context->style->fontsize);
    size.kind = (parsed.length.unit == CSS_UNIT_PERCENT) ? CSS_TRACK_PERCENT : CSS_TRACK_PX;
    size.value = parsed.length.value;
    size.valid = parsed.valid && parsed.length.unit <= CSS_UNIT_PERCENT;

    return size;

}

static struct css_track parsetrack(struct context *context, char *text, unsigned int length)
{

    struct css_track track;
    struct tracksize low;
    struct tracksize high;

    if (length > 7 && (buffer_match(text, "minmax(", 7) || buffer_match(text, "fit-content(", 12)))
    {

        unsigned int open = buffer_findbyte(text, length, '(');
        unsigned int close = findclose(text, open, length);
        unsigned int comma = findstop(text, open + 1, close, ",");
        unsigned int start = skipspace(text, comma + 1, close);

        /* fit-content grows with its content up to the size given */
        if (text[0] == 'f')
        {

            low.kind = CSS_TRACK_AUTO;
            low.value = 0;
            low.valid = 1;
            high = parsetracksize(context, text + skipspace(text, open + 1, close), close - skipspace(text, open + 1, close));

        }

        else
        {

            low = parsetracksize(context, text + skipspace(text, open + 1, comma), comma - skipspace(text, open + 1, comma));
            high = parsetracksize(context, text + start, close - start);

        }

    }

    else
    {

        low = parsetracksize(context, text, length);
        high = low;

        if (low.kind == CSS_TRACK_FR)
            low.kind = CSS_TRACK_AUTO;

    }

    track.minkind = (low.valid) ? low.kind : CSS_TRACK_AUTO;
    track.minvalue = low.value;
    track.maxkind = (high.valid) ? high.kind : CSS_TRACK_AUTO;
    track.maxvalue = high.value;

    return track;

}

static unsigned int addtracks(struct context *context, struct css_grid *grid, struct css_track *tracks, unsigned int count, char *value, unsigned int columns)
{

    struct span spans[CSS_TRACKS * 2];
    unsigned int total = countspans(value, spans, CSS_TRACKS * 2);
    unsigned int naming = 0;
    unsigned int i;

    for (i = 0; i < total; i++)
    {

        char *text = value + spans[i].start;
        unsigned int length = spans[i].length;

        /* line names are not used for placing, so they are skipped */
        if (text[0] == '[' || naming)
        {

            naming = text[length - 1] != ']';

            continue;

        }

        if (length > 7 && buffer_match(text, "repeat(", 7))
        {

            unsigned int close = findclose(text, 6, length);
            unsigned int comma = findstop(text, 7, close, ",");
            unsigned int start = skipspace(text, comma + 1, close);
            char inner[VALUESIZE];
            unsigned int times;

            if (comma >= close || close - start >= VALUESIZE)
                continue;

            buffer_copy(inner, text + start, close - start);

            inner[close - start] = '\0';

            if (columns && (buffer_match(text + 7, "auto-fill", 9) || buffer_match(text + 7, "auto-fit", 8)))
            {

                grid->repeat = (text[12] == 'f' && text[13] == 'i' && text[14] == 't') ? CSS_REPEAT_FIT : CSS_REPEAT_FILL;
                tracks[0] = parsetrack(context, inner, close - start);

                return 1;

            }

            for (times = cstring_read_value(text + 7, comma - 7, 10); times && count < CSS_TRACKS; times--)
                count = addtracks(context, grid, tracks, count, inner, 0);

            continue;

        }

        if (count < CSS_TRACKS)
            tracks[count++] = parsetrack(context, text, length);

    }

    return count;

}

static void settracks(struct context *context, char *value, unsigned int columns)
{

    struct css_grid *grid = getgrid(context);

    if (!grid)
        return;

    if (columns)
    {

        grid->repeat = CSS_REPEAT_NONE;
        grid->columncount = (matchword(value, cstring_length(value), "none")) ? 0 : addtracks(context, grid, grid->columns, 0, value, 1);

    }

    else
    {

        grid->rowcount = (matchword(value, cstring_length(value), "none")) ? 0 : addtracks(context, grid, grid->rows, 0, value, 0);

    }

}

static void setautorows(struct context *context, char *value)
{

    struct css_grid *grid = getgrid(context);
    struct span spans[1];

    if (grid && countspans(value, spans, 1))
        grid->autorow = parsetrack(context, value + spans[0].start, spans[0].length);

}

static void setareas(struct context *context, char *value)
{

    struct css_grid *grid = getgrid(context);
    unsigned int i = 0;

    if (!grid)
        return;

    grid->arearows = 0;
    grid->areacolumns = 0;

    /* every quoted string is a row of names, a dot leaves its cell unnamed */
    while (value[i])
    {

        char quote = value[i++];
        unsigned int column = 0;

        if (quote != '"' && quote != '\'')
            continue;

        while (value[i] && value[i] != quote)
        {

            unsigned int start = i;

            while (value[i] && value[i] != quote && !isspace(value[i]))
                i++;

            if (i > start && grid->arearows < CSS_TRACKS && column < CSS_TRACKS)
                grid->areas[grid->arearows][column++] = (value[start] == '.') ? 0 : hashname(value + start, i - start);

            while (value[i] && isspace(value[i]))
                i++;

        }

        if (value[i])
            i++;

        if (grid->arearows < CSS_TRACKS)
            grid->arearows++;

        grid->areacolumns = (column > grid->areacolumns) ? column : grid->areacolumns;

    }

}

static struct css_gridline parseline(char *text, unsigned int length)
{

    struct css_gridline line;
    struct number number;

    line.value = 0;
    line.span = 0;

    if (length > 4 && buffer_match(text, "span", 4) && isspace(text[4]))
    {

        unsigned int start = skipspace(text, 4, length);

        line.span = (start < length) ? cstring_read_value(text + start, length - start, 10) : 1;

        return line;

    }

    number = parsenumber(text, length);

    if (number.length == length)
        line.value = number.milli / 1000;

    return line;

}

static struct linepart nextpart(char *value, unsigned int offset, unsigned int length)
{

    struct linepart part;

    part.start = skipspace(value, offset, length);
    part.end = findstop(value, part.start, length, "/");

    while (part.end > part.start && isspace(value[part.end - 1]))
        part.end--;

    return part;

}

static void setlines(struct context *context, char *value, unsigned int first, unsigned int second)
{

    unsigned int length = cstring_length(value);
    struct linepart start = nextpart(value, 0, length);
    unsigned int slash = findstop(value, start.start, length, "/");
    struct linepart end;

    context->style->gridlines[first] = parseline(value + start.start, start.end - start.start);
    context->style->gridlines[second].value = 0;
    context->style->gridlines[second].span = 0;

    if (slash >= length)
        return;

    end = nextpart(value, slash + 1, length);
    context->style->gridlines[second] = parseline(value + end.start, end.end - end.start);

}

static void setarea(struct context *context, char *value)
{

    unsigned int length = cstring_length(value);
    struct linepart part = nextpart(value, 0, length);
    unsigned int index;

    context->style->gridarea = 0;

    for (index = 0; index < CSS_GRIDLINES; index++)
    {

        context->style->gridlines[index].value = 0;
        context->style->gridlines[index].span = 0;

    }

    /* a single name places the item in the area of that name */
    if (part.end > part.start && cstring_isalpha(value[part.start]) && !buffer_match(value + part.start, "span", 4) && findstop(value, 0, length, "/") >= length)
    {

        context->style->gridarea = hashname(value + part.start, part.end - part.start);

        return;

    }

    for (index = 0; index < CSS_GRIDLINES && part.start < length; index++)
    {

        context->style->gridlines[index] = parseline(value + part.start, part.end - part.start);
        part = nextpart(value, findstop(value, part.start, length, "/") + 1, length);

    }

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
        copylength(&style->radius[CSS_TOPLEFT], &from->radius[CSS_TOPLEFT]);
        copylength(&style->radius[CSS_TOPRIGHT], &from->radius[CSS_TOPRIGHT]);
        copylength(&style->radius[CSS_BOTTOMRIGHT], &from->radius[CSS_BOTTOMRIGHT]);
        copylength(&style->radius[CSS_BOTTOMLEFT], &from->radius[CSS_BOTTOMLEFT]);

        break;

    case PROP_TOPLEFTRADIUS:
    case PROP_TOPRIGHTRADIUS:
    case PROP_BOTTOMRIGHTRADIUS:
    case PROP_BOTTOMLEFTRADIUS:
        copylength(&style->radius[property - PROP_TOPLEFTRADIUS], &from->radius[property - PROP_TOPLEFTRADIUS]);

        break;

    case PROP_TEXTTRANSFORM:
        style->transform = from->transform;

        break;

    case PROP_GRIDTEMPLATECOLUMNS:
        style->grid = from->grid;

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
        style->hidden = from->hidden;

        break;

    case PROP_OPACITY:
        style->hidden = from->hidden;
        style->opacity = from->opacity;

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

    case PROP_ZINDEX:
        style->stacked = from->stacked;
        style->zindex = from->zindex;

        break;

    case PROP_BACKGROUNDCLIP:
    case PROP_WEBKITBACKGROUNDCLIP:
        style->textclip = from->textclip;

        break;

    case PROP_ASPECTRATIO:
        style->aspect = from->aspect;

        break;

    case PROP_TRANSFORM:
        copylength(&style->shift[0], &from->shift[0]);
        copylength(&style->shift[1], &from->shift[1]);

        break;

    case PROP_WORDBREAK:
    case PROP_OVERFLOWWRAP:
    case PROP_WORDWRAP:
        style->breaking = from->breaking;

        break;

    case PROP_LETTERSPACING:
        style->letterspacing = from->letterspacing;

        break;

    case PROP_VERTICALALIGN:
        style->valign = from->valign;
        style->valignoffset = from->valignoffset;

        break;

    case PROP_BOXSHADOW:
        style->shadow = from->shadow;
        style->shadowx = from->shadowx;
        style->shadowy = from->shadowy;
        style->shadowblur = from->shadowblur;
        style->shadowspread = from->shadowspread;

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
    style->minwidth.unit = CSS_UNIT_AUTO;
    style->maxwidth.unit = CSS_UNIT_NONE;
    style->maxheight.unit = CSS_UNIT_NONE;
    style->basis.unit = CSS_UNIT_AUTO;
    style->shrink = 100;
    style->alignitems = CSS_FLEX_STRETCH;
    style->alignself = CSS_FLEX_AUTO;
    style->opacity = 0xFF;

    for (side = 0; side < CSS_SIDES; side++)
    {

        style->borderwidth[side] = 3;
        style->bordercolor[side] = CSS_CURRENTCOLOR;
        style->inset[side].unit = CSS_UNIT_AUTO;

    }

}

static void setopacity(struct context *context, char *value, unsigned int length)
{

    struct number number = parsenumber(value, length);
    int milli = number.milli;

    if (!number.length)
        return;

    if (number.length < length && value[number.length] == '%')
        milli /= 100;

    milli = (milli < 0) ? 0 : (milli > 1000) ? 1000 : milli;

    /* a box fades with everything inside it, so the factor multiplies down the tree */
    context->style->opacity = context->parent->opacity * milli / 1000;

    if (!milli)
        context->style->hidden = 1;

}

static unsigned int setradius(struct context *context, char *value, unsigned int length, unsigned int corner)
{

    struct lengthvalue radius = parselength(context, value, length, context->style->fontsize);

    /* an elliptical corner keeps only its horizontal radius */
    if (!radius.valid || radius.length.unit > CSS_UNIT_PERCENT || radius.length.value < 0)
        return 0;

    copylength(&context->style->radius[corner], &radius.length);

    return 1;

}

static void setradii(struct context *context, char *value)
{

    struct span spans[CSS_CORNERS];
    unsigned int count = countspans(value, spans, CSS_CORNERS);
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (value[spans[i].start] == '/')
            break;

    }

    count = i;

    if (!count)
        return;

    /* the values go round from the top left as for sides, missing ones copy the opposite corner */
    for (i = 0; i < CSS_CORNERS; i++)
    {

        unsigned int source = (i < count) ? i : (i == CSS_BOTTOMRIGHT) ? CSS_TOPLEFT : (i == CSS_BOTTOMLEFT && count > 1) ? CSS_TOPRIGHT : 0;

        setradius(context, value + spans[source].start, spans[source].length, i);

    }

}

static void addshift(struct context *context, unsigned int axis, char *text, unsigned int length)
{

    struct css_length *shift = &context->style->shift[axis];
    struct lengthvalue parsed = parselength(context, text, length, context->style->fontsize);

    if (!parsed.valid || parsed.length.unit > CSS_UNIT_PERCENT)
        return;

    /* a pixel part rides along with a percentage as its offset, so both kinds add up */
    if (parsed.length.unit == CSS_UNIT_PERCENT)
    {

        shift->offset = (shift->unit == CSS_UNIT_PERCENT) ? shift->offset + parsed.length.offset : shift->value + parsed.length.offset;
        shift->value = (shift->unit == CSS_UNIT_PERCENT) ? shift->value + parsed.length.value : parsed.length.value;
        shift->unit = CSS_UNIT_PERCENT;

    }

    else if (shift->unit == CSS_UNIT_PERCENT)
    {

        shift->offset += parsed.length.value;

    }

    else
    {

        shift->value += parsed.length.value;
        shift->unit = CSS_UNIT_PX;

    }

}

static void settransform(struct context *context, char *value, unsigned int length)
{

    unsigned int offset = 0;
    unsigned int axis;

    for (axis = 0; axis < 2; axis++)
    {

        context->style->shift[axis].unit = CSS_UNIT_PX;
        context->style->shift[axis].value = 0;
        context->style->shift[axis].offset = 0;

    }

    /* only moves are kept, turning and scaling would need the whole picture redrawn */
    while (offset < length)
    {

        unsigned int open = offset + buffer_findbyte(value + offset, length - offset, '(');
        unsigned int close;
        unsigned int comma;
        unsigned int start;

        if (open >= length)
            break;

        close = findclose(value, open, length);
        comma = findstop(value, open + 1, close, ",");
        start = skipspace(value, open + 1, close);

        if (open - offset >= 9 && buffer_match(value + open - 9, "translate", 9))
        {

            addshift(context, 0, value + start, comma - start);

            if (comma < close)
                addshift(context, 1, value + skipspace(value, comma + 1, close), findstop(value, skipspace(value, comma + 1, close), close, ",") - skipspace(value, comma + 1, close));

        }

        else if (open - offset >= 10 && buffer_match(value + open - 10, "translatex", 10))
        {

            addshift(context, 0, value + start, close - start);

        }

        else if (open - offset >= 10 && buffer_match(value + open - 10, "translatey", 10))
        {

            addshift(context, 1, value + start, close - start);

        }

        else if (open - offset >= 11 && buffer_match(value + open - 11, "translate3d", 11))
        {

            addshift(context, 0, value + start, comma - start);

            if (comma < close)
                addshift(context, 1, value + skipspace(value, comma + 1, close), findstop(value, skipspace(value, comma + 1, close), close, ",") - skipspace(value, comma + 1, close));

        }

        offset = close + 1;

    }

}

static void setaspect(struct context *context, char *value, unsigned int length)
{

    unsigned int start = (length > 4 && buffer_match(value, "auto", 4)) ? skipspace(value, 4, length) : 0;
    unsigned int slash = findstop(value, start, length, "/");
    struct number width = parsenumber(value + start, slash - start);
    struct number height;

    context->style->aspect = 0;

    if (!width.length || width.milli <= 0)
        return;

    /* the ratio is kept in thousandths of width per height, a single number means a height of one */
    if (slash >= length)
    {

        context->style->aspect = width.milli;

        return;

    }

    start = skipspace(value, slash + 1, length);
    height = parsenumber(value + start, length - start);

    if (height.length && height.milli > 0)
        context->style->aspect = width.milli * 10 / (height.milli / 100);

}

static void setletterspacing(struct context *context, char *value, unsigned int length)
{

    struct lengthvalue parsed = parselength(context, value, length, context->style->fontsize);

    if (matchword(value, length, "normal"))
        context->style->letterspacing = 0;
    else if (parsed.valid && parsed.length.unit == CSS_UNIT_PX)
        context->style->letterspacing = parsed.length.value;

}

static void setvalign(struct context *context, char *value, unsigned int length)
{

    struct css_style *style = context->style;
    int size = context->parent->fontsize;
    struct lengthvalue parsed;

    style->valign = CSS_VALIGN_BASELINE;
    style->valignoffset = 0;

    /* text-top and text-bottom are close enough to the line's own edges */
    if (matchword(value, length, "top") || matchword(value, length, "text-top"))
        style->valign = CSS_VALIGN_TOP;
    else if (matchword(value, length, "bottom") || matchword(value, length, "text-bottom"))
        style->valign = CSS_VALIGN_BOTTOM;
    else if (matchword(value, length, "middle"))
        style->valign = CSS_VALIGN_MIDDLE;
    else if (matchword(value, length, "super"))
        style->valignoffset = size / 3;
    else if (matchword(value, length, "sub"))
        style->valignoffset = -size / 5;

    if (style->valign != CSS_VALIGN_BASELINE || style->valignoffset)
        return;

    parsed = parselength(context, value, length, style->fontsize);

    if (parsed.valid && parsed.length.unit == CSS_UNIT_PX)
        style->valignoffset = parsed.length.value;

}

static unsigned int parseshadow(struct context *context, char *value, unsigned int start, unsigned int end)
{

    struct css_style *style = context->style;
    unsigned int color = CSS_CURRENTCOLOR;
    int lengths[4];
    unsigned int count = 0;
    unsigned int offset = start;

    while (offset < end)
    {

        struct span span = nexttoken(value, offset);
        char *text = value + span.start;
        struct lengthvalue parsed;
        struct colorvalue parsedcolor;

        if (!span.length || span.start >= end)
            break;

        offset = span.start + span.length;

        if (matchword(text, span.length, "inset"))
            return 0;

        parsed = parselength(context, text, span.length, style->fontsize);

        if (parsed.valid && parsed.length.unit == CSS_UNIT_PX && count < 4)
        {

            lengths[count++] = parsed.length.value;

            continue;

        }

        parsedcolor = parsecolor(text, span.length);

        if (!parsedcolor.valid)
            return 0;

        color = parsedcolor.value;

    }

    if (count < 2)
        return 0;

    style->shadow = color;
    style->shadowx = lengths[0];
    style->shadowy = lengths[1];
    style->shadowblur = (count > 2 && lengths[2] > 0) ? lengths[2] : 0;
    style->shadowspread = (count > 3) ? lengths[3] : 0;

    return 1;

}

static void setshadow(struct context *context, char *value, unsigned int length)
{

    unsigned int start = 0;

    context->style->shadow = 0;

    /* only the first outer shadow is drawn, inset ones would need the box drawn first */
    while (start < length)
    {

        unsigned int end = findstop(value, start, length, ",");

        if (parseshadow(context, value, start, end))
            return;

        start = end + 1;

    }

}

static void addcodepoint(struct content *content, unsigned int codepoint)
{

    content->length += utf8_write(content->text + content->length, CONTENTSIZE - 1 - content->length, codepoint);

}

static void addcontent(struct content *content, char *text, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < length)
    {

        unsigned int codepoint;
        unsigned int count = utf8_read(text + offset, length - offset, &codepoint);

        if (!count)
            break;

        addcodepoint(content, codepoint);

        offset += count;

    }

}

static unsigned int readcontentstring(struct content *content, char *text, unsigned int offset, unsigned int end)
{

    char quote = text[offset++];

    while (offset < end && text[offset] != quote)
    {

        unsigned int codepoint = 0;
        unsigned int digits = 0;

        if (text[offset] != '\\' || offset + 1 >= end)
        {

            unsigned int count = utf8_read(text + offset, end - offset, &codepoint);

            offset += (count) ? count : 1;

            addcodepoint(content, codepoint);

            continue;

        }

        offset++;

        while (offset < end && digits < 6 && hexdigit(text[offset]) < 16)
        {

            codepoint = codepoint * 16 + hexdigit(text[offset++]);
            digits++;

        }

        if (digits)
        {

            if (offset < end && isspace(text[offset]))
                offset++;

            addcodepoint(content, codepoint);

        }

        else
        {

            unsigned int count = utf8_read(text + offset, end - offset, &codepoint);

            offset += (count) ? count : 1;

            if (codepoint != '\n')
                addcodepoint(content, codepoint);

        }

    }

    return offset + 1;

}

static unsigned int readcontentattribute(struct context *context, char *text, unsigned int offset, unsigned int end)
{

    unsigned int close = findclose(text, offset + 4, end);
    unsigned int start = skipspace(text, offset + 5, close);
    unsigned int stop = readident(text, start, close);
    char name[NAMESIZE];
    char *value;
    unsigned int i;

    if (stop - start >= NAMESIZE)
        return close + 1;

    for (i = start; i < stop; i++)
        name[i - start] = lower(text[i]);

    name[stop - start] = '\0';
    value = html_getattribute(context->document, context->node, name);

    if (value)
        addcontent(context->content, value, cstring_length(value));

    return close + 1;

}

static void setcontent(struct context *context, char *value, unsigned int length)
{

    struct content *content = context->content;
    unsigned int offset = 0;

    content->length = 0;
    content->valid = !matchword(value, length, "none") && !matchword(value, length, "normal");

    /* counters, quotes and images add nothing but still make the box */
    while (offset < length && content->valid)
    {

        char c = value[offset];

        if (c == '"' || c == '\'')
            offset = readcontentstring(content, value, offset, length);
        else if (offset + 5 < length && buffer_match(value + offset, "attr(", 5))
            offset = readcontentattribute(context, value, offset, length);
        else if (c == '(')
            offset = findclose(value, offset, length) + 1;
        else
            offset++;

    }

    content->text[content->length] = '\0';

}

static void apply(struct context *context, unsigned int property, char *value);

static void applyspan(struct context *context, unsigned int property, char *value, struct span *span)
{

    char buffer[VALUESIZE];

    if (span->length >= VALUESIZE)
        return;

    buffer_copy(buffer, value + span->start, span->length);

    buffer[span->length] = '\0';

    apply(context, property, buffer);

}

static void setpair(struct context *context, unsigned int start, unsigned int end, char *value)
{

    struct span spans[2];
    unsigned int count = countspans(value, spans, 2);

    if (!count)
        return;

    applyspan(context, start, value, &spans[0]);
    applyspan(context, end, value, &spans[count - 1]);

}

static void setfour(struct context *context, unsigned int first, char *value)
{

    struct span spans[CSS_SIDES];
    unsigned int count = countspans(value, spans, CSS_SIDES);
    unsigned int side;

    if (!count)
        return;

    /* top, right, bottom and left, with missing ones copied from the opposite side as for margins */
    for (side = 0; side < CSS_SIDES; side++)
    {

        unsigned int source = (side < count) ? side : (side == CSS_BOTTOM) ? CSS_TOP : (count > 1) ? CSS_RIGHT : CSS_TOP;

        applyspan(context, first + side, value, &spans[source]);

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
        settracks(context, value, 1);

        break;

    case PROP_GRIDTEMPLATEROWS:
        settracks(context, value, 0);

        break;

    case PROP_GRIDTEMPLATEAREAS:
        setareas(context, value);

        break;

    case PROP_GRIDAUTOROWS:
        setautorows(context, value);

        break;

    case PROP_GRIDCOLUMN:
        setlines(context, value, CSS_GRIDCOLUMNSTART, CSS_GRIDCOLUMNEND);

        break;

    case PROP_GRIDROW:
        setlines(context, value, CSS_GRIDROWSTART, CSS_GRIDROWEND);

        break;

    case PROP_GRIDCOLUMNSTART:
    case PROP_GRIDCOLUMNEND:
    case PROP_GRIDROWSTART:
    case PROP_GRIDROWEND:
        context->style->gridlines[(property == PROP_GRIDROWSTART) ? CSS_GRIDROWSTART : (property == PROP_GRIDCOLUMNSTART) ? CSS_GRIDCOLUMNSTART : (property == PROP_GRIDROWEND) ? CSS_GRIDROWEND : CSS_GRIDCOLUMNEND] = parseline(value, length);

        break;

    case PROP_ASPECTRATIO:
        setaspect(context, value, length);

        break;

    case PROP_TRANSFORM:
        settransform(context, value, length);

        break;

    case PROP_WORDBREAK:
        style->breaking = (matchword(value, length, "break-all") || matchword(value, length, "break-word")) ? style->breaking | CSS_BREAK_WORDBREAK : style->breaking & ~CSS_BREAK_WORDBREAK;

        break;

    case PROP_OVERFLOWWRAP:
    case PROP_WORDWRAP:
        style->breaking = (matchword(value, length, "anywhere")) ? style->breaking | CSS_BREAK_OVERFLOWWRAP : style->breaking & ~CSS_BREAK_OVERFLOWWRAP;

        break;

    case PROP_GRIDAREA:
        setarea(context, value);

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
        setradii(context, value);

        break;

    case PROP_TOPLEFTRADIUS:
    case PROP_TOPRIGHTRADIUS:
    case PROP_BOTTOMRIGHTRADIUS:
    case PROP_BOTTOMLEFTRADIUS:
        setradius(context, value, length, property - PROP_TOPLEFTRADIUS);

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
        if (matchword(value, length, "-webkit-center"))
            style->align = CSS_ALIGN_WEBKITCENTER;
        else if (matchword(value, length, "center"))
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
        setopacity(context, value, length);

        break;

    case PROP_INSET:
        setfour(context, PROP_TOP, value);

        break;

    case PROP_MARGININLINE:
        setpair(context, PROP_MARGINTOP + CSS_LEFT, PROP_MARGINTOP + CSS_RIGHT, value);

        break;

    case PROP_MARGINBLOCK:
        setpair(context, PROP_MARGINTOP + CSS_TOP, PROP_MARGINTOP + CSS_BOTTOM, value);

        break;

    case PROP_PADDINGINLINE:
        setpair(context, PROP_PADDINGTOP + CSS_LEFT, PROP_PADDINGTOP + CSS_RIGHT, value);

        break;

    case PROP_PADDINGBLOCK:
        setpair(context, PROP_PADDINGTOP + CSS_TOP, PROP_PADDINGTOP + CSS_BOTTOM, value);

        break;

    case PROP_INSETINLINE:
        setpair(context, PROP_TOP + CSS_LEFT, PROP_TOP + CSS_RIGHT, value);

        break;

    case PROP_INSETBLOCK:
        setpair(context, PROP_TOP + CSS_TOP, PROP_TOP + CSS_BOTTOM, value);

        break;

    case PROP_BORDERINLINE:
        apply(context, PROP_BORDERTOP + CSS_LEFT, value);
        apply(context, PROP_BORDERTOP + CSS_RIGHT, value);

        break;

    case PROP_BORDERBLOCK:
        apply(context, PROP_BORDERTOP + CSS_TOP, value);
        apply(context, PROP_BORDERTOP + CSS_BOTTOM, value);

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
        setlength(context, &style->minwidth, value, 0);

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
        style->monospace = getmonospace(value, length);

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

    case PROP_BOXSHADOW:
        setshadow(context, value, length);

        break;

    case PROP_BACKGROUNDCLIP:
    case PROP_WEBKITBACKGROUNDCLIP:
        style->textclip = matchword(value, length, "text");

        break;

    case PROP_VERTICALALIGN:
        setvalign(context, value, length);

        break;

    case PROP_LETTERSPACING:
        setletterspacing(context, value, length);

        break;

    case PROP_CONTENT:
        if (context->content)
            setcontent(context, value, length);

        break;

    case PROP_ZINDEX:
        if (matchword(value, length, "auto"))
        {

            style->stacked = 0;
            style->zindex = 0;

        }

        else
        {

            struct number number = parsenumber(value, length);

            if (number.length == length)
            {

                style->stacked = 1;
                style->zindex = (number.milli < -32000000) ? -32000 : (number.milli > 32000000) ? 32000 : number.milli / 1000;

            }

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
static unsigned int matchhas(struct matcher *matcher, unsigned int index, unsigned int node);
static unsigned int matchselector(struct matcher *matcher, unsigned int first, unsigned int end, unsigned int node);

static unsigned int matchlist(struct matcher *matcher, unsigned int start, unsigned int end, unsigned int node)
{

    unsigned int i;

    for (i = start; i < end; i += matcher->sheet->parts[i].name + 1)
    {

        if (matchselector(matcher, i + 1, i + 1 + matcher->sheet->parts[i].name, node))
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
        unsigned int matched = (part->type == PART_IS) ? matchlist(matcher, i + 1, i + 1 + part->name, node) : (part->type == PART_HAS) ? matchhas(matcher, i, node) : matchpart(matcher, part, node);

        if (matched == part->negate)
            return 0;

        if (part->type == PART_IS || part->type == PART_HAS)
            i += part->name;

    }

    return 1;

}

static unsigned int matchhas(struct matcher *matcher, unsigned int index, unsigned int node)
{

    struct html_document *document = matcher->document;
    struct css_part *part = &matcher->sheet->parts[index];
    unsigned int other;

    /* siblings are looked for after the element, everything else inside it */
    if (part->value == HAS_NEXT || part->value == HAS_LATER)
    {

        for (other = document->nodes[node].next; other; other = document->nodes[other].next)
        {

            if (document->nodes[other].type != HTML_NODE_ELEMENT)
                continue;

            if (matchlist(matcher, index + 1, index + 1 + part->name, other))
                return 1;

            if (part->value == HAS_NEXT)
                return 0;

        }

        return 0;

    }

    for (other = html_next(document, node, node); other; other = (part->value == HAS_CHILD) ? document->nodes[other].next : html_next(document, other, node))
    {

        if (document->nodes[other].type == HTML_NODE_ELEMENT && matchlist(matcher, index + 1, index + 1 + part->name, other))
            return 1;

    }

    return 0;

}

static unsigned int matchselector(struct matcher *matcher, unsigned int first, unsigned int end, unsigned int node)
{

    struct css_part *parts = matcher->sheet->parts;
    unsigned int start = findcompound(parts, first, end);
    unsigned int other;

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

static unsigned int collect(struct css_sheet *sheet, struct html_document *document, unsigned int node, unsigned int width, unsigned int scheme, unsigned int pseudo, struct match matches[MATCHES])
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

        if (selector->pseudo != pseudo || (int)width < rule->minwidth || (int)width > rule->maxwidth || (rule->scheme && rule->scheme != scheme))
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

static unsigned int isjoining(char c)
{

    return isidentchar(c) || c == '.' || c == '%' || c == '#';

}

static void addspace(struct expansion *expansion)
{

    if (expansion->length + 1 >= EXPANDSIZE)
        expansion->valid = 0;
    else
        expansion->text[expansion->length++] = ' ';

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

            /* what var() gives stays its own tokens, so minified text next to it is kept apart */
            if (expansion->length && isjoining(expansion->text[expansion->length - 1]))
                addspace(expansion);

            value = findcustom(context, text + start, end - start);

            if (value && value[0] && depth < EXPANDDEPTH)
                expand(context, expansion, value, cstring_length(value), depth + 1);
            else if (comma < close && depth < EXPANDDEPTH)
                expand(context, expansion, text + skipspace(text, comma + 1, close), close - skipspace(text, comma + 1, close), depth + 1);
            else
                expansion->valid = 0;

            i = close + 1;

            if (i < length && isjoining(text[i]))
                addspace(expansion);

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

            applyhint(context, PROP_TEXTALIGN, (cstring_match(value, "center") || cstring_match(value, "middle")) ? "-webkit-center" : value, "");

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

static void quirktable(struct css_style *style, struct css_style *initial)
{

    /* in quirks mode a table starts over with fonts, lines and alignment, as the html rendering rules say */
    style->fontsize = initial->fontsize;
    style->keyword = initial->keyword;
    style->lineheight = initial->lineheight;
    style->lineheightfactor = initial->lineheightfactor;
    style->bold = initial->bold;
    style->italic = initial->italic;
    style->align = initial->align;
    style->whitespace = initial->whitespace;

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
    style->opacity = parent->opacity;
    style->letterspacing = parent->letterspacing;
    style->breaking = parent->breaking;

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

    if (style->shadow == CSS_CURRENTCOLOR)
        style->shadow = style->color;

}

static void blockify(struct css_style *style)
{

    unsigned int outside = style->floating || style->position == CSS_POSITION_ABSOLUTE || style->position == CSS_POSITION_FIXED;

    if (outside && (style->display == CSS_DISPLAY_INLINE || style->display == CSS_DISPLAY_INLINEBLOCK))
        style->display = CSS_DISPLAY_BLOCK;

}

static unsigned int isatomic(unsigned int tag)
{

    return tag == HTML_TAG_IMG || tag == HTML_TAG_INPUT || tag == HTML_TAG_SELECT || tag == HTML_TAG_TEXTAREA || tag == HTML_TAG_BUTTON;

}

static void computeelement(struct css_sheet *sheet, struct html_document *document, struct context *context, unsigned int node)
{

    struct match matches[MATCHES];
    unsigned int count = collect(sheet, document, node, context->width, context->scheme, CSS_PSEUDO_NONE, matches);
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

        if (phase == PHASE_FONT && context->style->keyword && (context->style->monospace == CSS_MONOSPACE_GENERIC) != (context->parent->monospace == CSS_MONOSPACE_GENERIC))
            context->style->fontsize = (context->style->monospace == CSS_MONOSPACE_GENERIC) ? context->style->fontsize * 13 / 16 : context->style->fontsize * 16 / 13;

    }

    sheet->declarationcount = declarationmark;

    if (context->style->display == CSS_DISPLAY_INLINE && isatomic(document->nodes[node].tag))
        context->style->display = CSS_DISPLAY_INLINEBLOCK;

    blockify(context->style);
    finish(context->style);

}

static unsigned int hasgenerated(unsigned int tag)
{

    return tag != HTML_TAG_IMG && tag != HTML_TAG_INPUT && tag != HTML_TAG_SELECT && tag != HTML_TAG_TEXTAREA && tag != HTML_TAG_SVG;

}

static void computepseudo(struct css_sheet *sheet, struct html_document *document, struct context *context, struct css_style *styles, unsigned int pseudo)
{

    struct match matches[MATCHES];
    unsigned int count = collect(sheet, document, context->node, context->width, context->scheme, pseudo, matches);
    struct context pseudocontext;
    struct content content;
    struct css_style style;
    unsigned int generated;
    unsigned int phase;

    if (!count)
        return;

    buffer_copy(&pseudocontext, context, sizeof (struct context));

    pseudocontext.style = &style;
    pseudocontext.parent = context->style;
    pseudocontext.content = &content;
    content.length = 0;
    content.valid = 0;

    inherit(&style, context->style);

    /* custom properties stay with the element, so var() in a pseudo element reads the element's */
    for (phase = PHASE_FONT; phase < PHASES; phase++)
    {

        applyrules(&pseudocontext, sheet, matches, count, CSS_ORIGIN_AGENT, 0, phase);
        applyrules(&pseudocontext, sheet, matches, count, CSS_ORIGIN_AUTHOR, 0, phase);
        applyrules(&pseudocontext, sheet, matches, count, CSS_ORIGIN_AUTHOR, 1, phase);

    }

    if (!content.valid || style.display == CSS_DISPLAY_NONE)
        return;

    blockify(&style);
    finish(&style);

    generated = html_addgenerated(document, context->node, (pseudo == CSS_PSEUDO_BEFORE) ? HTML_TAG_BEFORE : HTML_TAG_AFTER, content.text, content.length);

    if (generated)
        buffer_copy(&styles[generated], &style, sizeof (struct css_style));

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
    unsigned int count;
    unsigned int node;

    html_dropgenerated(document);
    initstyle(&root);

    sheet->customtotal = 0;
    sheet->gradientcount = 0;
    sheet->gridcount = 0;
    context.sheet = sheet;
    context.document = document;
    context.rootsize = 16;
    context.width = width;
    context.height = height;
    context.scheme = scheme;
    context.content = 0;
    count = document->nodecount;

    for (node = 1; node < count; node++)
    {

        unsigned int parent = parentelement(document, node);

        if (document->nodes[node].type != HTML_NODE_ELEMENT)
            continue;

        context.node = node;
        context.style = &styles[node];
        context.parent = (parent) ? &styles[parent] : &root;

        inherit(context.style, context.parent);

        if (document->quirks && document->nodes[node].tag == HTML_TAG_TABLE)
            quirktable(context.style, &root);

        /* the old centering centers a table but not what is in it */
        if (document->nodes[node].tag == HTML_TAG_TABLE && context.style->align == CSS_ALIGN_WEBKITCENTER)
            context.style->align = CSS_ALIGN_LEFT;

        computeelement(sheet, document, &context, node);

        if (node == document->html)
            context.rootsize = context.style->fontsize;

        if (context.style->display != CSS_DISPLAY_NONE && hasgenerated(document->nodes[node].tag))
        {

            computepseudo(sheet, document, &context, styles, CSS_PSEUDO_BEFORE);
            computepseudo(sheet, document, &context, styles, CSS_PSEUDO_AFTER);

        }

    }

    /* generated boxes join the tree only now, so no selector above saw them as children */
    html_linkgenerated(document);

    sheet->poolcount = poolmark;
    sheet->full = full;

}

