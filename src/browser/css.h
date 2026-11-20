#define CSS_PARTS                       4096
#define CSS_SELECTORS                   2048
#define CSS_DECLARATIONS                4096
#define CSS_RULES                       2048
#define CSS_POOLSIZE                    0x10000
#define CSS_ORIGIN_AGENT                0
#define CSS_ORIGIN_AUTHOR               1
#define CSS_UNIT_PX                     0
#define CSS_UNIT_PERCENT                1
#define CSS_UNIT_AUTO                   2
#define CSS_UNIT_NONE                   3
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
#define CSS_POSITION_STATIC             0
#define CSS_POSITION_RELATIVE           1
#define CSS_POSITION_ABSOLUTE           2
#define CSS_POSITION_FIXED              3
#define CSS_TOP                         0
#define CSS_RIGHT                       1
#define CSS_BOTTOM                      2
#define CSS_LEFT                        3
#define CSS_SIDES                       4

struct css_length
{

    int value;
    unsigned int unit;

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
    unsigned char borderstyle[CSS_SIDES];
    unsigned char borderwidth[CSS_SIDES];
    unsigned short fontsize;
    unsigned short lineheight;
    unsigned short lineheightfactor;
    unsigned int color;
    unsigned int background;
    unsigned int bordercolor[CSS_SIDES];
    struct css_length margin[CSS_SIDES];
    struct css_length padding[CSS_SIDES];
    struct css_length width;
    struct css_length height;
    struct css_length minwidth;
    struct css_length maxwidth;
    struct css_length minheight;

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
    unsigned int value;

};

struct css_rule
{

    unsigned int declaration;
    unsigned int declarations;
    unsigned int origin;
    int minwidth;
    int maxwidth;

};

struct css_sheet
{

    struct css_part parts[CSS_PARTS];
    struct css_selector selectors[CSS_SELECTORS];
    struct css_declaration declarations[CSS_DECLARATIONS];
    struct css_rule rules[CSS_RULES];
    char pool[CSS_POOLSIZE];
    unsigned int partcount;
    unsigned int selectorcount;
    unsigned int declarationcount;
    unsigned int rulecount;
    unsigned int poolcount;
    unsigned int full;

};

void css_reset(struct css_sheet *sheet);
void css_parse(struct css_sheet *sheet, char *text, unsigned int count, unsigned int origin);
void css_compute(struct css_sheet *sheet, struct html_document *document, struct css_style *styles, unsigned int width, unsigned int height);
