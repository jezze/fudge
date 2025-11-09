#define PCF_MAGIC                       "\1fcp"
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

struct pcf_bdfencoding
{

    unsigned short mincharorbyte2;
    unsigned short maxcharorbyte2;
    unsigned short minbyte1;
    unsigned short maxbyte1;
    unsigned short defaultchar;

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

unsigned short pcf_convert16(unsigned short value, unsigned int format);
unsigned int pcf_convert32(unsigned int value, unsigned int format);
struct pcf_entry *pcf_findentry(struct pcf_entry *entries, unsigned int count, unsigned int type);
unsigned int pcf_getbdfoffset(struct pcf_entry *entry, struct pcf_bdfencoding *bdfencoding, unsigned int format, unsigned short encoding);
