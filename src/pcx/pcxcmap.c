#include <abi.h>
#include <fudge.h>
#include "gfx.h"
#include "pcx.h"

static struct pcx_surface pcx;

void main(void)
{

    call_open(CALL_PI);
    call_open(CALL_PO);
    pcx_initsurface(&pcx, CALL_PI);
    pcx_load(&pcx);
    file_writeall(CALL_PO, pcx.colormap, 768);
    call_close(CALL_PO);
    call_close(CALL_PI);

}

