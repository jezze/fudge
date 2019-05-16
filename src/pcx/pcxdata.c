#include <fudge.h>
#include <abi.h>
#include "gfx.h"
#include "pcx.h"

static struct pcx_surface pcx;

void init(void)
{

}

void main(void)
{

    file_open(FILE_P0);
    file_open(FILE_P1);
    pcx_initsurface(&pcx, FILE_P0);
    pcx_load(&pcx);
    gfx_wsurface(FILE_P1, &pcx.base);
    file_close(FILE_P1);
    file_close(FILE_P0);

}

