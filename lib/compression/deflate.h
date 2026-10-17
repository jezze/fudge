#define DEFLATE_OK                      1
#define DEFLATE_FULL                    2
#define DEFLATE_INVALID                 3
#define DEFLATE_COUNTS                  16
#define DEFLATE_LITERALS                288
#define DEFLATE_FASTBITS                9

struct deflate_huffman
{

    unsigned short count[DEFLATE_COUNTS];
    unsigned short symbol[DEFLATE_LITERALS];
    unsigned short fast[1 << DEFLATE_FASTBITS];

};

/* with ring set, out is a window of ocount bytes (a power of two) that the output wraps around, and ooffset counts all output; refill, when set, is called for more input and returns how much it put in in */
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
    unsigned int ring;
    unsigned int (*refill)(struct deflate *d);
    unsigned int last;
    unsigned int block;
    unsigned int length;
    unsigned int distance;
    struct deflate_huffman literals;
    struct deflate_huffman distances;

};

unsigned int deflate_inflate(struct deflate *d, unsigned int count);
unsigned int deflate_decompress(struct deflate *d);
unsigned int deflate_compress(struct deflate *d);
void deflate_init(struct deflate *d, void *out, unsigned int ocount, void *in, unsigned int icount);
void deflate_initring(struct deflate *d, void *window, unsigned int count, unsigned int (*refill)(struct deflate *d));
