#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/video.h>
#include <modules/virt/virt.h>

#define BPP                             4
#define FILES                           0x19
#define FILESIZE                        64
#define FOURCC_XRGB8888                 0x34325258
#define REG_DATA                        0x00
#define REG_SELECT                      0x08
#define REG_DMA                         0x10
#define DMA_ERROR                       (1 << 0)
#define DMA_SELECT                      (1 << 3)
#define DMA_WRITE                       (1 << 4)
#define POLLS                           0x100000

/* fw_cfg wants big-endian words, ramfb's configuration too */
struct dma
{

    unsigned int control;
    unsigned int length;
    unsigned int addresshigh;
    unsigned int addresslow;

};

struct config
{

    unsigned int addresshigh;
    unsigned int addresslow;
    unsigned int fourcc;
    unsigned int flags;
    unsigned int width;
    unsigned int height;
    unsigned int stride;

};

static struct base_driver driver;
static struct video_interface videointerface;
static unsigned long fwcfg;
static unsigned short ramfb;
static struct dma dma;
static struct config config;

static unsigned int swap32(unsigned int value)
{

    return (value >> 24) | ((value >> 8) & 0xFF00) | ((value << 8) & 0xFF0000) | (value << 24);

}

static void select(unsigned short key)
{

    *(volatile unsigned short *)(fwcfg + REG_SELECT) = (key >> 8) | (key << 8);

}

static unsigned int read32(void)
{

    volatile unsigned char *data = (volatile unsigned char *)(fwcfg + REG_DATA);
    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < 4; i++)
        value = (value << 8) | *data;

    return value;

}

/* walks fw_cfg's file directory for etc/ramfb, which qemu offers with -device ramfb */
static unsigned short findfile(char *name)
{

    volatile unsigned char *data = (volatile unsigned char *)(fwcfg + REG_DATA);
    unsigned int count;
    unsigned int i;

    select(FILES);

    count = read32();

    for (i = 0; i < count; i++)
    {

        unsigned char entry[FILESIZE];
        unsigned int j;

        for (j = 0; j < FILESIZE; j++)
            entry[j] = *data;

        if (cstring_match((char *)entry + 8, name))
            return (entry[4] << 8) | entry[5];

    }

    return 0;

}

static unsigned int write(unsigned short key, void *buffer, unsigned int count)
{

    unsigned long address = (unsigned long)&dma;
    unsigned long data = (unsigned long)buffer;
    unsigned int i;

    dma.control = swap32((key << 16) | DMA_SELECT | DMA_WRITE);
    dma.length = swap32(count);
    dma.addresshigh = swap32((data >> 16) >> 16);
    dma.addresslow = swap32(data);

    *(volatile unsigned int *)(fwcfg + REG_DMA) = swap32((address >> 16) >> 16);
    *(volatile unsigned int *)(fwcfg + REG_DMA + 4) = swap32(address);

    for (i = 0; i < POLLS; i++)
    {

        unsigned int control = swap32(*(volatile unsigned int *)&dma.control);

        if (!(control & ~DMA_ERROR))
            return !(control & DMA_ERROR);

    }

    return 0;

}

/* qemu shows the framebuffer from memory, so a mode is only its size and format */
static void videointerface_setmode(unsigned int width, unsigned int height, unsigned int bpp)
{

    if (width * height * BPP > ARCH_FRAMEBUFFER_SIZE)
        height = ARCH_FRAMEBUFFER_SIZE / (width * BPP);

    videointerface.width = width;
    videointerface.height = height;
    videointerface.bpp = BPP;

    arch_kmap(ARCH_FRAMEBUFFER_BASE, 0xA0000000, width * height * BPP, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);

    config.addresshigh = swap32((ARCH_FRAMEBUFFER_BASE >> 16) >> 16);
    config.addresslow = swap32(ARCH_FRAMEBUFFER_BASE);
    config.fourcc = swap32(FOURCC_XRGB8888);
    config.flags = 0;
    config.width = swap32(width);
    config.height = swap32(height);
    config.stride = swap32(width * BPP);

    write(ramfb, &config, sizeof (struct config));

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id, 0xA0000000, 0, videointerface_setmode);

}

static unsigned int driver_match(unsigned int id)
{

    return id == VIRT_FWCFG;

}

static void driver_reset(unsigned int id)
{

    fwcfg = virt_getbase(id);

    arch_kmap(fwcfg, fwcfg, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    ramfb = findfile("etc/ramfb");

}

static void driver_attach(unsigned int id)
{

    if (ramfb)
        video_registerinterface(&videointerface);

}

static void driver_detach(unsigned int id)
{

    if (ramfb)
        video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "ramfb", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, VIRT_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, VIRT_BUS);

}
