#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct pcx_surface pcx;

void main()
{

    call_open(CALL_I0);
    pcx_init_surface(&pcx, CALL_I0);
    call_close(CALL_I0);
    pcx_load(&pcx);
    call_open(CALL_O0);
    gfx_write_surface(CALL_O0, &pcx.base);
    call_close(CALL_O0);

}

