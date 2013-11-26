#include <fudge/module.h>
#include <base/base.h>
#include <base/terminal.h>
#include <base/video.h>
#include <arch/x86/pci/pci.h>
#include <arch/x86/vga/vga.h>
#include <arch/x86/vga/timing.h>
#include "cirrus.h"

#define CIRRUS_PCI_VENDOR               0x1013
#define CIRRUS_PCI_DEVICE               0x00B8

#define NU_FIXED_CLOCKS                 21
#define min(x, y)                       (((x) < (y)) ? (x) : (y))
#define max(x, y)                       (((x) > (y)) ? (x) : (y))

enum
{

    CLGD5420 = 0,
    CLGD7548,
    CLGD5420B,
    CLGD5422,
    CLGD5422C,
    CLGD5424,
    CLGD5426,
    CLGD5428,
    CLGD5429,
    CLGD5430,
    CLGD5434,
    CLGD5436

};

static int cirrus_fixed_clocks[NU_FIXED_CLOCKS] = {
    12599, 18000, 19600,
    25227, 28325, 31500, 36025, 37747, 39992, 41164,
    45076, 49867, 64983, 72163, 75000, 80013, 85226, 89998,
    95019, 100226, 108035
};

static int cirrus_memory;
static int cirrus_chiptype;
static int cirrus_chiprev;
static int DRAMbandwidth, DRAMbandwidthLimit;

static int cirrus_map_clock(int bpp, int pixelclock)
{

    if (bpp == 24 && cirrus_chiptype < CLGD5436)
        return pixelclock * 3;

    if (bpp == 16 && cirrus_chiptype <= CLGD5424)
        return pixelclock * 2;

    return pixelclock;

}

static int cirrus_map_horizontal_crtc(int bpp, int pixelclock, int htiming)
{

#ifdef ALWAYS_USE_5434_PALETTE_CLOCK_DOUBLING
    if (bpp == 8 && cirrus_chiptype >= CLGD5434)
#else
    if (bpp == 8 && cirrus_chiptype >= CLGD5434 && pixelclock > 86000)
#endif
        return htiming / 2;

    return htiming;

}

static void mode(struct base_device *device)
{

    unsigned char registers[60];
    struct vga_modetiming modetiming;
    struct vga_modeinfo modeinfo;
    struct vga_cardspecs cardspecs;
    int mclk = 0x22;

    cirrus_chiptype = CLGD5436;
    cirrus_chiprev = 0;
    cardspecs.videoMemory = 2048;
    cardspecs.maxPixelClock4bpp = 75000;
    cardspecs.maxPixelClock8bpp = 45000;
    cardspecs.maxPixelClock16bpp = 0;
    cardspecs.maxPixelClock24bpp = 0; 
    cardspecs.maxPixelClock32bpp = 0;

    DRAMbandwidth = 14318 * mclk / 16;

    if (cirrus_memory >= 512)
        DRAMbandwidth *= 2;

    if (cirrus_memory >= 1024)
        DRAMbandwidth *= 2;

    if (cirrus_memory >= 2048)
        DRAMbandwidth *= 2;

    DRAMbandwidthLimit = (DRAMbandwidth * 10) / 11;

    if (cirrus_chiptype == CLGD5420B)
    {

        cardspecs.maxPixelClock16bpp = 75000 / 2;
        cardspecs.maxPixelClock24bpp = 25175;

    }

    if (cirrus_chiptype >= CLGD5422)
    {

        cardspecs.maxPixelClock4bpp = 80000;
        cardspecs.maxPixelClock8bpp = 80000;

        if (cirrus_chiptype >= CLGD5426)
            cardspecs.maxPixelClock16bpp = 80000;
        else if (cirrus_memory >= 1024)
            cardspecs.maxPixelClock16bpp = 80000 / 2;

        if (cirrus_memory >= 1024)
            cardspecs.maxPixelClock24bpp = 80000 / 3;

    }

    if (cirrus_chiptype >= CLGD5429)
    {

        cardspecs.maxPixelClock4bpp = 86000;
        cardspecs.maxPixelClock8bpp = 86000;
        cardspecs.maxPixelClock16bpp = 86000;

        if (cirrus_memory >= 1024)
            cardspecs.maxPixelClock24bpp = 86000 / 3;

    }

    if (cirrus_chiptype == CLGD5434)
    {

#ifdef SUPPORT_5434_PALETTE_CLOCK_DOUBLING
        cardspecs.maxPixelClock8bpp = 108300;

        if (cirrus_chiprev > 0)
            cardspecs.maxPixelClock8bpp = 135300;
#endif

        if (cirrus_memory >= 2048)
            cardspecs.maxPixelClock32bpp = 86000;

    }

    if (cirrus_chiptype >= CLGD5436)
    {

#ifdef SUPPORT_5434_PALETTE_CLOCK_DOUBLING
        cardspecs.maxPixelClock8bpp = 135300;
#endif

        if (cirrus_memory >= 2048)
            cardspecs.maxPixelClock32bpp = 86000;

    }

    cardspecs.maxPixelClock8bpp = min(cardspecs.maxPixelClock8bpp, DRAMbandwidthLimit);
    cardspecs.maxPixelClock16bpp = min(cardspecs.maxPixelClock16bpp, DRAMbandwidthLimit / 2);
    cardspecs.maxPixelClock24bpp = min(cardspecs.maxPixelClock24bpp, DRAMbandwidthLimit / 3);
    cardspecs.maxPixelClock32bpp = min(cardspecs.maxPixelClock32bpp, DRAMbandwidthLimit / 4);
    cardspecs.flags = INTERLACE_DIVIDE_VERT | GREATER_1024_DIVIDE_VERT;
    cardspecs.nClocks = NU_FIXED_CLOCKS;
    cardspecs.clocks = cirrus_fixed_clocks;
    cardspecs.mapClock = cirrus_map_clock;
    cardspecs.mapHorizontalCrtc = cirrus_map_horizontal_crtc;
    cardspecs.maxHorizontalCrtc = 2040;
    cardspecs.maxPixelClock4bpp = 0;

    vga_initmodeinfo(&modeinfo, 19);
    vga_initmodetiming(&modetiming, &modeinfo, &cardspecs);
    vga_initregisters(registers, &modetiming, &modeinfo);

}

static unsigned int read_video_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_video_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int read_video_colormap(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_video_colormap(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static void attach(struct base_device *device)
{

    struct cirrus_driver *driver = (struct cirrus_driver *)device->driver;

    base_video_register_interface(&driver->ivideo, device);

}

static unsigned int check(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    return pci_device_inw(pciDevice, PCI_CONFIG_VENDOR) == CIRRUS_PCI_VENDOR && pci_device_inw(pciDevice, PCI_CONFIG_DEVICE) == CIRRUS_PCI_DEVICE;

}

void cirrus_init_driver(struct cirrus_driver *driver)
{

    memory_clear(driver, sizeof (struct cirrus_driver));
    base_init_driver(&driver->base, "cirrus", check, attach);
    base_video_init_interface(&driver->ivideo, mode, read_video_data, write_video_data, read_video_colormap, write_video_colormap);

    driver->ivideo.xres = 80;
    driver->ivideo.yres = 25;
    driver->ivideo.bpp = 16;

}

