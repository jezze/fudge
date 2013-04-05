#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct pcx_surface pcx;

void main()
{

    pcx_init_surface(&pcx, FUDGE_IN);
    pcx_load(&pcx);
    call_write(FUDGE_OUT, 0, 768, pcx.colormap);

}

