#ifndef LIB_GFX_H
#define LIB_GFX_H

enum gfx_surface_type
{

    RGB16,
    ARGB16,
    RGB32,
    ARGB32

};

struct gfx_surface
{

    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    unsigned int type;
    void (*fill)(struct gfx_surface *self, unsigned int color);

};

struct gfx_memory_surface
{

    struct gfx_surface base;
    void *buffer;

};

struct gfx_video_surface
{

    struct gfx_surface base;
    unsigned int id;

};

void gfx_draw_surface(struct gfx_surface *out, struct gfx_surface *in);
void gfx_init_surface(struct gfx_surface *surface, unsigned int width, unsigned int height, enum gfx_surface_type type);
void gfx_init_memory_surface(struct gfx_memory_surface *surface, void *buffer, unsigned int width, unsigned int height, enum gfx_surface_type type);
void gfx_init_video_surface(struct gfx_video_surface *surface, unsigned int id, unsigned int width, unsigned int height, enum gfx_surface_type type);

#endif

