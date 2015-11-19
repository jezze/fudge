#include <abi.h>
#include <fudge.h>
#include <lib/file.h>
#include <lib/gfx.h>
#include "pcx.h"

static struct pcx_surface pcx;

void main(void)
{

    call_open(CALL_PI);
    call_open(CALL_PO);
    pcx_initsurface(&pcx, CALL_PI);
    pcx_load(&pcx);
    gfx_wsurface(CALL_PO, &pcx.base);
    call_close(CALL_PO);
    call_close(CALL_PI);

}

