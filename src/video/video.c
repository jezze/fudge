#include <abi.h>
#include <fudge.h>
#include "video.h"

void video_getmode(unsigned int descriptor, struct ctrl_videosettings *settings)
{

    call_walk(descriptor, CALL_PR, 19, "system/video:0/ctrl");
    call_open(descriptor);
    call_read(descriptor, 0, sizeof (struct ctrl_videosettings), 1, settings);
    call_close(descriptor);

}

void video_setmode(unsigned int descriptor, struct ctrl_videosettings *settings)
{

    call_walk(descriptor, CALL_PR, 19, "system/video:0/ctrl");
    call_open(descriptor);
    call_write(descriptor, 0, sizeof (struct ctrl_videosettings), 1, settings);
    call_read(descriptor, 0, sizeof (struct ctrl_videosettings), 1, settings);
    call_close(descriptor);

}

void video_setcolormap(unsigned int descriptor, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    call_walk(descriptor, CALL_PR, 23, "system/video:0/colormap");
    call_open(descriptor);
    call_write(descriptor, offset, size, count, buffer);
    call_close(descriptor);

}

void video_open(unsigned int descriptor)
{

    call_walk(descriptor, CALL_PR, 19, "system/video:0/data");
    call_open(descriptor);

}

void video_close(unsigned int descriptor)
{

    call_close(descriptor);

}

void video_draw(unsigned int descriptor, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int woff;
    unsigned int wcount;

    for (woff = 0; (wcount = call_write(descriptor, offset + woff, 1, count - woff, buffer)); woff += wcount);

}

