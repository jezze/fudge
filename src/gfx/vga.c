#include <abi.h>
#include <fudge.h>
#include "gfx.h"
#include "vga.h"

static unsigned int surface_read(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_surface *surface = (struct vga_surface *)self;

    return call_read(surface->id, offset, 1, count, buffer);

}

static unsigned int surface_write(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_surface *surface = (struct vga_surface *)self;

    return call_write(surface->id, offset, 1, count, buffer);

}

void vga_load(struct vga_surface *surface)
{

}

void vga_initsurface(struct vga_surface *surface, unsigned int id)
{

    memory_clear(surface, sizeof (struct vga_surface));
    gfx_initsurface(&surface->base, surface_read, surface_write);

    surface->id = id;

}

