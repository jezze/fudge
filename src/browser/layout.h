#define LAYOUT_FRAGMENTS                16384
#define LAYOUT_ITEMS                    512
#define LAYOUT_TEXTSIZE                 0x10000
#define LAYOUT_FLEXITEMS                4096
#define LAYOUT_FLOATS                   64
#define LAYOUT_ABSOLUTES                256
#define LAYOUT_FRAGMENT_TEXT            0
#define LAYOUT_FRAGMENT_GENERATED       1
#define LAYOUT_FRAGMENT_MARKER          2

struct layout_picture
{

    unsigned int width;
    unsigned int height;
    unsigned int ratiowidth;
    unsigned int ratioheight;
    unsigned int offset;
    unsigned int columns;
    unsigned int rows;

};

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

struct layout_flexitem
{

    unsigned int node;
    unsigned int fragmentstart;
    unsigned int fragmentend;
    unsigned int automargins;
    unsigned int frozen;
    int size;
    int minimum;
    int ml;
    int mr;
    int mt;
    int mb;
    int height;

};

struct layout_measure
{

    int availw;
    int width;

};

struct layout_absolute
{

    unsigned int node;
    int x;
    int y;

};

struct layout_float
{

    int left;
    int right;
    int top;
    int bottom;
    unsigned int side;

};

struct layout
{

    struct html_document *document;
    struct css_style *styles;
    struct css_gradient *gradients;
    struct font_face *faces;
    struct layout_picture *pictures;
    unsigned short picture[HTML_NODES];
    struct layout_box boxes[HTML_NODES];
    struct layout_measure measures[HTML_NODES];
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
    unsigned int truncated;
    unsigned int minimum;
    unsigned int forcenode;
    unsigned int baselinedepth;
    int baseline;
    int forcewidth;
    unsigned int flexcount;
    struct layout_flexitem flexitems[LAYOUT_FLEXITEMS];
    struct layout_float floats[LAYOUT_FLOATS];
    unsigned int floatcount;
    unsigned int floatbase;
    struct layout_absolute absolutes[LAYOUT_ABSOLUTES];
    unsigned int absolutecount;
    int extent;
    int height;
    int viewheight;
    int trailing;
    unsigned int full;

};

void layout_document(struct layout *layout, unsigned int width, unsigned int height);
unsigned int layout_findnode(struct layout *layout, int x, int y);
unsigned int layout_findlink(struct layout *layout, int x, int y);
