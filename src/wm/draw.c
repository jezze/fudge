#include <abi.h>
#include <fudge.h>
#include <video/video.h>

static unsigned char backbuffer[4096];

void flush(unsigned int line, unsigned int bpp, unsigned int offset, unsigned int count)
{

    video_draw(line + offset, count, backbuffer + offset * bpp / 8);

}

void fill8(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *b = (unsigned char *)backbuffer;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        b[i] = color;

}

void fill32(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int *b = (unsigned int *)backbuffer;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        b[i] = color;

}

