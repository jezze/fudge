#include <abi.h>
#include <fudge.h>
#include "gfx.h"
#include "pcx.h"

static struct pcx_surface pcx;

void main(void)
{

    file_open(FILE_PI);
    file_open(FILE_PO);
    pcx_initsurface(&pcx, FILE_PI);
    pcx_load(&pcx);
    gfx_wsurface(FILE_PO, &pcx.base);
    file_close(FILE_PO);
    file_close(FILE_PI);

}

