#include <abi.h>
#include <fudge.h>
#include "video.h"

void video_setmode()
{

    call_walk(CALL_L0, CALL_PR, 19, "system/video:0/ctrl");
    call_open(CALL_L0);
    call_write(CALL_L0, 0, 0, 0, 0);
    call_close(CALL_L0);

}

void video_setcolormap(void *colormap)
{

    call_walk(CALL_L0, CALL_PR, 23, "system/video:0/colormap");
    call_open(CALL_L0);
    call_write(CALL_L0, 0, 1, 27, colormap);
    call_close(CALL_L0);

}

void video_open()
{

    call_walk(CALL_L0, CALL_PR, 19, "system/video:0/data");
    call_open(CALL_L0);

}

void video_close()
{

    call_close(CALL_L0);

}

void video_draw(unsigned int offset, unsigned int count, void *buffer)
{

    call_write(CALL_L0, offset, 1, count, buffer);

}

