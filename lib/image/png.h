#define PNG_PALETTE                     256

struct png
{

    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int colortype;
    unsigned int channels;
    unsigned int pixelsize;
    unsigned int stride;
    unsigned int size;
    unsigned int npalette;
    unsigned int palette[PNG_PALETTE];
    unsigned int haskey;
    unsigned int key[3];
    unsigned char *data;

};

unsigned int png_readheader(struct png *png, void *in, unsigned int icount);
unsigned int png_decode(struct png *png, void *out, unsigned int ocount, void *in, unsigned int icount);
void png_readrow(struct png *png, unsigned int row, unsigned int *pixels);
