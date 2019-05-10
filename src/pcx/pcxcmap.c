#include <fudge.h>
#include <abi.h>
#include "gfx.h"
#include "pcx.h"

static struct pcx_surface pcx;

void init(void)
{

    event_initsignals();

}

void main(void)
{

    file_open(FILE_P0);
    file_open(FILE_P1);
    pcx_initsurface(&pcx, FILE_P0);
    pcx_load(&pcx);
    file_writeall(FILE_P1, pcx.colormap, 768);
    file_close(FILE_P1);
    file_close(FILE_P0);

}

