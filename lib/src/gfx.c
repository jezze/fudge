#include <gfx.h>
#include <memory.h>

void gfx_draw_surface(struct gfx_surface *out, struct gfx_surface *in)
{

}

void gfx_init_surface(struct gfx_surface *surface, unsigned int x, unsigned int y, enum gfx_surface_type type)
{

}

void gfx_init_memory_surface(struct gfx_memory_surface *surface, void *buffer, unsigned int x, unsigned int y, enum gfx_surface_type type)
{

    gfx_init_surface(&surface->base, x, y, type);

}

void gfx_init_video_surface(struct gfx_memory_surface *surface, unsigned int id, unsigned int x, unsigned int y, enum gfx_surface_type type)
{

    gfx_init_surface(&surface->base, x, y, type);

}

