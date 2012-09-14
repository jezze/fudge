#ifndef LIB_GFX_H
#define LIB_GFX_H

enum gfx_surface_type
{

    RGB8,
    ARGB8,
    RGB16,
    ARGB16,
    RGB24,
    ARGB24,
    RGB32,
    ARGB32

};

struct gfx_surface
{

    unsigned int x;
    unsigned int y;
    unsigned int type;

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
void gfx_init_surface(struct gfx_surface *surface, unsigned int x, unsigned int y, enum gfx_surface_type type);
void gfx_memory_surface(struct gfx_surface *surface, void *buffer, unsigned int x, unsigned int y, enum gfx_surface_type type);
void gfx_video_surface(struct gfx_surface *surface, unsigned int id, unsigned int x, unsigned int y, enum gfx_surface_type type);

#endif

