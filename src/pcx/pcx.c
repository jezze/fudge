#include <abi.h>
#include <fudge.h>
#include <format/pcx.h>
#include <video/gfx.h>
#include "pcx.h"

static unsigned int size(unsigned int id)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count = 0;

    while ((count += call_read(id, FUDGE_BSIZE, buffer)));

    return count;

}

static unsigned int surface_read(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pcx_surface *surface = (struct pcx_surface *)self;
    unsigned char raw[FUDGE_BSIZE];
    unsigned int width = surface->header.xend - surface->header.xstart + 1;
    unsigned int rindex = 0;
    unsigned int oindex = 0;

    call_seek(surface->id, offset + 128);
    call_read(surface->id, count, raw);

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

static unsigned int surface_write(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void pcx_load(struct pcx_surface *surface)
{

    unsigned char magic;

    call_read(surface->id, sizeof (struct pcx_header), &surface->header);

    surface->base.width = surface->header.xend - surface->header.xstart + 1;
    surface->base.height = surface->header.yend - surface->header.ystart + 1;
    surface->base.bpp = GFX_R24G24B24;

    call_seek(surface->id, size(surface->id) - 768 - 1);
    call_read(surface->id, 1, &magic);

    if (magic != PCX_COLORMAP_MAGIC)
        return;

    call_seek(surface->id, size(surface->id) - 768);
    call_read(surface->id, 768, surface->colormap);

}

void pcx_initsurface(struct pcx_surface *surface, unsigned int id)
{

    memory_clear(surface, sizeof (struct pcx_surface));
    gfx_initsurface(&surface->base, surface_read, surface_write);

    surface->id = id;

}

