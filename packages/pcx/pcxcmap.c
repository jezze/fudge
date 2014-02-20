#include <fudge.h>

static struct pcx_surface pcx;

void main()
{

    pcx_init_surface(&pcx, CALL_I0);
    pcx_load(&pcx);
    call_write(CALL_O0, 0, 768, pcx.colormap);

}

