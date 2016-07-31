#define GFX_R01G01B01                   0x00010101
#define GFX_R02G02B02                   0x00020202
#define GFX_R04G04B04                   0x00040404
#define GFX_R06G06B06                   0x00060606
#define GFX_R08G08B08                   0x00080808
#define GFX_R12G12B12                   0x000C0C0C
#define GFX_R16G16B16                   0x000F0F0F
#define GFX_R24G24B24                   0x00181818
#define GFX_R32G32B32                   0x00FFFFFF
#define GFX_PRIMITIVE_NONE              0
#define GFX_PRIMITIVE_RECTANGLE         1
#define GFX_PRIMITIVE_CIRCLE            2
#define GFX_PRIMITIVE_TRIANGLE          3

struct gfx_context
{

    unsigned int color;
    unsigned int primitive;
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

};

struct gfx_surface
{

    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    struct gfx_context context;
    unsigned int (*read)(struct gfx_surface *self, void *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(struct gfx_surface *self, void *buffer, unsigned int count, unsigned int offset);

};

void gfx_fill(struct gfx_surface *surface);
void gfx_setcolor(struct gfx_surface *surface, unsigned int color);
void gfx_setcolorrgb(struct gfx_surface *self, unsigned char red, unsigned char green, unsigned char blue);
void gfx_setcolorrgba(struct gfx_surface *self, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
void gfx_setrectangle(struct gfx_surface *surface, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
void gfx_wsurface(unsigned int id, struct gfx_surface *in);
void gfx_initcontext(struct gfx_context *context);
void gfx_initsurface(struct gfx_surface *surface, unsigned int (*read)(struct gfx_surface *self, void *buffer, unsigned int count, unsigned int offset), unsigned int (*write)(struct gfx_surface *self, void *buffer, unsigned int count, unsigned int offset));
