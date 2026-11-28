struct js_canvasop;
struct font_set;

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
void svg_render(struct svg_canvas *canvas, struct html_document *document, struct css_sheet *sheet, unsigned int root, unsigned int color, unsigned int fill);
void svg_rendercanvas(struct svg_canvas *canvas, struct js_canvasop *ops, unsigned int count, unsigned int node, unsigned int width, unsigned int height, char *strings, struct font_set *fonts);