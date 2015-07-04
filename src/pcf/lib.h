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

} __attribute__((packed));

struct pcf_entry
{

    unsigned int type;
    unsigned int format;
    unsigned int size;
    unsigned int offset;

} __attribute__((packed));

struct pcf_bitmap
{

    unsigned int format;
    unsigned int count;

};

struct pcf_bdfencoding
{

    unsigned int format;
    unsigned short mincharorbyte2;
    unsigned short maxcharorbyte2;
    unsigned short minbyte1;
    unsigned short maxbyte1;
    unsigned short defaultchar;

} __attribute__((packed));

struct pcf_metrics
{

    unsigned int format;

} __attribute__((packed));

struct pcf_metrics_normal
{

    struct pcf_metrics header;
    unsigned int count;

} __attribute__((packed));

struct pcf_metrics_compressed
{

    struct pcf_metrics header;
    unsigned short count;

} __attribute__((packed));

struct pcf_metricsdata_normal
{

    unsigned short lbearing;
    unsigned short rbearing;
    unsigned short width;
    unsigned short ascent;
    unsigned short descent;
    unsigned short attributes;

} __attribute__((packed));

struct pcf_metricsdata_compressed
{

    unsigned char lbearing;
    unsigned char rbearing;
    unsigned char width;
    unsigned char ascent;
    unsigned char descent;

} __attribute__((packed));

unsigned short pcf_getindex(unsigned short encoding);
unsigned int pcf_getascent(unsigned short index);
unsigned int pcf_getdescent(unsigned short index);
unsigned int pcf_getpadding();
void *pcf_getdata(unsigned short index);
void pcf_load(unsigned int descriptor);
