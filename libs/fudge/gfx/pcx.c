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

unsigned int pcx_read(struct pcx_surface *surface, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned char raw[FUDGE_BSIZE];
    unsigned int width = surface->header.xend - surface->header.xstart + 1;
    unsigned int rindex = 0;
    unsigned int oindex = 0;
    unsigned int repeat;
    unsigned char current;

    call_read(surface->id, offset, count, raw);

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

void pcx_load(struct pcx_surface *surface)
{

    unsigned char magic;

    call_read(FUDGE_IN, 0, sizeof (struct pcx_header), &surface->header);
    call_read(surface->id, size(surface->id) - 768 - 1, 1, &magic);

    if (magic != PCX_COLORMAP_MAGIC)
        return;

    call_read(surface->id, size(surface->id) - 768, 768, surface->colormap);

}

void pcx_init_surface(struct pcx_surface *surface, unsigned int id)
{

    memory_clear(surface, sizeof (struct pcx_surface));

    surface->id = id;

}

