#include <arch.h>
#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct pcx_surface pcx;

void main()
{

    pcx_init_surface(&pcx, CALL_DI);
    pcx_load(&pcx);
    call_write(CALL_DO, 0, 768, pcx.colormap);

}

