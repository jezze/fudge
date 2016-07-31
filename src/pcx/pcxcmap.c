#include <abi.h>
#include <fudge.h>
#include "gfx.h"
#include "pcx.h"

static struct pcx_surface pcx;

void main(void)
{

    file_open(CALL_PI);
    file_open(CALL_PO);
    pcx_initsurface(&pcx, CALL_PI);
    pcx_load(&pcx);
    file_writeall(CALL_PO, pcx.colormap, 768);
    file_close(CALL_PO);
    file_close(CALL_PI);

}

