#include <fudge.h>

static struct pcx_surface pcx;

void main()
{

    pcx_init_surface(&pcx, CALL_I0);
    pcx_load(&pcx);
    gfx_write_surface(CALL_O0, &pcx.base);

}

