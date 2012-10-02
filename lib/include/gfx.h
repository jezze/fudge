#ifndef LIB_GFX_H
#define LIB_GFX_H

enum gfx_surface_type
{

    RGB16,
    ARGB16,
    RGB32,
    ARGB32

};

struct gfx_context
{

    unsigned int color;

};

struct gfx_backend
{

    unsigned int id;
    void (*read)(struct gfx_backend *backend, unsigned int offset, unsigned int count, void *buffer);
    void (*write)(struct gfx_backend *backend, unsigned int offset, unsigned int count, void *buffer);

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

struct gfx_window
{

    unsigned int x;
    unsigned int y;
    struct gfx_surface *surface;

};

void gfx_set_color(struct gfx_surface *surface, unsigned int color);
void gfx_fill(struct gfx_surface *surface);
void gfx_init_backend(struct gfx_backend *backend, unsigned int id);
void gfx_init_surface(struct gfx_surface *surface, unsigned int width, unsigned int height, enum gfx_surface_type type, struct gfx_backend *backend);
void gfx_init_window(struct gfx_window *window, unsigned int x, unsigned int y, struct gfx_surface *surface);

#endif

