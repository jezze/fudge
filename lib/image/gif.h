#define GIF_PALETTE                     256
#define GIF_CODES                       4096
#define GIF_INPUT                       1024

/* read reads count bytes of the file at offset; only the first frame is decoded */
struct gif
{

    unsigned int (*read)(struct gif *gif, void *buffer, unsigned int count, unsigned int offset);
    unsigned int width;
    unsigned int height;
    unsigned int left;
    unsigned int top;
    unsigned int framewidth;
    unsigned int frameheight;
    unsigned int interlaced;
    unsigned int transparent;
    unsigned int palette[GIF_PALETTE];
    unsigned int offset;
    unsigned int position;
    unsigned int count;
    unsigned int blockleft;
    unsigned short prefix[GIF_CODES];
    unsigned char suffix[GIF_CODES];
    unsigned char stack[GIF_CODES + 1];
    unsigned char input[GIF_INPUT];

};

unsigned int gif_open(struct gif *gif, unsigned int (*read)(struct gif *gif, void *buffer, unsigned int count, unsigned int offset));
unsigned int gif_decode(struct gif *gif, unsigned int *pixels);
