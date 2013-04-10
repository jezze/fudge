#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct pcx_surface pcx;

void main()
{

    pcx_init_surface(&pcx, FUDGE_DI);
    pcx_load(&pcx);
    call_write(FUDGE_DO, 0, 768, pcx.colormap);

}

