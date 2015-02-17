#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <video/video.h>
#include <arch/x86/pci/pci.h>
#include <arch/x86/vga/timing.h>

#define CIRRUS_PCI_VENDOR               0x1013
#define CIRRUS_PCI_DEVICE               0x00B8

#define CLOCKS                          21
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

static int clocks[CLOCKS] = {
    12599, 18000, 19600, 25227, 28325, 31500, 36025, 37747,
    39992, 41164, 45076, 49867, 64983, 72163, 75000, 80013,
    85226, 89998, 95019, 100226, 108035
};

static struct base_driver driver;
static struct video_interface videointerface;
static int chiptype;
static int chiprev;

static int mapclock(int bpp, int pixelclock)
{

    if (bpp == VGA_BPP24 && chiptype < CLGD5436)
        return pixelclock * 3;

    if (bpp == VGA_BPP16 && chiptype <= CLGD5424)
        return pixelclock * 2;

    return pixelclock;

}

static int maphorizontalcrtc(int bpp, int pixelclock, int htiming)
{

    if (bpp == VGA_BPP8 && chiptype >= CLGD5434)
        return htiming / 2;

    return htiming;

}

static void videointerface_setmode(unsigned int xres, unsigned int yres, unsigned int bpp)
{

    unsigned char registers[60];
    struct vga_modetiming modetiming;
    struct vga_modeinfo modeinfo;
    struct vga_cardspecs cardspecs;
    int mclk;
    int DRAMbandwidth;
    int DRAMbandwidthLimit;

    /* Identify real values */
    chiptype = CLGD5436;
    chiprev = 0;
    cardspecs.videoMemory = 2048;
    mclk = 0x22;

    DRAMbandwidth = 14318 * mclk / 16;

    if (cardspecs.videoMemory >= 512)
        DRAMbandwidth *= 2;

    if (cardspecs.videoMemory >= 1024)
        DRAMbandwidth *= 2;

    if (cardspecs.videoMemory >= 2048)
        DRAMbandwidth *= 2;

    DRAMbandwidthLimit = (DRAMbandwidth * 10) / 11;

    cardspecs.maxPixelClock4bpp = 75000;
    cardspecs.maxPixelClock8bpp = 45000;
    cardspecs.maxPixelClock16bpp = 0;
    cardspecs.maxPixelClock24bpp = 0; 
    cardspecs.maxPixelClock32bpp = 0;

    if (chiptype == CLGD5420B)
    {

        cardspecs.maxPixelClock16bpp = 75000 / 2;
        cardspecs.maxPixelClock24bpp = 25175;

    }

    if (chiptype >= CLGD5422)
    {

        cardspecs.maxPixelClock4bpp = 80000;
        cardspecs.maxPixelClock8bpp = 80000;

        if (chiptype >= CLGD5426)
            cardspecs.maxPixelClock16bpp = 80000;
        else if (cardspecs.videoMemory >= 1024)
            cardspecs.maxPixelClock16bpp = 80000 / 2;

        if (cardspecs.videoMemory >= 1024)
            cardspecs.maxPixelClock24bpp = 80000 / 3;

    }

    if (chiptype >= CLGD5429)
    {

        cardspecs.maxPixelClock4bpp = 86000;
        cardspecs.maxPixelClock8bpp = 86000;
        cardspecs.maxPixelClock16bpp = 60000;

        if (cardspecs.videoMemory >= 1024)
            cardspecs.maxPixelClock24bpp = 86000 / 3;

    }

    if (chiptype == CLGD5434)
    {

        cardspecs.maxPixelClock8bpp = 108300;

        if (chiprev > 0)
            cardspecs.maxPixelClock8bpp = 135300;

        if (cardspecs.videoMemory >= 2048)
            cardspecs.maxPixelClock32bpp = 86000;

    }

    if (chiptype >= CLGD5436)
    {

        cardspecs.maxPixelClock8bpp = 135300;

        if (cardspecs.videoMemory >= 2048)
            cardspecs.maxPixelClock32bpp = 86000;

    }

    cardspecs.maxPixelClock8bpp = min(cardspecs.maxPixelClock8bpp, DRAMbandwidthLimit);
    cardspecs.maxPixelClock16bpp = min(cardspecs.maxPixelClock16bpp, DRAMbandwidthLimit / 2);
    cardspecs.maxPixelClock24bpp = min(cardspecs.maxPixelClock24bpp, DRAMbandwidthLimit / 3);
    cardspecs.maxPixelClock32bpp = min(cardspecs.maxPixelClock32bpp, DRAMbandwidthLimit / 4);
    cardspecs.flags = INTERLACE_DIVIDE_VERT | GREATER_1024_DIVIDE_VERT;
    cardspecs.nClocks = CLOCKS;
    cardspecs.clocks = clocks;
    cardspecs.mapClock = mapclock;
    cardspecs.mapHorizontalCrtc = maphorizontalcrtc;
    cardspecs.maxHorizontalCrtc = 2040;
    cardspecs.maxPixelClock4bpp = 0;

    vga_initmodeinfo(&modeinfo, 19);
    vga_initmodetiming(&modetiming, &modeinfo, &cardspecs);
    vga_initregisters(registers, &modetiming, &modeinfo);
    vga_setregisters(registers, 0);

}

static unsigned int videointerface_rdata(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int videointerface_wdata(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int videointerface_rcolormap(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int videointerface_wcolormap(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static void driver_init(struct base_driver *self)
{

    video_initinterface(&videointerface, self, videointerface_setmode, videointerface_rdata, videointerface_wdata, videointerface_rcolormap, videointerface_wcolormap);

}

static unsigned int driver_match(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_inw(bus, id, PCI_CONFIG_VENDOR) == CIRRUS_PCI_VENDOR && pci_inw(bus, id, PCI_CONFIG_DEVICE) == CIRRUS_PCI_DEVICE;

}

static void driver_attach(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    video_registerinterface(&videointerface, bus, id);

}

static void driver_detach(struct base_driver *self, struct base_bus *bus, unsigned int id)
{

    video_unregisterinterface(&videointerface);

}

void module_init()
{

    base_initdriver(&driver, "cirrus", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver);

}

void module_unregister()
{

    base_unregisterdriver(&driver);

}

