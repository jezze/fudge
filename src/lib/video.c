#include <abi.h>
#include <fudge.h>
#include <lib/file.h>
#include "video.h"

void video_getmode(unsigned int descriptor, struct ctrl_videosettings *settings)
{

    call_walk(descriptor, CALL_PR, 19, "system/video:0/ctrl");
    call_open(descriptor);
    file_readall(descriptor, settings, sizeof (struct ctrl_videosettings));
    call_close(descriptor);

}

void video_setmode(unsigned int descriptor, struct ctrl_videosettings *settings)
{

    call_walk(descriptor, CALL_PR, 19, "system/video:0/ctrl");
    call_open(descriptor);
    file_writeall(descriptor, settings, sizeof (struct ctrl_videosettings));
    call_close(descriptor);

}

void video_setcolormap(unsigned int descriptor, unsigned int offset, unsigned int count, void *buffer)
{

    call_walk(descriptor, CALL_PR, 23, "system/video:0/colormap");
    call_open(descriptor);
    file_writeall(descriptor, buffer, count);
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

    file_seekwriteall(descriptor, buffer, count, offset);

}

