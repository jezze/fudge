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

unsigned int *video_getfb(void)
{

    return (unsigned int *)fb;

}

