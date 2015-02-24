#include <abi.h>
#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct pcx_surface pcx;

void main()
{

    call_open(CALL_P0);
    call_open(CALL_PO);
    pcx_initsurface(&pcx, CALL_P0);
    pcx_load(&pcx);
    gfx_wsurface(CALL_PO, &pcx.base);
    call_close(CALL_PO);
    call_close(CALL_P0);

}

