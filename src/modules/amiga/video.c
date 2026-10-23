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
#define PLANES                          4
#define ROWBYTES                        (WIDTH / 8)
#define PLANESIZE                       (ROWBYTES * HEIGHT)
#define FRAMEBUFFER                     0xA0000000
#define CHIP                            0x00020000
#define CHIPSIZE                        0x00020000
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
#define REG_BPLCON1                     0x102
#define REG_BPLCON2                     0x104
#define REG_BPLCON3                     0x106
#define REG_BPL1MOD                     0x108
#define REG_BPL2MOD                     0x10A
#define REG_BPLCON4                     0x10C
#define REG_COLOR00                     0x180
#define REG_FMODE                       0x1FC

static struct base_driver driver;
static struct video_interface videointerface;
static volatile unsigned short *custom = (volatile unsigned short *)REG_CUSTOM;
static unsigned short *copperlist = (unsigned short *)COPPERLIST;
static unsigned char *bitplanes = (unsigned char *)BITPLANES;
static unsigned int *framebuffer = (unsigned int *)FRAMEBUFFER;
static unsigned int created;

/* eight greys (0 to 7) and the eight corners of the colour cube at three quarters (8 to 15) */
static unsigned short palette[16] = {
    0x000, 0x222, 0x444, 0x666, 0x888, 0xAAA, 0xCCC, 0xFFF,
    0x000, 0x00C, 0x0C0, 0x0CC, 0xC00, 0xC0C, 0xCC0, 0xCCC
};

/* nearly grey pixels keep their brightness, others keep which components stand out */
static unsigned int getcolor(unsigned int pixel)
{

    unsigned int r = (pixel >> 16) & 0xFF;
    unsigned int g = (pixel >> 8) & 0xFF;
    unsigned int b = pixel & 0xFF;
    unsigned int max = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
    unsigned int min = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);

    if (max - min < 48)
        return (r * 2 + g * 5 + b) >> 8;

    return 8 | ((r > max / 2) << 2) | ((g > max / 2) << 1) | (b > max / 2);

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

/* a pal hires screen: the display window and data fetch of the usual 640 by 256 */
static void setupdisplay(void)
{

    unsigned int i;

    custom[REG_DMACON / 2] = REG_DMACON_BITPLANE | REG_DMACON_COPPER | REG_DMACON_SPRITE;
    custom[REG_FMODE / 2] = 0;
    custom[REG_BPLCON0 / 2] = REG_BPLCON0_HIRES | (PLANES << 12) | REG_BPLCON0_COLOR;
    custom[REG_BPLCON1 / 2] = 0;
    custom[REG_BPLCON2 / 2] = 0;
    custom[REG_BPLCON3 / 2] = 0;
    custom[REG_BPLCON4 / 2] = 0x0011;
    custom[REG_BPL1MOD / 2] = 0;
    custom[REG_BPL2MOD / 2] = 0;
    custom[REG_DIWSTRT / 2] = 0x2C81;
    custom[REG_DIWSTOP / 2] = 0x2CC1;
    custom[REG_DDFSTRT / 2] = 0x003C;
    custom[REG_DDFSTOP / 2] = 0x00D4;

    for (i = 0; i < 16; i++)
        custom[REG_COLOR00 / 2 + i] = palette[i];

    custom[REG_COP1LCH / 2] = COPPERLIST >> 16;
    custom[REG_COP1LCL / 2] = COPPERLIST & 0xFFFF;
    custom[REG_COPJMP1 / 2] = 0;
    custom[REG_DMACON / 2] = REG_DMACON_SET | REG_DMACON_MASTER | REG_DMACON_BITPLANE | REG_DMACON_COPPER;

}

/* chunky to planar: each group of eight pixels becomes one byte in every plane */
static void convert(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{

    unsigned int y;

    for (y = y0; y < y1; y++)
    {

        unsigned int column;

        for (column = x0 / 8; column < (x1 + 7) / 8; column++)
        {

            unsigned int *pixels = framebuffer + y * WIDTH + column * 8;
            unsigned int offset = y * ROWBYTES + column;
            unsigned int p0 = 0;
            unsigned int p1 = 0;
            unsigned int p2 = 0;
            unsigned int p3 = 0;
            unsigned int i;

            for (i = 0; i < 8; i++)
            {

                unsigned int color = getcolor(pixels[i]);

                p0 = (p0 << 1) | (color & 1);
                p1 = (p1 << 1) | ((color >> 1) & 1);
                p2 = (p2 << 1) | ((color >> 2) & 1);
                p3 = (p3 << 1) | ((color >> 3) & 1);

            }

            bitplanes[offset] = p0;
            bitplanes[offset + PLANESIZE] = p1;
            bitplanes[offset + PLANESIZE * 2] = p2;
            bitplanes[offset + PLANESIZE * 3] = p3;

        }

    }

}

/* only one mode: whatever wm asks for, it gets pal hires */
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

