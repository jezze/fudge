#include <fudge.h>
#include "gfx.h"
#include "vga.h"

static unsigned int read(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_surface *surface = (struct vga_surface *)self;

    return call_read(surface->id, offset, count, buffer);

}

static unsigned int write(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_surface *surface = (struct vga_surface *)self;

    return call_write(surface->id, offset, count, buffer);

}

void vga_load(struct vga_surface *surface)
{

}

void vga_init_surface(struct vga_surface *surface, unsigned int id)
{

    memory_clear(surface, sizeof (struct vga_surface));
    gfx_init_surface(&surface->base, read, write);

    surface->id = id;

}

