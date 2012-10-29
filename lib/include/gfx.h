#ifndef LIB_GFX_H
#define LIB_GFX_H

#define GFX_BUFFER_SIZE                 0x1000

enum gfx_surface_type
{

    GFX_RGB16,
    GFX_ARGB16,
    GFX_RGB32,
    GFX_ARGB32

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
    void (*read)(struct gfx_backend *self, unsigned int offset, unsigned int count, void *buffer);
    void (*write)(struct gfx_backend *self, unsigned int offset, unsigned int count, void *buffer);

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
    unsigned int bpp;
    unsigned int type;
    struct gfx_context context;
    struct gfx_backend *backend;

};

void gfx_fill(struct gfx_surface *surface);
void gfx_set_color(struct gfx_surface *surface, unsigned int color);
void gfx_set_color_rgb(struct gfx_surface *self, unsigned char red, unsigned char green, unsigned char blue);
void gfx_set_color_rgba(struct gfx_surface *self, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
void gfx_set_rectangle(struct gfx_surface *surface, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
void gfx_init_backend(struct gfx_backend *backend, unsigned int id);
void gfx_init_context(struct gfx_context *context);
void gfx_init_surface(struct gfx_surface *surface, unsigned int width, unsigned int height, enum gfx_surface_type type, struct gfx_backend *backend);

#endif

