#include "../call.h"
#include "../define.h"
#include "../memory.h"
#include "gfx.h"
#include "vga.h"

static unsigned int read(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void vga_load(struct vga_surface *surface)
{

}

void vga_init_surface(struct vga_surface *surface)
{

    memory_clear(surface, sizeof (struct vga_surface));
    gfx_init_surface(&surface->base, read, write);

}

