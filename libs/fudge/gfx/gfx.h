enum gfx_surface_bpp
{

    GFX_R01G01B01                       = 0x00010101,
    GFX_R02G02B02                       = 0x00020202,
    GFX_R04G04B04                       = 0x00040404,
    GFX_R06G06B06                       = 0x00060606,
    GFX_R08G08B08                       = 0x00080808,
    GFX_R12G12B12                       = 0x000C0C0C,
    GFX_R16G16B16                       = 0x000F0F0F,
    GFX_R24G24B24                       = 0x00181818,
    GFX_R32G32B32                       = 0x00FFFFFF

};

enum gfx_primitive_type
{

    GFX_NONE,
    GFX_RECTANGLE,
    GFX_CIRCLE,
    GFX_TRIANGLE

};

struct gfx_backend
{

    unsigned int id;
    unsigned int (*read)(struct gfx_backend *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct gfx_backend *self, unsigned int offset, unsigned int count, void *buffer);

};

struct gfx_context
{

    unsigned int color;
    enum gfx_primitive_type primitive;
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

};

struct gfx_surface
{

    unsigned int width;
    unsigned int height;
    enum gfx_surface_bpp bpp;
    struct gfx_context context;
    struct gfx_backend backend;

};

void gfx_fill(struct gfx_surface *surface);
void gfx_set_color(struct gfx_surface *surface, unsigned int color);
void gfx_set_color_rgb(struct gfx_surface *self, unsigned char red, unsigned char green, unsigned char blue);
void gfx_set_color_rgba(struct gfx_surface *self, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
void gfx_set_rectangle(struct gfx_surface *surface, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
void gfx_init_backend(struct gfx_backend *backend, unsigned int id);
void gfx_init_context(struct gfx_context *context);
void gfx_init_surface(struct gfx_surface *surface, unsigned int id, unsigned int width, unsigned int height, enum gfx_surface_bpp bpp);
