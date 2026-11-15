#define TOFFEE_SHAPES                   32
#define TOFFEE_POINTS                   512
#define TOFFEE_CROSSINGS                64
#define TOFFEE_SPANS                    (TOFFEE_CROSSINGS / 2)
#define TOFFEE_SLOTS                    10
#define TOFFEE_GRID                     4096
#define TOFFEE_SIZE                     2048
#define TOFFEE_SUBPIXELS                16

struct toffee_point
{

    int x;
    int y;
    unsigned int move;

};

struct toffee_shape
{

    unsigned int color;
    unsigned int slot;
    unsigned int start;
    unsigned int count;

};

struct toffee
{

    unsigned int width;
    unsigned int height;
    unsigned int nshapes;
    unsigned int npoints;
    struct toffee_shape shapes[TOFFEE_SHAPES];
    struct toffee_point points[TOFFEE_POINTS];

};

struct toffee_span
{

    int x0;
    int x2;

};

struct toffee_line
{

    unsigned int ncrossings;
    unsigned int count;
    int crossings[TOFFEE_CROSSINGS];
    struct toffee_span spans[TOFFEE_SPANS];

};

unsigned int toffee_parse(struct toffee *toffee, char *text, unsigned int count);
void toffee_scanline(struct toffee *toffee, struct toffee_line *scan, unsigned int index, unsigned int width, unsigned int height, unsigned int line);
