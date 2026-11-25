struct paint_canvas
{

    unsigned int *pixels;
    unsigned int width;
    unsigned int height;
    int top;
    unsigned int *heap;

};

unsigned int paint_getbackground(struct layout *layout);
void paint_document(struct paint_canvas *canvas, struct layout *layout);
