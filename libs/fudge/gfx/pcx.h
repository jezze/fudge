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

    struct pcx_header header;
    struct gfx_context context;
    struct gfx_backend *backend;

};

void pcx_init_surface(struct pcx_surface *surface, unsigned int width, unsigned int height, enum gfx_surface_type type, struct gfx_backend *backend);
