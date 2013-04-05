#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct pcx_surface pcx;

void main()
{

    unsigned char colormap[768];

    pcx_init_surface(&pcx, FUDGE_IN);
    pcx_load(&pcx);
    gfx_convert_colormap(colormap, pcx.colormap, 768);
    call_write(FUDGE_OUT, 0, 768, colormap);

}

