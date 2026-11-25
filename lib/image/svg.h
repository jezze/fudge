#define SVG_ONE                         0x10000
#define SVG_TURN                        0x10000
#define SVG_POINTS                      16384
#define SVG_SUBPATHS                    1024
#define SVG_EDGES                       16384
#define SVG_CROSSINGS                   1024
#define SVG_WIDTH                       4096
#define SVG_CAP_BUTT                    0
#define SVG_CAP_ROUND                   1
#define SVG_CAP_SQUARE                  2

struct svg_matrix
{

    int a;
    int b;
    int c;
    int d;
    int e;
    int f;

};

struct svg_point
{

    int x;
    int y;

};

struct svg_subpath
{

    unsigned int start;
    unsigned int count;
    unsigned int closed;

};

struct svg_edge
{

    int x0;
    int y0;
    int y1;
    int slope;
    int winding;

};

struct svg_reader
{

    char *text;
    unsigned int offset;

};

struct svg_number
{

    int value;
    unsigned int valid;

};

/* numbers are 16.16 fixed point, angles a 16 bit fraction of a turn and the canvas at most SVG_WIDTH wide */
struct svg
{

    unsigned int *pixels;
    unsigned int width;
    unsigned int height;
    struct svg_point points[SVG_POINTS];
    struct svg_subpath subpaths[SVG_SUBPATHS];
    struct svg_edge edges[SVG_EDGES];
    int coverage[SVG_WIDTH];
    int crossings[SVG_CROSSINGS];
    int windings[SVG_CROSSINGS];
    unsigned int pointcount;
    unsigned int subpathcount;
    unsigned int edgecount;

};

int svg_mulfix(int a, int b);
int svg_divfix(int a, int b);
void svg_setmatrix(struct svg_matrix *matrix, int a, int b, int c, int d, int e, int f);
void svg_multiply(struct svg_matrix *matrix, struct svg_matrix *by);
void svg_parsetransform(struct svg_matrix *matrix, char *text);
struct svg_number svg_readnumber(struct svg_reader *reader);
unsigned int svg_readpoints(struct svg_reader *reader, int *values, unsigned int count);
void svg_reset(struct svg *svg);
void svg_moveto(struct svg *svg, struct svg_matrix *matrix, int x, int y);
void svg_lineto(struct svg *svg, struct svg_matrix *matrix, int x, int y);
void svg_closepath(struct svg *svg);
void svg_ellipse(struct svg *svg, struct svg_matrix *matrix, int cx, int cy, int rx, int ry, int from, int sweep, int cosine, int sine);
void svg_parsepath(struct svg *svg, struct svg_matrix *matrix, char *text);
void svg_parsepoints(struct svg *svg, struct svg_matrix *matrix, char *text, unsigned int closed);
void svg_fill(struct svg *svg, unsigned int color, unsigned int alpha, unsigned int evenodd);
void svg_stroke(struct svg *svg, struct svg_matrix *matrix, int width, unsigned int cap, unsigned int color, unsigned int alpha);
