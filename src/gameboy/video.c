#include <fudge.h>
#include "video.h"

static unsigned short fb[LCD_HEIGHT][LCD_WIDTH];
static unsigned short selected_palette[3][4];
static unsigned short palette[3][4] = {
    { 0x7FFF, 0x5294, 0x294A, 0x0000 },
    { 0x7FFF, 0x5294, 0x294A, 0x0000 },
    { 0x7FFF, 0x5294, 0x294A, 0x0000 }
};

void video_setpalette(void)
{

    buffer_copy(selected_palette, palette, 3 * 4 * sizeof(unsigned short));

}

void video_drawline(const unsigned char pixels[160], const unsigned char line)
{

    unsigned int x;

    for (x = 0; x < LCD_WIDTH; x++)
        fb[line][x] = selected_palette[(pixels[x] & LCD_PALETTE_ALL) >> 4][pixels[x] & 3];

}

unsigned short *video_getfb(void)
{

    return (unsigned short *)fb;

}

