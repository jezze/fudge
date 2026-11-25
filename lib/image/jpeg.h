#define JPEG_COMPONENTS                 3
#define JPEG_TABLES                     4
#define JPEG_BLOCK                      64
#define JPEG_SYMBOLS                    256
#define JPEG_LENGTHS                    17
#define JPEG_FASTBITS                   9
#define JPEG_FAST                       (1 << JPEG_FASTBITS)
#define JPEG_INPUT                      1024
#define JPEG_WIDTH                      4096
#define JPEG_PLANE                      ((JPEG_WIDTH + 16) * 16)

struct jpeg_huffman
{

    int maxcode[JPEG_LENGTHS];
    unsigned int mincode[JPEG_LENGTHS];
    unsigned int first[JPEG_LENGTHS];
    unsigned char values[JPEG_SYMBOLS];
    unsigned short fast[JPEG_FAST];

};

struct jpeg_component
{

    unsigned int id;
    unsigned int h;
    unsigned int v;
    unsigned int quant;
    unsigned int dc;
    unsigned int ac;
    unsigned int stride;
    unsigned int blocks;
    unsigned int rows;
    unsigned int offset;
    int predictor;

};

/* read reads count bytes of the file at offset; a progressive file, or one with a scan per component, needs needed bytes given to jpeg_decode */
struct jpeg
{

    unsigned int (*read)(struct jpeg *jpeg, void *buffer, unsigned int count, unsigned int offset);
    unsigned int width;
    unsigned int height;
    unsigned int ncomponents;
    unsigned int hmax;
    unsigned int vmax;
    unsigned int mcuwidth;
    unsigned int mcuheight;
    unsigned int mcus;
    unsigned int mcurows;
    unsigned int progressive;
    unsigned int buffered;
    unsigned int needed;
    unsigned int scancount;
    unsigned int scan[JPEG_COMPONENTS];
    unsigned int start;
    unsigned int end;
    unsigned int high;
    unsigned int low;
    unsigned int eobrun;
    unsigned int restart;
    unsigned int restartleft;
    unsigned int transform;
    unsigned int offset;
    unsigned int inputstart;
    unsigned int inputcount;
    unsigned int bits;
    unsigned int bitcount;
    unsigned int marker;
    unsigned int row;
    unsigned int line;
    struct jpeg_component components[JPEG_COMPONENTS];
    unsigned short quant[JPEG_TABLES][JPEG_BLOCK];
    int block[JPEG_BLOCK];
    short *coefficients;
    struct jpeg_huffman dc[JPEG_TABLES];
    struct jpeg_huffman ac[JPEG_TABLES];
    unsigned char input[JPEG_INPUT];
    unsigned char planes[JPEG_COMPONENTS][JPEG_PLANE];

};

unsigned int jpeg_open(struct jpeg *jpeg, unsigned int (*read)(struct jpeg *jpeg, void *buffer, unsigned int count, unsigned int offset));
unsigned int jpeg_decode(struct jpeg *jpeg, short *coefficients);
unsigned int jpeg_readrow(struct jpeg *jpeg, unsigned int *pixels);
