#define PCX_IDENTIFIER                  0x10
#define PCX_VERSION                     0x05
#define PCX_ENCODING                    0x01

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
    short palettetype;
    short xscreen;
    short yscreen;
    char reserved2[54];

};

struct pcx_surface
{

    struct gfx_surface base;
    struct pcx_header header;

};

void pcx_init_surface(struct pcx_surface *surface, unsigned int id, unsigned int width, unsigned int height, enum gfx_surface_bpp bpp);
