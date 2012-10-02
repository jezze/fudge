#include <call.h>
#include <gfx.h>
#include <memory.h>

static void fill(struct gfx_surface *self, unsigned int color)
{

    struct gfx_video_surface *surface = (struct gfx_video_surface *)self;
    char buffer[0x1000];
    unsigned int size = self->width * self->height * self->bpp;
    unsigned int i;

    for (i = 0; i < 0x1000; i += 4)
        memory_copy(buffer + i, &color, 4);

    for (i = 0; i < size; i += 0x1000)
        call_write(surface->id, i, 0x1000, buffer);

}

void gfx_init_surface(struct gfx_surface *surface, unsigned int width, unsigned int height, enum gfx_surface_type type)
{

    memory_clear(surface, sizeof (struct gfx_surface));

    surface->width = width;
    surface->height = height;
    surface->bpp = 4;
    surface->type = type;

}

void gfx_init_memory_surface(struct gfx_memory_surface *surface, void *buffer, unsigned int width, unsigned int height, enum gfx_surface_type type)
{

    memory_clear(surface, sizeof (struct gfx_memory_surface));

    gfx_init_surface(&surface->base, width, height, type);

    surface->buffer = buffer;

}

void gfx_init_video_surface(struct gfx_video_surface *surface, unsigned int id, unsigned int width, unsigned int height, enum gfx_surface_type type)
{

    memory_clear(surface, sizeof (struct gfx_video_surface));

    gfx_init_surface(&surface->base, width, height, type);

    surface->id = id;
    surface->base.fill = fill;

}

