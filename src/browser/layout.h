#define LAYOUT_FRAGMENTS                4096
#define LAYOUT_ITEMS                    512
#define LAYOUT_TEXTSIZE                 0x4000
#define LAYOUT_FRAGMENT_TEXT            0
#define LAYOUT_FRAGMENT_GENERATED       1
#define LAYOUT_FRAGMENT_MARKER          2

struct layout_box
{

    int x;
    int y;
    int w;
    int h;
    unsigned int placed;

};

struct layout_fragment
{

    unsigned int node;
    int x;
    int y;
    unsigned short w;
    unsigned short h;
    unsigned int text;
    unsigned short length;
    unsigned char type;
    unsigned char collapse;
    unsigned char face;
    unsigned char scale;
    unsigned char ascent;
    unsigned char depth;

};

struct layout_item
{

    unsigned int node;
    unsigned int fragmentstart;
    unsigned int fragmentend;
    unsigned int atomic;
    int ascent;
    int descent;
    int shift;

};

struct layout
{

    struct html_document *document;
    struct css_style *styles;
    struct font_face *faces;
    struct layout_box boxes[HTML_NODES];
    struct layout_fragment fragments[LAYOUT_FRAGMENTS];
    struct layout_item items[LAYOUT_ITEMS];
    char text[LAYOUT_TEXTSIZE];
    unsigned int fragmentcount;
    unsigned int itemcount;
    unsigned int textcount;
    unsigned int marker;
    unsigned int markerdepth;
    unsigned int atomicdepth;
    unsigned int measuring;
    int extent;
    int height;
    unsigned int full;

};

void layout_document(struct layout *layout, unsigned int width);
unsigned int layout_findlink(struct layout *layout, int x, int y);
