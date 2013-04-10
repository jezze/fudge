#include "../call.h"
#include "../define.h"
#include "../memory.h"
#include "gfx.h"
#include "pcx.h"

static unsigned int size(unsigned int id)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;

    for (offset = 0; (count = call_read(id, offset, FUDGE_BSIZE, buffer)); offset += count);

    return offset;

}

static unsigned int read(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct pcx_surface *surface = (struct pcx_surface *)self;
    unsigned char raw[FUDGE_BSIZE];
    unsigned int width = surface->header.xend - surface->header.xstart + 1;
    unsigned int rindex = 0;
    unsigned int oindex = 0;
    unsigned int repeat;
    unsigned char current;

    call_read(surface->id, offset + 128, count, raw);

    do
    {

        repeat = 1;
        current = raw[rindex];
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

static unsigned int write(struct gfx_surface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void pcx_load(struct pcx_surface *surface)
{

    unsigned char magic;

    call_read(FUDGE_DI, 0, sizeof (struct pcx_header), &surface->header);

    surface->base.width = surface->header.xend - surface->header.xstart + 1;
    surface->base.height = surface->header.yend - surface->header.ystart + 1;
    surface->base.bpp = GFX_R24G24B24;

    call_read(surface->id, size(surface->id) - 768 - 1, 1, &magic);

    if (magic != PCX_COLORMAP_MAGIC)
        return;

    call_read(surface->id, size(surface->id) - 768, 768, surface->colormap);

}

void pcx_init_surface(struct pcx_surface *surface, unsigned int id)
{

    memory_clear(surface, sizeof (struct pcx_surface));
    gfx_init_surface(&surface->base, read, write);

    surface->id = id;

}

