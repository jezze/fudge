#define PCX_IDENTIFIER                  0x10
#define PCX_VERSION                     0x05
#define PCX_ENCODING                    0x01
#define PCX_COLORMAP_MAGIC              0x0C
#define PCX_COLORMAP_SLOTS              256

struct pcx_header
{

    char identifier;
    char version;
    char encoding;
    char bpp;
    short xstart;
    short ystart;
    short xend;
    short yend;
    short xdpi;
    short ydpi;
    char palette[48];
    char reserved1[1];
    char nplanes;
    short bpl;
    short palettemode;
    short xscreen;
    short yscreen;
    char reserved2[54];

};

unsigned int pcx_readline(unsigned char *raw, unsigned int width, unsigned char *buffer);
