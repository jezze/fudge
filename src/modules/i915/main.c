#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/video.h>
#include <modules/pci/pci.h>
#include "i915.h"

struct hdmi
{

    unsigned int found;
    unsigned int port;
    unsigned int ddc;
    unsigned int typec;

};

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
static struct hdmi hdmi;
static unsigned long opregion;
static unsigned int opregionsize;
static char ports[I915_PORTS] = "ABCD???BCDAEEFFGGHHII";

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

static unsigned int read8(unsigned int offset)
{

    return *(volatile unsigned char *)(opregion + offset);

}

static unsigned int read16(unsigned int offset)
{

    return read8(offset) | (read8(offset + 1) << 8);

}

static unsigned int read32(unsigned int offset)
{

    return read16(offset) | (read16(offset + 2) << 16);

}

static unsigned int issignature(unsigned int offset, char *signature)
{

    unsigned int i;

    for (i = 0; signature[i]; i++)
    {

        if (read8(offset + i) != (unsigned char)signature[i])
            return 0;

    }

    return 1;

}

static unsigned int findvbt(void)
{

    if (!issignature(0, "IntelGraphicsMem"))
        return 0;

    if ((read32(I915_OPREGION_MAILBOXES) & I915_OPREGION_MAILBOX_VBT) && issignature(I915_OPREGION_VBT, "$VBT"))
        return I915_OPREGION_VBT;

    if (read8(I915_OPREGION_VERSION + 3) > 2 || (read8(I915_OPREGION_VERSION + 3) == 2 && read8(I915_OPREGION_VERSION + 2) >= 1))
    {

        unsigned int rvda = read32(I915_OPREGION_RVDA);

        if (!read32(I915_OPREGION_RVDA + 4) && rvda && rvda + read32(I915_OPREGION_RVDS) <= opregionsize && issignature(rvda, "$VBT"))
            return rvda;

    }

    return 0;

}

static char *kindof(unsigned int type)
{

    unsigned int hdmi = (type & I915_CHILD_TYPE_TMDS) && !(type & I915_CHILD_TYPE_NOTHDMI);

    if (type & I915_CHILD_TYPE_DP)
        return (type & I915_CHILD_TYPE_INTERNAL) ? "eDP" : (hdmi) ? "DP+HDMI" : "DP";

    return (hdmi) ? "HDMI" : "other";

}

static void logchild(unsigned int child, unsigned int size)
{

    unsigned int type = read16(child + I915_CHILD_TYPE);
    unsigned int dvo = read8(child + I915_CHILD_PORT);
    unsigned int ddc = read8(child + I915_CHILD_DDC);
    unsigned int aux = read8(child + I915_CHILD_AUX);
    unsigned int flags = (size >= I915_CHILD_MINSIZE) ? read8(child + I915_CHILD_FLAGS) : 0;
    char *kind = kindof(type);
    char *link = (flags & I915_CHILD_FLAGS_TBT) ? " Type-C/TBT" : (flags & I915_CHILD_FLAGS_TYPEC) ? " Type-C" : "";
    char port[2];

    port[0] = (dvo < I915_PORTS - 1) ? ports[dvo] : '?';
    port[1] = '\0';

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: VBT port %s %s%s, type 0x%H4u, ddc %u, aux 0x%H2u", port, kind, link, &type, &ddc, &aux);

    /* the first HDMI-capable connector, but a plain HDMI socket over a Type-C one */
    if ((!hdmi.found || (hdmi.typec && !(flags & (I915_CHILD_FLAGS_TYPEC | I915_CHILD_FLAGS_TBT)))) && port[0] != '?' && ddc && (type & I915_CHILD_TYPE_TMDS) && !(type & (I915_CHILD_TYPE_NOTHDMI | I915_CHILD_TYPE_INTERNAL)))
    {

        hdmi.found = 1;
        hdmi.port = port[0] - 'A';
        hdmi.ddc = ddc;
        hdmi.typec = flags & (I915_CHILD_FLAGS_TYPEC | I915_CHILD_FLAGS_TBT);

    }

}

static void probevbt(unsigned int id)
{

    unsigned long asls = pci_ind(id, I915_ASLS);
    unsigned int vbt;
    unsigned int bdb;
    unsigned int end;
    unsigned int index;

    if (!asls)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no OpRegion");

        return;

    }

    arch_kmap(asls & ~0xFFFUL, I915_OPREGIONMAP, I915_OPREGIONSIZE, MMAP_FLAG_GLOBAL);

    opregion = I915_OPREGIONMAP + (asls & 0xFFF);
    opregionsize = I915_OPREGIONSIZE - (asls & 0xFFF);
    vbt = findvbt();

    if (!vbt || vbt + read32(vbt + I915_VBT_BDB) + I915_BDB_SIZE + 2 > opregionsize)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no VBT in the OpRegion");

        return;

    }

    bdb = vbt + read32(vbt + I915_VBT_BDB);
    end = bdb + read16(bdb + I915_BDB_SIZE);

    if (!issignature(bdb, "BIOS_DATA_BLOCK") || end > opregionsize)
        return;

    for (index = bdb + read16(bdb + I915_BDB_HEADERSIZE); index + 3 <= end; index += 3 + read16(index + 1))
    {

        if (read8(index) == I915_BDB_GENERAL)
        {

            unsigned int block = index + 3;
            unsigned int size = read8(block + I915_GENERAL_CHILDSIZE);
            unsigned int last = block + read16(index + 1);
            unsigned int version = read16(bdb + I915_BDB_VERSION);
            unsigned int child;

            debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: VBT version %u, child devices of %u bytes", &version, &size);

            if (!size || last > end)
                return;

            for (child = block + I915_GENERAL_CHILDREN; child + size <= last; child += size)
            {

                if (read16(child + I915_CHILD_TYPE))
                    logchild(child, size);

            }

            return;

        }

    }

    debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no general definitions in the VBT");

}

static unsigned int isconnected(unsigned int port)
{

    unsigned int control = (port < I915_COMBOPORTS) ? I915_SHOTPLUG_CTL_DDI : I915_SHOTPLUG_CTL_TC;
    unsigned int pin = (port < I915_COMBOPORTS) ? port : port - I915_COMBOPORTS;
    unsigned int status = (port < I915_COMBOPORTS) ? I915_SDEISR_COMBO + pin : I915_SDEISR_TC + pin;
    unsigned int value = readreg(control);
    unsigned int i;

    if (!(value & (I915_SHOTPLUG_ENABLE << (pin * I915_SHOTPLUG_BITS))))
    {

        writereg(control, value | (I915_SHOTPLUG_ENABLE << (pin * I915_SHOTPLUG_BITS)));

        for (i = 0; i < I915_TIMEOUT; i++)
            readreg(I915_SDEISR);

    }

    return (readreg(I915_SDEISR) >> status) & 1;

}

static unsigned int gmbuswait(unsigned int bits)
{

    unsigned int i;

    for (i = 0; i < I915_TIMEOUT; i++)
    {

        unsigned int status = readreg(I915_GMBUS2);

        if (status & I915_GMBUS2_NAK)
            return 0;

        if (status & bits)
            return 1;

    }

    return 0;

}

static void gmbusreset(void)
{

    writereg(I915_GMBUS1, I915_GMBUS1_CLEAR);
    writereg(I915_GMBUS1, 0);
    writereg(I915_GMBUS0, 0);

}

static unsigned int readgmbusedid(unsigned int pin)
{

    unsigned int offset;
    unsigned int i;

    writereg(I915_GMBUS0, pin | I915_GMBUS0_RATE100KHZ);
    writereg(I915_GMBUS5, 0);
    writereg(I915_GMBUS1, I915_GMBUS1_READY | I915_GMBUS1_WAIT | I915_GMBUS1_INDEX | (I915_EDIDSIZE << I915_GMBUS1_COUNTSHIFT) | (I915_EDIDADDRESS << I915_GMBUS1_ADDRESSSHIFT) | I915_GMBUS1_READ);

    for (offset = 0; offset < I915_EDIDSIZE; offset += 4)
    {

        unsigned int value;

        if (!gmbuswait(I915_GMBUS2_HWREADY))
        {

            gmbusreset();

            return 0;

        }

        value = readreg(I915_GMBUS3);

        for (i = 0; i < 4; i++)
            edid[offset + i] = value >> (i * 8);

    }

    if (!gmbuswait(I915_GMBUS2_WAITPHASE))
    {

        gmbusreset();

        return 0;

    }

    writereg(I915_GMBUS1, I915_GMBUS1_READY | I915_GMBUS1_STOP);

    for (i = 0; i < I915_TIMEOUT; i++)
    {

        if (!(readreg(I915_GMBUS2) & I915_GMBUS2_ACTIVE))
            break;

    }

    writereg(I915_GMBUS0, 0);

    return edid[0] == 0x00 && edid[1] == 0xFF && edid[7] == 0x00;

}

static void logedid(char *name)
{

    unsigned int clock = (edid[I915_EDIDTIMING] | (edid[I915_EDIDTIMING + 1] << 8)) * 10;
    unsigned int width = edid[I915_EDIDTIMING + 2] | ((edid[I915_EDIDTIMING + 4] & 0xF0) << 4);
    unsigned int height = edid[I915_EDIDTIMING + 5] | ((edid[I915_EDIDTIMING + 7] & 0xF0) << 4);
    unsigned int extensions = edid[126];

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: %s %ux%u at %u kHz, %u extension blocks", name, &width, &height, &clock, &extensions);

}

static void probehdmi(void)
{

    unsigned int pin;
    unsigned int connected;
    char port[2];

    if (!hdmi.found)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no HDMI connector in the VBT");

        return;

    }

    pin = (hdmi.ddc <= I915_DDCCOMBOPINS) ? hdmi.ddc : hdmi.ddc + I915_DDCTCPIN;
    connected = isconnected(hdmi.port);
    port[0] = 'A' + hdmi.port;
    port[1] = '\0';

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: HDMI on port %s, GMBUS pin %u, %s", port, &pin, (connected) ? "connected" : "not connected");

    if (readgmbusedid(pin))
        logedid("monitor");
    else
        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no EDID over DDC");

}

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

        logedid("panel");
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: firmware %ux%u", &activewidth, &activeheight);

    }

    else
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no EDID on AUX A");

    }

}

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

static void driver_reset(unsigned int id)
{

    mmio = pci_getbar(id, 0);

    if (!mmio)
        return;

    arch_kmap(mmio, mmio, I915_MMIOSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    probevbt(id);
    probehdmi();

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

