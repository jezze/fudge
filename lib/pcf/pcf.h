#define PCF_TYPE_PROPERTIES             (1 << 0)
#define PCF_TYPE_ACCELERATORS           (1 << 1)
#define PCF_TYPE_METRICS                (1 << 2)
#define PCF_TYPE_BITMAPS                (1 << 3)
#define PCF_TYPE_INKMETRICS             (1 << 4)
#define PCF_TYPE_BDFENCODINGS           (1 << 5)
#define PCF_TYPE_SWIDTHS                (1 << 6)
#define PCF_TYPE_GLYPHNAMES             (1 << 7)
#define PCF_TYPE_BDFACCELERATORS        (1 << 8)
#define PCF_FORMAT_DEFAULT              0x00000000
#define PCF_FORMAT_INKBOUNDS            0x00000200
#define PCF_FORMAT_ACCELINKBOUNDS       0x00000100
#define PCF_FORMAT_COMPRESSED           0x00000100
#define PCF_FORMAT_PADMASK              (3 << 0)
#define PCF_FORMAT_BYTEMASK             (1 << 2)
#define PCF_FORMAT_BITMASK              (1 << 3)
#define PCF_FORMAT_UNITMASK             (3 << 4)

#define BSWAP16(x)                      (((x) >> 8) | ((x) << 8))
#define BSWAP32(x)                      (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

struct pcf_header
{

    unsigned char magic[4];
    unsigned int entries;

};

struct pcf_entry
{

    unsigned int type;
    unsigned int format;
    unsigned int size;
    unsigned int offset;

};

struct pcf_bitmap
{

    unsigned int count;

};

struct pcf_bdfencoding
{

    unsigned short mincharorbyte2;
    unsigned short maxcharorbyte2;
    unsigned short minbyte1;
    unsigned short maxbyte1;
    unsigned short defaultchar;

};

struct pcf_metrics
{

    unsigned int count;

};

struct pcf_metrics_compressed
{

    unsigned short count;

};

struct pcf_metricsdata
{

    unsigned short lbearing;
    unsigned short rbearing;
    unsigned short width;
    unsigned short ascent;
    unsigned short descent;
    unsigned short attributes;

};

struct pcf_metricsdata_compressed
{

    unsigned char lbearing;
    unsigned char rbearing;
    unsigned char width;
    unsigned char ascent;
    unsigned char descent;

};

void pcf_readbitmap(void *base, struct pcf_bitmap *data);
unsigned int pcf_getbitmapoffset(void *base, unsigned short index);
unsigned int *pcf_getbitmapsizes(void *base);
unsigned char *pcf_getbitmapdata(void *base);
unsigned int pcf_getbitmapalign(void *base);
void pcf_readmetricsdata(void *base, unsigned int index, struct pcf_metricsdata *data);
void pcf_readbdfencoding(void *base, struct pcf_bdfencoding *data);
unsigned short pcf_getindex(void *base, unsigned short encoding);
