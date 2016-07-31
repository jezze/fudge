#include <abi.h>
#include <fudge.h>
#include "gfx.h"
#include "pcx.h"

static unsigned int size(unsigned int id)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count = 0;

    while ((count += file_read(id, buffer, FUDGE_BSIZE)));

    return count;

}

static unsigned int surface_read(struct gfx_surface *self, void *buffer, unsigned int count, unsigned int offset)
{

    struct pcx_surface *surface = (struct pcx_surface *)self;
    unsigned char raw[FUDGE_BSIZE];
    unsigned int width = surface->header.xend - surface->header.xstart + 1;
    unsigned int rindex = 0;
    unsigned int oindex = 0;

    file_seekread(surface->id, raw, count, offset + 128);

    do
    {

        unsigned int repeat = 1;
        unsigned char current = raw[rindex];
        rindex++;

        if ((current & 0xC0) == 0xC0)
        {

            repeat = current & 0x3F;
            current = raw[rindex];
            rindex++;

        }

        while (repeat--)
        {

            ((char *)buffer)[oindex] = current;
            oindex++;

        }

    } while (oindex < width);

    return rindex;

}

static unsigned int surface_write(struct gfx_surface *self, void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

void pcx_load(struct pcx_surface *surface)
{

    unsigned char magic;

    file_seekreadall(surface->id, &surface->header, sizeof (struct pcx_header), 0);

    surface->base.width = surface->header.xend - surface->header.xstart + 1;
    surface->base.height = surface->header.yend - surface->header.ystart + 1;
    surface->base.bpp = GFX_R24G24B24;

    file_seekreadall(surface->id, &magic, 1, size(surface->id) - 768 - 1);

    if (magic != PCX_COLORMAP_MAGIC)
        return;

    file_seekreadall(surface->id, surface->colormap, 768, size(surface->id) - 768);

}

void pcx_initsurface(struct pcx_surface *surface, unsigned int id)
{

    memory_clear(surface, sizeof (struct pcx_surface));
    gfx_initsurface(&surface->base, surface_read, surface_write);

    surface->id = id;

}

