#define DEFLATE_OK                      1
#define DEFLATE_FULL                    2
#define DEFLATE_INVALID                 3

struct deflate
{

    unsigned char *out;
    unsigned int ocount;
    unsigned int ooffset;
    unsigned char *in;
    unsigned int icount;
    unsigned int ioffset;
    unsigned int bits;
    unsigned int nbits;
    unsigned int status;

};

unsigned int deflate_decompress(struct deflate *d);
unsigned int deflate_compress(struct deflate *d);
void deflate_init(struct deflate *d, void *out, unsigned int ocount, void *in, unsigned int icount);
