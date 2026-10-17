#define PNG_PALETTE                     256
#define PNG_WINDOW                      32768
#define PNG_INPUT                       1024
#define PNG_LINE                        8192
#define PNG_CHECKPOINTS                 8

/* the decoder as it was after row rows: the unread input is read again from offset */
struct png_checkpoint
{

    struct deflate deflate;
    unsigned int offset;
    unsigned int left;
    unsigned int row;
    unsigned int saved;
    unsigned char window[PNG_WINDOW];
    unsigned char line[PNG_LINE];

};

/* deflate comes first so the refill finds the png; read reads count bytes of the file at offset */
struct png
{

    struct deflate deflate;
    unsigned int (*read)(struct png *png, void *buffer, unsigned int count, unsigned int offset);
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int colortype;
    unsigned int channels;
    unsigned int pixelsize;
    unsigned int stride;
    unsigned int npalette;
    unsigned int palette[PNG_PALETTE];
    unsigned int haskey;
    unsigned int key[3];
    unsigned int start;
    unsigned int startleft;
    unsigned int offset;
    unsigned int left;
    unsigned int row;
    struct png_checkpoint *checkpoints[PNG_CHECKPOINTS];
    unsigned int ncheckpoints;
    unsigned char window[PNG_WINDOW];
    unsigned char input[PNG_INPUT];
    unsigned char line[PNG_LINE];

};

unsigned int png_readheader(struct png *png, void *in, unsigned int icount);
unsigned int png_open(struct png *png, unsigned int (*read)(struct png *png, void *buffer, unsigned int count, unsigned int offset));
void png_setcheckpoints(struct png *png, struct png_checkpoint **checkpoints, unsigned int count);
unsigned int png_readrow(struct png *png, unsigned int row, unsigned int *pixels);
