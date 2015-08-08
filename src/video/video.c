#include <abi.h>
#include <fudge.h>
#include "video.h"

void video_getmode(struct ctrl_videosettings *settings)
{

    call_walk(CALL_L0, CALL_PR, 19, "system/video:0/ctrl");
    call_open(CALL_L0);
    call_read(CALL_L0, 0, sizeof (struct ctrl_videosettings), 1, settings);
    call_close(CALL_L0);

}

void video_setmode(struct ctrl_videosettings *settings)
{

    call_walk(CALL_L0, CALL_PR, 19, "system/video:0/ctrl");
    call_open(CALL_L0);
    call_write(CALL_L0, 0, sizeof (struct ctrl_videosettings), 1, settings);
    call_read(CALL_L0, 0, sizeof (struct ctrl_videosettings), 1, settings);
    call_close(CALL_L0);

}

void video_setcolormap(unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    call_walk(CALL_L0, CALL_PR, 23, "system/video:0/colormap");
    call_open(CALL_L0);
    call_write(CALL_L0, offset, size, count, buffer);
    call_close(CALL_L0);

}

void video_open(void)
{

    call_walk(CALL_L0, CALL_PR, 19, "system/video:0/data");
    call_open(CALL_L0);

}

void video_close(void)
{

    call_close(CALL_L0);

}

void video_draw(unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int woff;
    unsigned int wcount;

    for (woff = 0; (wcount = call_write(CALL_L0, offset + woff, 1, count - woff, buffer)); woff += wcount);

}

