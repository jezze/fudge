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
    file_writeall(FILE_PO, pcx.colormap, 768);
    file_close(FILE_PO);
    file_close(FILE_PI);

}

