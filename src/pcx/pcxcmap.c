#include <abi.h>
#include <fudge.h>
#include <format/pcx.h>
#include <video/gfx.h>
#include "pcx.h"

static struct pcx_surface pcx;

void main(void)
{

    call_open(CALL_PI);
    call_open(CALL_PO);
    pcx_initsurface(&pcx, CALL_PI);
    pcx_load(&pcx);
    call_write(CALL_PO, 768, pcx.colormap);
    call_close(CALL_PO);
    call_close(CALL_PI);

}

