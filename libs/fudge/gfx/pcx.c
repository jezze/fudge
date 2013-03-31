#include "../call.h"
#include "../memory.h"
#include "gfx.h"
#include "pcx.h"

void pcx_init_surface(struct pcx_surface *surface, unsigned int id, unsigned int width, unsigned int height, enum gfx_surface_bpp bpp)
{

    memory_clear(surface, sizeof (struct pcx_surface));
    gfx_init_surface(&surface->base, id, width, height, bpp);

}

