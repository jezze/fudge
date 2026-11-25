struct svg_canvas
{

    unsigned int *pixels;
    unsigned int width;
    unsigned int height;

};

struct svg_size
{

    unsigned int width;
    unsigned int height;
    unsigned int ratiowidth;
    unsigned int ratioheight;

};

unsigned int svg_findroot(struct html_document *document);
struct svg_size svg_measure(struct html_document *document, unsigned int root, unsigned int fontsize);
void svg_render(struct svg_canvas *canvas, struct html_document *document, unsigned int root, unsigned int color, unsigned int fill);
