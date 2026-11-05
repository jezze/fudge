#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/video.h>
#include <modules/pci/pci.h>
#include "i915.h"

struct aux
{

    unsigned int command;
    unsigned int address;
    unsigned int length;
    unsigned int value;
    unsigned char data[I915_AUX_SIZE];
    unsigned int count;

};

static struct base_driver driver;
static struct video_interface videointerface;
static struct arch_framebuffer *framebuffer = (struct arch_framebuffer *)ARCH_FIRMWARE_FRAMEBUFFER;
static unsigned short devices[I915_DEVICES] = {0x9A40, 0x9A49, 0x9A59, 0x9A60, 0x9A68, 0x9A70, 0x9A78, 0x9AC0, 0x9AC9, 0x9AD9, 0x9AF8};
static unsigned long mmio;
static unsigned int activewidth;
static unsigned int activeheight;
static unsigned char edid[I915_EDIDSIZE];

static unsigned int readreg(unsigned int offset)
{

    return *(volatile unsigned int *)(mmio + offset);

}

static void writereg(unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(mmio + offset) = value;

}

static unsigned int isdevice(unsigned short device)
{

    unsigned int i;

    for (i = 0; i < I915_DEVICES; i++)
    {

        if (devices[i] == device)
            return 1;

    }

    return 0;

}

static unsigned int ownsframebuffer(unsigned int id)
{

    unsigned int i;

    for (i = 0; i < 6; i++)
    {

        if (pci_getbar(id, i) == framebuffer->address)
            return 1;

    }

    return 0;

}

/* sends aux->count bytes of aux->data on AUX A (the eDP panel's) and puts the reply there, four bytes per register, first byte in the top bits */
static unsigned int auxtransfer(struct aux *aux)
{

    unsigned int status = I915_AUX_CTL_BUSY;
    unsigned int i;

    for (i = 0; i < I915_AUX_SIZE; i += 4)
        writereg(I915_AUX_DATA_A + i, ((unsigned int)aux->data[i] << 24) | ((unsigned int)aux->data[i + 1] << 16) | ((unsigned int)aux->data[i + 2] << 8) | aux->data[i + 3]);

    writereg(I915_AUX_CTL_A, I915_AUX_CTL_BUSY | I915_AUX_CTL_DONE | I915_AUX_CTL_TIMEOUT | I915_AUX_CTL_TIMEOUTMAX | I915_AUX_CTL_RECEIVEERROR | (aux->count << I915_AUX_CTL_SIZESHIFT) | I915_AUX_CTL_SYNC);

    for (i = 0; i < I915_TIMEOUT && (status & I915_AUX_CTL_BUSY); i++)
        status = readreg(I915_AUX_CTL_A);

    if (status & I915_AUX_CTL_BUSY)
        return 0;

    writereg(I915_AUX_CTL_A, status | I915_AUX_CTL_DONE | I915_AUX_CTL_TIMEOUT | I915_AUX_CTL_RECEIVEERROR);

    if (!(status & I915_AUX_CTL_DONE) || (status & (I915_AUX_CTL_TIMEOUT | I915_AUX_CTL_RECEIVEERROR)))
        return 0;

    aux->count = (status >> I915_AUX_CTL_SIZESHIFT) & I915_AUX_CTL_SIZE;

    if (!aux->count || aux->count > I915_AUX_SIZE)
        return 0;

    for (i = 0; i < I915_AUX_SIZE; i += 4)
    {

        unsigned int value = readreg(I915_AUX_DATA_A + i);

        aux->data[i] = value >> 24;
        aux->data[i + 1] = value >> 16;
        aux->data[i + 2] = value >> 8;
        aux->data[i + 3] = value;

    }

    return aux->count;

}

/* one request: length 0 is address only, writes send one byte (value); returns the reply's size with its code (data[0]), 0 on failure */
static unsigned int auxrequest(struct aux *aux)
{

    unsigned int i;

    for (i = 0; i < I915_AUX_RETRIES; i++)
    {

        aux->data[0] = (aux->command << 4) | ((aux->address >> 16) & 0x0F);
        aux->data[1] = aux->address >> 8;
        aux->data[2] = aux->address;
        aux->data[3] = aux->length - 1;
        aux->data[4] = aux->value;
        aux->count = (!aux->length) ? 3 : (aux->command & I915_AUX_I2CREAD) ? 4 : 5;

        if (!auxtransfer(aux) || (aux->data[0] & I915_AUX_REPLYDEFER))
            continue;

        if (aux->data[0] & I915_AUX_REPLYNACK)
            return 0;

        return aux->count;

    }

    return 0;

}

static unsigned int auxread(struct aux *aux, unsigned int command, unsigned int address, unsigned int length)
{

    aux->command = command;
    aux->address = address;
    aux->length = length;
    aux->value = 0;

    return auxrequest(aux) == length + 1;

}

/* EDID over I2C-over-AUX: offset 0, then 16-byte reads, then a stop (address only, no MOT) */
static unsigned int readedid(struct aux *aux)
{

    unsigned int status;
    unsigned int offset;

    aux->command = I915_AUX_I2CWRITE | I915_AUX_I2CMOT;
    aux->address = I915_EDIDADDRESS;
    aux->length = 1;
    aux->value = 0;
    status = auxrequest(aux) != 0;

    for (offset = 0; status && offset < I915_EDIDSIZE; offset += I915_EDIDCHUNK)
    {

        status = auxread(aux, I915_AUX_I2CREAD | I915_AUX_I2CMOT, I915_EDIDADDRESS, I915_EDIDCHUNK);

        if (status)
            buffer_copy(edid + offset, aux->data + 1, I915_EDIDCHUNK);

    }

    auxread(aux, I915_AUX_I2CREAD, I915_EDIDADDRESS, 0);

    return status && edid[0] == 0x00 && edid[1] == 0xFF && edid[7] == 0x00;

}

/* what the panel says about itself, for the log: DPCD revision, link limits, preferred mode (the first detailed timing) */
static void probepanel(void)
{

    struct aux aux;

    if (auxread(&aux, I915_AUX_NATIVEREAD, 0x000, 3))
    {

        unsigned int revision = aux.data[1];
        unsigned int rate = aux.data[2] * 270;
        unsigned int lanes = aux.data[3] & 0x1F;

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: DPCD 0x%H2u, %u Mbps, %u lanes", &revision, &rate, &lanes);

    }

    else
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no DPCD on AUX A");

        return;

    }

    if (readedid(&aux))
    {

        unsigned int clock = (edid[I915_EDIDTIMING] | (edid[I915_EDIDTIMING + 1] << 8)) * 10;
        unsigned int width = edid[I915_EDIDTIMING + 2] | ((edid[I915_EDIDTIMING + 4] & 0xF0) << 4);
        unsigned int height = edid[I915_EDIDTIMING + 5] | ((edid[I915_EDIDTIMING + 7] & 0xF0) << 4);

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: panel %ux%u at %u kHz, firmware %ux%u", &width, &height, &clock, &activewidth, &activeheight);

    }

    else
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no EDID on AUX A");

    }

}

/* the firmware's mode: transcoder A lit, its plane linear XRGB8888 over the whole pipe, showing the firmware framebuffer */
static unsigned int checkfirmware(void)
{

    unsigned int control = readreg(I915_PLANE_CTL_1_A);
    unsigned int source = readreg(I915_PIPESRC_A);

    if (!(readreg(I915_TRANS_CONF_A) & I915_TRANS_CONF_ENABLE) || !(control & I915_PLANE_CTL_ENABLE))
        return 0;

    if ((control & I915_PLANE_CTL_FORMAT) != I915_PLANE_CTL_FORMAT_XRGB8888 || (control & I915_PLANE_CTL_TILED))
        return 0;

    if (readreg(I915_PS_CTRL_1_A) & I915_PS_CTRL_ENABLE)
        return 0;

    return (source >> 16) + 1 == framebuffer->width && (source & 0xFFFF) + 1 == framebuffer->height && activewidth == framebuffer->width && activeheight == framebuffer->height;

}

/* double-buffered registers latch at the start of vblank: don't start writing them in the lines just before it */
static void evadevblank(void)
{

    unsigned int i;

    for (i = 0; i < I915_TIMEOUT; i++)
    {

        unsigned int line = readreg(I915_PIPEDSL_A) & I915_PIPEDSL_LINE;

        if (line + I915_EVASION < activeheight || line >= activeheight)
            return;

    }

}

/* the pipe scaler stretches the source to the largest centered window with its aspect ratio; the rest shows the pipe's black background */
static void setscaler(unsigned int width, unsigned int height)
{

    unsigned int w = activewidth;
    unsigned int h = activewidth * height / width;

    if (h > activeheight)
    {

        h = activeheight;
        w = activeheight * width / height;

    }

    if (width == activewidth && height == activeheight)
    {

        writereg(I915_PS_CTRL_1_A, 0);
        writereg(I915_PS_WIN_POS_1_A, 0);
        writereg(I915_PS_WIN_SZ_1_A, 0);

    }

    else
    {

        writereg(I915_PS_CTRL_1_A, I915_PS_CTRL_ENABLE);
        writereg(I915_PS_WIN_POS_1_A, (((activewidth - w) / 2) << 16) | ((activeheight - h) / 2));
        writereg(I915_PS_WIN_SZ_1_A, (w << 16) | h);

    }

}

/* modes up to the firmware's size, width a multiple of 16 (the stride is counted in 64 bytes); others keep the current mode */
static void videointerface_setmode(unsigned int width, unsigned int height, unsigned int bpp)
{

    if (mmio && bpp == 4 && width >= I915_MINSIZE && height >= I915_MINSIZE && width <= activewidth && height <= activeheight && !(width % 16))
    {

        unsigned int surface = readreg(I915_PLANE_SURF_1_A);

        evadevblank();
        writereg(I915_PIPESRC_A, ((width - 1) << 16) | (height - 1));
        setscaler(width, height);
        writereg(I915_PLANE_STRIDE_1_A, width * 4 / 64);
        writereg(I915_PLANE_SIZE_1_A, ((height - 1) << 16) | (width - 1));
        writereg(I915_PLANE_SURF_1_A, surface);

        videointerface.width = width;
        videointerface.height = height;

    }

    arch_kmap(framebuffer->address, 0xA0000000, framebuffer->pitch * framebuffer->height, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, 0xA0000000, 0, videointerface_setmode);

    videointerface.width = framebuffer->width;
    videointerface.height = framebuffer->height;
    videointerface.bpp = 4;

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    if (type != PCI_BUS || !framebuffer->address || framebuffer->bpp != 32 || framebuffer->pitch != framebuffer->width * 4)
        return 0;

    return pci_inw(id, PCI_CONFIG_VENDOR) == I915_VENDOR && isdevice(pci_inw(id, PCI_CONFIG_DEVICE)) && ownsframebuffer(id);

}

/* without the firmware's mode as expected the driver only shows the firmware framebuffer, like gop */
static void driver_reset(unsigned int id)
{

    mmio = pci_getbar(id, 0);

    if (!mmio)
        return;

    arch_kmap(mmio, mmio, I915_MMIOSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    activewidth = (readreg(I915_TRANS_HTOTAL_A) & 0xFFFF) + 1;
    activeheight = (readreg(I915_TRANS_VTOTAL_A) & 0xFFFF) + 1;

    if (checkfirmware())
        probepanel();
    else
        mmio = 0;

}

static void driver_attach(unsigned int id)
{

    video_registerinterface(&videointerface);

}

static void driver_detach(unsigned int id)
{

    video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "i915", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

