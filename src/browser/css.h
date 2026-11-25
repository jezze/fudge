#define CSS_PARTS                       32768
#define CSS_SELECTORS                   16384
#define CSS_DECLARATIONS                32768
#define CSS_RULES                       16384
#define CSS_POOLSIZE                    0x80000
#define CSS_CUSTOMS                     16384
#define CSS_GRADIENTS                   64
#define CSS_STOPS                       6
#define CSS_ORIGIN_AGENT                0
#define CSS_ORIGIN_AUTHOR               1
#define CSS_UNIT_PX                     0
#define CSS_UNIT_PERCENT                1
#define CSS_UNIT_AUTO                   2
#define CSS_UNIT_NONE                   3
#define CSS_UNIT_FIT                    4
#define CSS_DISPLAY_INLINE              0
#define CSS_DISPLAY_BLOCK               1
#define CSS_DISPLAY_LISTITEM            2
#define CSS_DISPLAY_INLINEBLOCK         3
#define CSS_DISPLAY_NONE                4
#define CSS_DISPLAY_TABLE               5
#define CSS_DISPLAY_ROWGROUP            6
#define CSS_DISPLAY_ROW                 7
#define CSS_DISPLAY_CELL                8
#define CSS_DISPLAY_FLEX                9
#define CSS_DISPLAY_GRID                10
#define CSS_COLUMNS_AUTOFILL            0xFF
#define CSS_ALIGN_LEFT                  0
#define CSS_ALIGN_CENTER                1
#define CSS_ALIGN_RIGHT                 2
#define CSS_WHITESPACE_NORMAL           0
#define CSS_WHITESPACE_PRE              1
#define CSS_WHITESPACE_NOWRAP           2
#define CSS_WHITESPACE_PREWRAP          3
#define CSS_WHITESPACE_PRELINE          4
#define CSS_LIST_NONE                   0
#define CSS_LIST_DISC                   1
#define CSS_LIST_CIRCLE                 2
#define CSS_LIST_SQUARE                 3
#define CSS_LIST_DECIMAL                4
#define CSS_DECORATION_UNDERLINE        1
#define CSS_DECORATION_LINETHROUGH      2
#define CSS_TRANSFORM_NONE              0
#define CSS_TRANSFORM_UPPERCASE         1
#define CSS_TRANSFORM_LOWERCASE         2
#define CSS_POSITION_STATIC             0
#define CSS_POSITION_RELATIVE           1
#define CSS_POSITION_ABSOLUTE           2
#define CSS_POSITION_FIXED              3
#define CSS_TOP                         0
#define CSS_RIGHT                       1
#define CSS_BOTTOM                      2
#define CSS_LEFT                        3
#define CSS_SIDES                       4
#define CSS_SCHEME_ANY                  0
#define CSS_SCHEME_LIGHT                1
#define CSS_SCHEME_DARK                 2
#define CSS_CURRENTCOLOR                0x00000001
#define CSS_FLEX_START                  0
#define CSS_FLEX_END                    1
#define CSS_FLEX_CENTER                 2
#define CSS_FLEX_STRETCH                3
#define CSS_FLEX_BETWEEN                4
#define CSS_FLEX_AROUND                 5
#define CSS_FLEX_EVENLY                 6
#define CSS_FLEX_AUTO                   7
#define CSS_FLOAT_NONE                  0
#define CSS_FLOAT_LEFT                  1
#define CSS_FLOAT_RIGHT                 2
#define CSS_CLEAR_LEFT                  1
#define CSS_CLEAR_RIGHT                 2

struct css_length
{

    int value;
    unsigned short unit;
    short offset;

};

struct css_style
{

    unsigned char display;
    unsigned char align;
    unsigned char whitespace;
    unsigned char liststyle;
    unsigned char decoration;
    unsigned char bold;
    unsigned char italic;
    unsigned char hidden;
    unsigned char borderbox;
    unsigned char position;
    unsigned char column;
    unsigned char transform;
    unsigned char columns;
    unsigned char flexbox;
    unsigned char wrap;
    unsigned char justify;
    unsigned char alignitems;
    unsigned char alignself;
    unsigned char floating;
    unsigned char clear;
    unsigned char overflow;
    unsigned char monospace;
    unsigned char keyword;
    unsigned char spacing;
    unsigned char collapse;
    unsigned char reverse;
    unsigned char stacked;
    unsigned char borderstyle[CSS_SIDES];
    unsigned char borderwidth[CSS_SIDES];
    unsigned short fontsize;
    unsigned short lineheight;
    unsigned short lineheightfactor;
    unsigned short gradient;
    unsigned short trackmin;
    unsigned short rowgap;
    unsigned short columngap;
    unsigned short grow;
    unsigned short shrink;
    short order;
    short zindex;
    unsigned int color;
    unsigned int fill;
    unsigned int background;
    unsigned int bordercolor[CSS_SIDES];
    struct css_length margin[CSS_SIDES];
    struct css_length padding[CSS_SIDES];
    struct css_length width;
    struct css_length height;
    struct css_length minwidth;
    struct css_length maxwidth;
    struct css_length minheight;
    struct css_length maxheight;
    struct css_length radius;
    struct css_length basis;
    struct css_length inset[CSS_SIDES];

};

struct css_stop
{

    unsigned int color;
    int position;

};

struct css_gradient
{

    unsigned int radial;
    unsigned int count;
    int angle;
    int x;
    int y;
    int width;
    int height;
    struct css_stop stops[CSS_STOPS];

};

struct css_part
{

    unsigned char type;
    unsigned char combinator;
    unsigned char op;
    unsigned char negate;
    unsigned int name;
    unsigned int value;

};

struct css_selector
{

    unsigned int part;
    unsigned int count;
    unsigned int specificity;
    unsigned int rule;
    unsigned int keytype;
    unsigned int key;

};

struct css_declaration
{

    unsigned short property;
    unsigned short important;
    unsigned int name;
    unsigned int value;

};

struct css_custom
{

    unsigned int name;
    unsigned int value;

};

struct css_rule
{

    unsigned int declaration;
    unsigned int declarations;
    unsigned int origin;
    unsigned int scheme;
    int minwidth;
    int maxwidth;

};

struct css_sheet
{

    struct css_part parts[CSS_PARTS];
    struct css_selector selectors[CSS_SELECTORS];
    struct css_declaration declarations[CSS_DECLARATIONS];
    struct css_rule rules[CSS_RULES];
    struct css_custom customs[CSS_CUSTOMS];
    struct css_gradient gradients[CSS_GRADIENTS];
    unsigned int gradientcount;
    unsigned short customstart[HTML_NODES];
    unsigned short customcount[HTML_NODES];
    char pool[CSS_POOLSIZE];
    unsigned int customtotal;
    unsigned int partcount;
    unsigned int selectorcount;
    unsigned int declarationcount;
    unsigned int rulecount;
    unsigned int poolcount;
    unsigned int full;

};

struct css_color
{

    unsigned int value;
    unsigned int valid;

};

void css_reset(struct css_sheet *sheet);
struct css_color css_parsecolor(char *text, unsigned int length);
void css_parse(struct css_sheet *sheet, char *text, unsigned int count, unsigned int origin);
void css_compute(struct css_sheet *sheet, struct html_document *document, struct css_style *styles, unsigned int width, unsigned int height, unsigned int scheme);
