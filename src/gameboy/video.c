#include <fudge.h>
#include "video.h"

static unsigned char fb[LCD_HEIGHT][LCD_WIDTH];
static unsigned int palette[3][4] = {
    { 0x00FFFFFF, 0x00A0A0A0, 0x00606060, 0x00000000 },
    { 0x00FFFFFF, 0x00A0A0A0, 0x00606060, 0x00000000 },
    { 0x00FFFFFF, 0x00A0A0A0, 0x00606060, 0x00000000 }
};

void video_drawline(const unsigned char pixels[160], const unsigned char line)
{

    unsigned int x;

    for (x = 0; x < LCD_WIDTH; x++)
        fb[line][x] = pixels[x];

}

void video_render(void *framebuffer, unsigned int w, unsigned int scalew, unsigned int scaleh, unsigned int totw, unsigned int toth, unsigned int offx, unsigned int offy)
{

    unsigned int *ptarget = framebuffer;
    unsigned char *psource = (unsigned char *)fb;
    unsigned int y;

    for (y = 0; y < toth; y++)
    {

        unsigned int ytarget = (y + offy) * w;
        unsigned int ysource = (y / scaleh) * LCD_WIDTH;
        unsigned int x;

        for (x = 0; x < totw; x++)
        {

            unsigned int xtarget = x + offx;
            unsigned int xsource = x / scalew;
            unsigned char pixel = psource[ysource + xsource];
            unsigned int color = palette[(pixel & LCD_PALETTE_ALL) >> 4][pixel & 3];

            ptarget[ytarget + xtarget] = color;

        }

    }

}

