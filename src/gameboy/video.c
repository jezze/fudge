#include <fudge.h>
#include "video.h"

static unsigned int fb[LCD_HEIGHT][LCD_WIDTH];
static unsigned int selected_palette[3][4];
static unsigned int palette[3][4] = {
    { 0x00FFFFFF, 0x00A0A0A0, 0x00606060, 0x00000000 },
    { 0x00FFFFFF, 0x00A0A0A0, 0x00606060, 0x00000000 },
    { 0x00FFFFFF, 0x00A0A0A0, 0x00606060, 0x00000000 }
};

void video_setpalette(void)
{

    buffer_copy(selected_palette, palette, 3 * 4 * sizeof(unsigned int));

}

void video_drawline(const unsigned char pixels[160], const unsigned char line)
{

    unsigned int x;

    for (x = 0; x < LCD_WIDTH; x++)
        fb[line][x] = selected_palette[(pixels[x] & LCD_PALETTE_ALL) >> 4][pixels[x] & 3];

}

void video_render(void *framebuffer, unsigned int w, unsigned int scalew, unsigned int scaleh, unsigned int totw, unsigned int toth, unsigned int offx, unsigned int offy)
{

    unsigned int *ptarget = framebuffer;
    unsigned int *psource = (unsigned int *)fb;
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

            ptarget[ytarget + xtarget] = psource[ysource + xsource];

        }

    }

}

