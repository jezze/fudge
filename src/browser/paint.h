#define PAINT_SCRATCH                   0x40000

struct paint_canvas
{

    unsigned int *pixels;
    unsigned int width;
    unsigned int height;
    int top;
    unsigned int *heap;
    unsigned int *scratch;
    unsigned int scratchsize;
    int clipleft;
    int cliptop;
    int clipright;
    int clipbottom;

};

struct paint_area
{

    int x;
    int y;
    int w;
    int h;

};

unsigned int paint_getbackground(struct layout *layout);
struct paint_area paint_getcontent(struct layout *layout, unsigned int node);
void paint_document(struct paint_canvas *canvas, struct layout *layout);
