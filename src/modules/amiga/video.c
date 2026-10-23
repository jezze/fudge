#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/video.h>
#include "amiga.h"

#define BPP                             4
#define WIDTH                           640
#define HEIGHT                          256
#define PLANES                          8
#define COLORS                          256
#define ROWBYTES                        (WIDTH / 8)
#define PLANESIZE                       (ROWBYTES * HEIGHT)
#define FRAMEBUFFER                     0xA0000000
#define CHIP                            0x00020000
#define CHIPSIZE                        0x00030000
#define COPPERLIST                      CHIP
#define BITPLANES                       (CHIP + 0x1000)
#define REG_CUSTOM                      0x00DFF000
#define REG_DMACON                      0x096
#define REG_DMACON_SET                  0x8000
#define REG_DMACON_MASTER               0x0200
#define REG_DMACON_BITPLANE             0x0100
#define REG_DMACON_COPPER               0x0080
#define REG_DMACON_SPRITE               0x0020
#define REG_COP1LCH                     0x080
#define REG_COP1LCL                     0x082
#define REG_COPJMP1                     0x088
#define REG_DIWSTRT                     0x08E
#define REG_DIWSTOP                     0x090
#define REG_DDFSTRT                     0x092
#define REG_DDFSTOP                     0x094
#define REG_BPL1PTH                     0x0E0
#define REG_BPLCON0                     0x100
#define REG_BPLCON0_HIRES               0x8000
#define REG_BPLCON0_COLOR               0x0200
#define REG_BPLCON0_BPU3                0x0010
#define REG_BPLCON1                     0x102
#define REG_BPLCON2                     0x104
#define REG_BPLCON3                     0x106
#define REG_BPLCON3_DEFAULT             0x0C00
#define REG_BPLCON3_LOCT                0x0200
#define REG_BPL1MOD                     0x108
#define REG_BPL2MOD                     0x10A
#define REG_BPLCON4                     0x10C
#define REG_COLOR00                     0x180
#define REG_FMODE                       0x1FC
#define REG_FMODE_BPL4X                 0x0003

static struct base_driver driver;
static struct video_interface videointerface;
static volatile unsigned short *custom = (volatile unsigned short *)REG_CUSTOM;
static unsigned short *copperlist = (unsigned short *)COPPERLIST;
static unsigned char *bitplanes = (unsigned char *)BITPLANES;
static unsigned int *framebuffer = (unsigned int *)FRAMEBUFFER;
static unsigned int palette[COLORS];
static unsigned char nearest[0x8000];
static unsigned int created;

/* indices 0 to 215 are a 6x6x6 colour cube, 216 to 255 the greys between the cube's six on a ramp of 46 */
static void setuppalette(void)
{

    unsigned int i;
    unsigned int k;

    for (i = 0; i < 216; i++)
    {

        unsigned int r = (i / 36) * 51;
        unsigned int g = ((i / 6) % 6) * 51;
        unsigned int b = (i % 6) * 51;

        palette[i] = (r << 16) | (g << 8) | b;

    }

    for (i = 216, k = 0; k < 46; k++)
    {

        unsigned int v = k * 255 / 45;

        if (k % 9)
            palette[i++] = (v << 16) | (v << 8) | v;

    }

}

static unsigned int distance(unsigned int a, unsigned int b)
{

    int r = (int)((a >> 16) & 0xFF) - (int)((b >> 16) & 0xFF);
    int g = (int)((a >> 8) & 0xFF) - (int)((b >> 8) & 0xFF);
    int bl = (int)(a & 0xFF) - (int)(b & 0xFF);

    return r * r * 2 + g * g * 4 + bl * bl;

}

/* for every 15-bit colour the closer of its nearest cube colour and its nearest grey */
static void setupnearest(void)
{

    unsigned int i;

    for (i = 0; i < 0x8000; i++)
    {

        unsigned int r = ((i >> 10) & 0x1F) * 255 / 31;
        unsigned int g = ((i >> 5) & 0x1F) * 255 / 31;
        unsigned int b = (i & 0x1F) * 255 / 31;
        unsigned int color = (r << 16) | (g << 8) | b;
        unsigned int cube = ((r + 25) / 51) * 36 + ((g + 25) / 51) * 6 + (b + 25) / 51;
        unsigned int level = ((r * 2 + g * 4 + b) / 7 * 45 + 127) / 255;
        unsigned int grey = (level % 9) ? 216 + level - level / 9 - 1 : (level / 9) * 43;

        nearest[i] = (distance(color, palette[grey]) < distance(color, palette[cube])) ? grey : cube;

    }

}

/* every frame the copper points the bitplane pointers back at the start of each plane */
static void setupcopper(void)
{

    unsigned int i;
    unsigned int n = 0;

    for (i = 0; i < PLANES; i++)
    {

        unsigned long plane = BITPLANES + i * PLANESIZE;

        copperlist[n++] = REG_BPL1PTH + i * 4;
        copperlist[n++] = plane >> 16;
        copperlist[n++] = REG_BPL1PTH + i * 4 + 2;
        copperlist[n++] = plane & 0xFFFF;

    }

    copperlist[n++] = 0xFFFF;
    copperlist[n++] = 0xFFFE;

}

/* aga keeps 24-bit colours in eight banks of 32: the high nibbles are written first, then with loct the low ones */
static void setupcolors(void)
{

    unsigned int i;

    for (i = 0; i < COLORS; i++)
    {

        unsigned int color = palette[i];
        unsigned int bank = (i / 32) << 13;
        unsigned int high = ((color >> 12) & 0xF00) | ((color >> 8) & 0x0F0) | ((color >> 4) & 0x00F);
        unsigned int low = ((color >> 8) & 0xF00) | ((color >> 4) & 0x0F0) | (color & 0x00F);

        custom[REG_BPLCON3 / 2] = REG_BPLCON3_DEFAULT | bank;
        custom[REG_COLOR00 / 2 + i % 32] = high;
        custom[REG_BPLCON3 / 2] = REG_BPLCON3_DEFAULT | bank | REG_BPLCON3_LOCT;
        custom[REG_COLOR00 / 2 + i % 32] = low;

    }

    custom[REG_BPLCON3 / 2] = REG_BPLCON3_DEFAULT;

}

/* a pal hires screen in eight planes needs aga's 4x fetch; its data fetch window is the one aros computes for it */
static void setupdisplay(void)
{

    custom[REG_DMACON / 2] = REG_DMACON_BITPLANE | REG_DMACON_COPPER | REG_DMACON_SPRITE;
    custom[REG_FMODE / 2] = REG_FMODE_BPL4X;
    custom[REG_BPLCON0 / 2] = REG_BPLCON0_HIRES | REG_BPLCON0_BPU3 | REG_BPLCON0_COLOR;
    custom[REG_BPLCON1 / 2] = 0;
    custom[REG_BPLCON2 / 2] = 0;
    custom[REG_BPLCON4 / 2] = 0x0011;
    custom[REG_BPL1MOD / 2] = 0;
    custom[REG_BPL2MOD / 2] = 0;
    custom[REG_DIWSTRT / 2] = 0x2C81;
    custom[REG_DIWSTOP / 2] = 0x2CC1;
    custom[REG_DDFSTRT / 2] = 0x0038;
    custom[REG_DDFSTOP / 2] = 0x00C0;

    setupcolors();

    custom[REG_COP1LCH / 2] = COPPERLIST >> 16;
    custom[REG_COP1LCL / 2] = COPPERLIST & 0xFFFF;
    custom[REG_COPJMP1 / 2] = 0;
    custom[REG_DMACON / 2] = REG_DMACON_SET | REG_DMACON_MASTER | REG_DMACON_BITPLANE | REG_DMACON_COPPER;

}

static unsigned int getindex(unsigned int pixel)
{

    return nearest[((pixel >> 9) & 0x7C00) | ((pixel >> 6) & 0x03E0) | ((pixel >> 3) & 0x001F)];

}

/* chunky to planar on 32 pixels: their palette indices, four to a word with the last four in w0, become one word per plane in five rounds of swaps (each swaps a bit of the word number with a bit of the position in the word), so each plane gets one long write */
static void convert(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{

    unsigned int y;

    for (y = y0; y < y1; y++)
    {

        unsigned int column;

        for (column = x0 / 32; column < (x1 + 31) / 32; column++)
        {

            unsigned int *pixels = framebuffer + y * WIDTH + column * 32;
            unsigned int *plane = (unsigned int *)(bitplanes + y * ROWBYTES) + column;
            unsigned int w0;
            unsigned int w1;
            unsigned int w2;
            unsigned int w3;
            unsigned int w4;
            unsigned int w5;
            unsigned int w6;
            unsigned int w7;
            unsigned int t;

            w7 = (getindex(pixels[0]) << 24) | (getindex(pixels[1]) << 16) | (getindex(pixels[2]) << 8) | getindex(pixels[3]);
            w6 = (getindex(pixels[4]) << 24) | (getindex(pixels[5]) << 16) | (getindex(pixels[6]) << 8) | getindex(pixels[7]);
            w5 = (getindex(pixels[8]) << 24) | (getindex(pixels[9]) << 16) | (getindex(pixels[10]) << 8) | getindex(pixels[11]);
            w4 = (getindex(pixels[12]) << 24) | (getindex(pixels[13]) << 16) | (getindex(pixels[14]) << 8) | getindex(pixels[15]);
            w3 = (getindex(pixels[16]) << 24) | (getindex(pixels[17]) << 16) | (getindex(pixels[18]) << 8) | getindex(pixels[19]);
            w2 = (getindex(pixels[20]) << 24) | (getindex(pixels[21]) << 16) | (getindex(pixels[22]) << 8) | getindex(pixels[23]);
            w1 = (getindex(pixels[24]) << 24) | (getindex(pixels[25]) << 16) | (getindex(pixels[26]) << 8) | getindex(pixels[27]);
            w0 = (getindex(pixels[28]) << 24) | (getindex(pixels[29]) << 16) | (getindex(pixels[30]) << 8) | getindex(pixels[31]);

            t = ((w0 >> 16) ^ w4) & 0x0000FFFF;
            w4 ^= t;
            w0 ^= t << 16;
            t = ((w1 >> 16) ^ w5) & 0x0000FFFF;
            w5 ^= t;
            w1 ^= t << 16;
            t = ((w2 >> 16) ^ w6) & 0x0000FFFF;
            w6 ^= t;
            w2 ^= t << 16;
            t = ((w3 >> 16) ^ w7) & 0x0000FFFF;
            w7 ^= t;
            w3 ^= t << 16;

            t = ((w0 >> 8) ^ w2) & 0x00FF00FF;
            w2 ^= t;
            w0 ^= t << 8;
            t = ((w1 >> 8) ^ w3) & 0x00FF00FF;
            w3 ^= t;
            w1 ^= t << 8;
            t = ((w4 >> 8) ^ w6) & 0x00FF00FF;
            w6 ^= t;
            w4 ^= t << 8;
            t = ((w5 >> 8) ^ w7) & 0x00FF00FF;
            w7 ^= t;
            w5 ^= t << 8;

            t = ((w0 >> 4) ^ w1) & 0x0F0F0F0F;
            w1 ^= t;
            w0 ^= t << 4;
            t = ((w2 >> 4) ^ w3) & 0x0F0F0F0F;
            w3 ^= t;
            w2 ^= t << 4;
            t = ((w4 >> 4) ^ w5) & 0x0F0F0F0F;
            w5 ^= t;
            w4 ^= t << 4;
            t = ((w6 >> 4) ^ w7) & 0x0F0F0F0F;
            w7 ^= t;
            w6 ^= t << 4;

            t = ((w0 >> 2) ^ w4) & 0x33333333;
            w4 ^= t;
            w0 ^= t << 2;
            t = ((w1 >> 2) ^ w5) & 0x33333333;
            w5 ^= t;
            w1 ^= t << 2;
            t = ((w2 >> 2) ^ w6) & 0x33333333;
            w6 ^= t;
            w2 ^= t << 2;
            t = ((w3 >> 2) ^ w7) & 0x33333333;
            w7 ^= t;
            w3 ^= t << 2;

            t = ((w0 >> 1) ^ w2) & 0x55555555;
            w2 ^= t;
            w0 ^= t << 1;
            t = ((w1 >> 1) ^ w3) & 0x55555555;
            w3 ^= t;
            w1 ^= t << 1;
            t = ((w4 >> 1) ^ w6) & 0x55555555;
            w6 ^= t;
            w4 ^= t << 1;
            t = ((w5 >> 1) ^ w7) & 0x55555555;
            w7 ^= t;
            w5 ^= t << 1;

            plane[PLANESIZE / 4 * 0] = w0;
            plane[PLANESIZE / 4 * 4] = w1;
            plane[PLANESIZE / 4 * 1] = w2;
            plane[PLANESIZE / 4 * 5] = w3;
            plane[PLANESIZE / 4 * 2] = w4;
            plane[PLANESIZE / 4 * 6] = w5;
            plane[PLANESIZE / 4 * 3] = w6;
            plane[PLANESIZE / 4 * 7] = w7;

        }

    }

}

/* only one mode: whatever wm asks for, it gets pal hires in 256 colours */
static void videointerface_setmode(unsigned int width, unsigned int height, unsigned int bpp)
{

    arch_kmap(ARCH_FRAMEBUFFER_BASE, FRAMEBUFFER, WIDTH * HEIGHT * BPP, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);
    buffer_clear(framebuffer, WIDTH * HEIGHT * BPP);
    buffer_clear(bitplanes, PLANESIZE * PLANES);
    setupcopper();
    setupdisplay();

    videointerface.width = WIDTH;
    videointerface.height = HEIGHT;
    videointerface.bpp = BPP;
    created = 1;

}

static void videointerface_flush(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{

    if (!created || x >= WIDTH || y >= HEIGHT)
        return;

    if (width > WIDTH - x)
        width = WIDTH - x;

    if (height > HEIGHT - y)
        height = HEIGHT - y;

    convert(x, y, x + width, y + height);

}

static void driver_init(unsigned int id)
{

    setuppalette();
    setupnearest();
    video_initinterface(&videointerface, id, FRAMEBUFFER, 0, videointerface_setmode);

    videointerface.flush = videointerface_flush;

}

static unsigned int driver_match(unsigned int id)
{

    return id == AMIGA_VIDEO;

}

static void driver_reset(unsigned int id)
{

    arch_kmap(CHIP, CHIP, CHIPSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

}

static void driver_attach(unsigned int id)
{

    video_registerinterface(&videointerface);

}

static void driver_detach(unsigned int id)
{

    custom[REG_DMACON / 2] = REG_DMACON_BITPLANE | REG_DMACON_COPPER;

    video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "amiga-video", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, AMIGA_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, AMIGA_BUS);

}

