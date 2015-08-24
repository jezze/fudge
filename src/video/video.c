#include <abi.h>
#include <fudge.h>
#include "video.h"

void video_getmode(unsigned int descriptor, struct ctrl_videosettings *settings)
{

    call_walk(descriptor, CALL_PR, 19, "system/video:0/ctrl");
    call_open(descriptor);
    call_read(descriptor, sizeof (struct ctrl_videosettings), settings);
    call_close(descriptor);

}

void video_setmode(unsigned int descriptor, struct ctrl_videosettings *settings)
{

    call_walk(descriptor, CALL_PR, 19, "system/video:0/ctrl");
    call_open(descriptor);
    call_write(descriptor, sizeof (struct ctrl_videosettings), settings);
    call_close(descriptor);
    call_open(descriptor);
    call_read(descriptor, sizeof (struct ctrl_videosettings), settings);
    call_close(descriptor);

}

void video_setcolormap(unsigned int descriptor, unsigned int offset, unsigned int count, void *buffer)
{

    call_walk(descriptor, CALL_PR, 23, "system/video:0/colormap");
    call_open(descriptor);
    call_seek(descriptor, offset);
    call_write(descriptor, count, buffer);
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

    call_seek(descriptor, offset);
    call_write(descriptor, count, buffer);

}

