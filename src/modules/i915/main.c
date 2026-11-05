#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/video.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>
#include <modules/pci/pci.h>
#include "i915.h"

struct mode
{

    unsigned int clock;
    unsigned int hactive;
    unsigned int hsyncstart;
    unsigned int hsyncend;
    unsigned int htotal;
    unsigned int vactive;
    unsigned int vsyncstart;
    unsigned int vsyncend;
    unsigned int vtotal;
    unsigned int sync;
    unsigned int divider;

};

struct hdmi
{

    unsigned int found;
    unsigned int port;
    unsigned int ddc;
    unsigned int typec;
    unsigned int connected;
    unsigned int pll;
    struct mode mode;

};

struct pipe
{

    unsigned int registers;
    unsigned int scaler;
    unsigned int on;
    unsigned int activewidth;
    unsigned int activeheight;

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
static struct video_interface secondinterface;
static unsigned int secondsurface;
static unsigned int secondsize;
static struct arch_framebuffer *framebuffer = (struct arch_framebuffer *)ARCH_FIRMWARE_FRAMEBUFFER;
static unsigned short devices[I915_DEVICES] = {0x9A40, 0x9A49, 0x9A59, 0x9A60, 0x9A68, 0x9A70, 0x9A78, 0x9AC0, 0x9AC9, 0x9AD9, 0x9AF8};
static unsigned long mmio;
static struct pipe pipea = {0, 0, 0, 0, 0};
static struct pipe pipeb = {I915_PIPESTRIDE, I915_SCALERSTRIDE, 0, 0, 0};
static unsigned char edid[I915_EDIDBUFFER];
static unsigned char dividers[I915_DIVIDERS] = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 24, 28, 30, 32, 36, 40, 42, 44, 48, 52, 54, 56, 60, 64, 66, 68, 70, 72, 76, 78, 80, 84, 88, 90, 92, 96, 98};
static struct mode cea1080 = {148500, 1920, 2008, 2052, 2200, 1080, 1084, 1089, 1125, I915_DDI_FUNC_VSYNCHIGH | I915_DDI_FUNC_HSYNCHIGH, 12};
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

static unsigned int readgmbusedid(unsigned int pin, unsigned int block)
{

    unsigned int start = block * I915_EDIDSIZE;
    unsigned int offset;
    unsigned int i;

    writereg(I915_GMBUS0, pin | I915_GMBUS0_RATE100KHZ);
    writereg(I915_GMBUS5, 0);
    writereg(I915_GMBUS1, I915_GMBUS1_READY | I915_GMBUS1_WAIT | I915_GMBUS1_INDEX | (I915_EDIDSIZE << I915_GMBUS1_COUNTSHIFT) | (start << I915_GMBUS1_INDEXSHIFT) | (I915_EDIDADDRESS << I915_GMBUS1_ADDRESSSHIFT) | I915_GMBUS1_READ);

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
            edid[start + offset + i] = value >> (i * 8);

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

    return (block) ? edid[start] == I915_EDIDCTA : edid[0] == 0x00 && edid[1] == 0xFF && edid[7] == 0x00;

}

static unsigned int finddivider(unsigned int clock)
{

    unsigned int best = 0;
    unsigned int distance = I915_DCOMAX;
    unsigned int i;

    for (i = 0; i < I915_DIVIDERS; i++)
    {

        unsigned int dco = clock * 5 * dividers[i];
        unsigned int d = (dco > I915_DCOMID) ? dco - I915_DCOMID : I915_DCOMID - dco;

        if (dco >= I915_DCOMIN && dco <= I915_DCOMAX && d < distance)
        {

            best = dividers[i];
            distance = d;

        }

    }

    return best;

}

static struct mode readtiming(unsigned int offset)
{

    struct mode mode;
    unsigned int flags = edid[offset + 17];

    mode.clock = (edid[offset] | (edid[offset + 1] << 8)) * 10;
    mode.hactive = edid[offset + 2] | ((edid[offset + 4] & 0xF0) << 4);
    mode.hsyncstart = mode.hactive + (edid[offset + 8] | ((edid[offset + 11] & 0xC0) << 2));
    mode.hsyncend = mode.hsyncstart + (edid[offset + 9] | ((edid[offset + 11] & 0x30) << 4));
    mode.htotal = mode.hactive + (edid[offset + 3] | ((edid[offset + 4] & 0x0F) << 8));
    mode.vactive = edid[offset + 5] | ((edid[offset + 7] & 0xF0) << 4);
    mode.vsyncstart = mode.vactive + ((edid[offset + 10] >> 4) | ((edid[offset + 11] & 0x0C) << 2));
    mode.vsyncend = mode.vsyncstart + ((edid[offset + 10] & 0x0F) | ((edid[offset + 11] & 0x03) << 4));
    mode.vtotal = mode.vactive + (edid[offset + 6] | ((edid[offset + 7] & 0x0F) << 8));
    mode.sync = 0;
    mode.divider = finddivider(mode.clock);

    if ((flags & I915_DTD_DIGITALSEPARATE) == I915_DTD_DIGITALSEPARATE)
        mode.sync = ((flags & I915_DTD_VSYNCHIGH) ? I915_DDI_FUNC_VSYNCHIGH : 0) | ((flags & I915_DTD_HSYNCHIGH) ? I915_DDI_FUNC_HSYNCHIGH : 0);

    if (flags & I915_DTD_INTERLACED)
        mode.clock = 0;

    return mode;

}

static unsigned int isusable(struct mode *mode)
{

    return mode->clock && mode->clock <= I915_HDMI_MAXCLOCK && mode->divider && mode->hactive >= I915_HDMI_MINWIDTH && mode->vactive >= I915_HDMI_MINHEIGHT && mode->hactive <= I915_HDMI_MAXWIDTH && mode->vactive <= I915_HDMI_MAXHEIGHT && mode->hsyncend < mode->htotal && mode->vsyncend < mode->vtotal;

}

static void considertiming(unsigned int offset)
{

    struct mode mode = readtiming(offset);
    unsigned int area = mode.hactive * mode.vactive;
    unsigned int best = hdmi.mode.hactive * hdmi.mode.vactive;

    if (isusable(&mode) && (area > best || (area == best && mode.clock > hdmi.mode.clock)))
        buffer_copy(&hdmi.mode, &mode, sizeof (struct mode));

}

static void choosemode(unsigned int blocks)
{

    unsigned int offset;

    buffer_clear(&hdmi.mode, sizeof (struct mode));

    for (offset = 54; offset < 54 + I915_EDIDDESCRIPTORS * I915_EDIDDESCRIPTOR; offset += I915_EDIDDESCRIPTOR)
        considertiming(offset);

    if (blocks > 1 && edid[I915_EDIDSIZE + 2] >= 4)
    {

        for (offset = I915_EDIDSIZE + edid[I915_EDIDSIZE + 2]; offset + I915_EDIDDESCRIPTOR < I915_EDIDBUFFER && (edid[offset] || edid[offset + 1]); offset += I915_EDIDDESCRIPTOR)
            considertiming(offset);

    }

    if (!hdmi.mode.clock)
        buffer_copy(&hdmi.mode, &cea1080, sizeof (struct mode));

}

static void logedid(char *name)
{

    unsigned int clock = (edid[I915_EDIDTIMING] | (edid[I915_EDIDTIMING + 1] << 8)) * 10;
    unsigned int width = edid[I915_EDIDTIMING + 2] | ((edid[I915_EDIDTIMING + 4] & 0xF0) << 4);
    unsigned int height = edid[I915_EDIDTIMING + 5] | ((edid[I915_EDIDTIMING + 7] & 0xF0) << 4);
    unsigned int extensions = edid[126];

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: %s %ux%u at %u kHz, %u extension blocks", name, &width, &height, &clock, &extensions);

}

static void evadevblank(struct pipe *pipe)
{

    unsigned int i;

    for (i = 0; i < I915_TIMEOUT; i++)
    {

        unsigned int line = readreg(I915_PIPEDSL_A + pipe->registers) & I915_PIPEDSL_LINE;

        if (line + I915_EVASION < pipe->activeheight || line >= pipe->activeheight)
            return;

    }

}

static void setscaler(struct pipe *pipe, unsigned int width, unsigned int height)
{

    unsigned int w = pipe->activewidth;
    unsigned int h = pipe->activewidth * height / width;

    if (h > pipe->activeheight)
    {

        h = pipe->activeheight;
        w = pipe->activeheight * width / height;

    }

    if (width == pipe->activewidth && height == pipe->activeheight)
    {

        writereg(I915_PS_CTRL_1_A + pipe->scaler, 0);
        writereg(I915_PS_WIN_POS_1_A + pipe->scaler, 0);
        writereg(I915_PS_WIN_SZ_1_A + pipe->scaler, 0);

    }

    else
    {

        writereg(I915_PS_CTRL_1_A + pipe->scaler, I915_PS_CTRL_ENABLE);
        writereg(I915_PS_WIN_POS_1_A + pipe->scaler, (((pipe->activewidth - w) / 2) << 16) | ((pipe->activeheight - h) / 2));
        writereg(I915_PS_WIN_SZ_1_A + pipe->scaler, (w << 16) | h);

    }

}

static void setsource(struct pipe *pipe, unsigned int width, unsigned int height, unsigned int surface)
{

    evadevblank(pipe);
    writereg(I915_PIPESRC_A + pipe->registers, ((width - 1) << 16) | (height - 1));
    setscaler(pipe, width, height);
    writereg(I915_PLANE_STRIDE_1_A + pipe->registers, width * 4 / 64);
    writereg(I915_PLANE_SIZE_1_A + pipe->registers, ((height - 1) << 16) | (width - 1));
    writereg(I915_PLANE_SURF_1_A + pipe->registers, surface);

}

static unsigned int waitreg(unsigned int offset, unsigned int mask, unsigned int value)
{

    unsigned int i;

    for (i = 0; i < I915_TIMEOUT; i++)
    {

        if ((readreg(offset) & mask) == value)
            return 1;

    }

    return 0;

}

static void modifyreg(unsigned int offset, unsigned int clear, unsigned int set)
{

    writereg(offset, (readreg(offset) & ~clear) | set);

}

static unsigned int fail(char *step)
{

    debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: HDMI stopped at %s", step);

    return 0;

}

static unsigned int powerup(void)
{

    modifyreg(I915_PWR_WELL_CTL, 0, (I915_PWR_WELL_PW2 << I915_PWR_WELL_REQUEST));

    if (!waitreg(I915_PWR_WELL_CTL, I915_PWR_WELL_PW2, I915_PWR_WELL_PW2) || !waitreg(I915_FUSE_STATUS, I915_FUSE_PG2, I915_FUSE_PG2))
        return fail("power well 2");

    modifyreg(I915_PWR_WELL_CTL, 0, (I915_PWR_WELL_PW3 << I915_PWR_WELL_REQUEST));

    if (!waitreg(I915_PWR_WELL_CTL, I915_PWR_WELL_PW3, I915_PWR_WELL_PW3) || !waitreg(I915_FUSE_STATUS, I915_FUSE_PG3, I915_FUSE_PG3))
        return fail("power well 3");

    modifyreg(I915_PWR_WELL_CTL_DDI, 0, (I915_PWR_WELL_DDIB << I915_PWR_WELL_REQUEST));

    if (!waitreg(I915_PWR_WELL_CTL_DDI, I915_PWR_WELL_DDIB, I915_PWR_WELL_DDIB))
        return fail("DDI B power");

    modifyreg(I915_DBUF_CTL_S2, I915_DBUF_TRACKER, I915_DBUF_REQUEST | I915_DBUF_TRACKERVALUE);

    if (!waitreg(I915_DBUF_CTL_S2, I915_DBUF_STATE, I915_DBUF_STATE))
        return fail("DBUF slice 2");

    return 1;

}

static unsigned int enablepll(void)
{

    unsigned int clocks = readreg(I915_DPCLKA_CFGCR0);
    unsigned int pll = (!(clocks & I915_DPCLKA_DDIAOFF) && !((clocks >> I915_DPCLKA_DDIASHIFT) & I915_DPCLKA_DDIA)) ? 1 : 0;
    unsigned int enable = I915_DPLL_ENABLE + pll * 4;
    unsigned int reference = ((readreg(I915_DSSM) & I915_DSSM_REFCLK) == I915_DSSM_REFCLK24) ? 24000 : 19200;
    unsigned int divider = hdmi.mode.divider;
    unsigned int dco = hdmi.mode.clock * 5 * divider;
    unsigned int p = (divider == 2 || !(divider % 4)) ? 2 : (!(divider % 6)) ? 3 : (!(divider % 5)) ? 5 : 7;
    unsigned int k = (divider == 2) ? 1 : 2;
    unsigned int q = divider / (p * k);
    unsigned int pcode = (p == 2) ? 1 : (p == 3) ? 2 : (p == 5) ? 4 : 8;

    if (readreg(enable) & I915_DPLL_ON)
        return fail("DPLL busy");

    hdmi.pll = pll;

    modifyreg(enable, 0, I915_DPLL_POWER);

    if (!waitreg(enable, I915_DPLL_POWERSTATE, I915_DPLL_POWERSTATE))
        return fail("DPLL power");

    writereg(I915_DPLL_CFGCR0 + pll * I915_DPLL_STRIDE, ((((dco % reference) << 15) / reference) << I915_DPLL_FRACTIONSHIFT) | (dco / reference));
    writereg(I915_DPLL_CFGCR1 + pll * I915_DPLL_STRIDE, (q << I915_DPLL_QDIVSHIFT) | ((q > 1) ? I915_DPLL_QDIVMODE : 0) | (k << I915_DPLL_KDIVSHIFT) | (pcode << I915_DPLL_PDIVSHIFT));
    readreg(I915_DPLL_CFGCR1 + pll * I915_DPLL_STRIDE);
    modifyreg(enable, 0, I915_DPLL_ON);

    if (!waitreg(enable, I915_DPLL_LOCK, I915_DPLL_LOCK))
        return fail("DPLL lock");

    modifyreg(I915_DPCLKA_CFGCR0, I915_DPCLKA_DDIA << I915_DPCLKA_DDIASHIFTB, pll << I915_DPCLKA_DDIASHIFTB);
    modifyreg(I915_DPCLKA_CFGCR0, I915_DPCLKA_DDIBOFF, 0);

    return 1;

}

static void setswing(void)
{

    unsigned int lane;

    modifyreg(I915_PHYB_CL_DW10, I915_PWR_DOWN_LN, 0);
    writereg(I915_PHYB_PCS_DW1_GRP, readreg(I915_PHYB_PCS_DW1_LN0) & ~I915_COMMON_KEEPER);

    for (lane = 0; lane < I915_LANES; lane++)
        modifyreg(I915_PHYB_TX_DW4_LN0 + lane * I915_PHY_LANESTRIDE, I915_TX_DW4_LOADGEN, (lane) ? I915_TX_DW4_LOADGEN : 0);

    modifyreg(I915_PHYB_CL_DW5, 0, I915_SUS_CLOCK);
    writereg(I915_PHYB_TX_DW5_GRP, readreg(I915_PHYB_TX_DW5_LN0) & ~I915_TX_DW5_TRAINING);
    writereg(I915_PHYB_TX_DW5_GRP, (readreg(I915_PHYB_TX_DW5_LN0) & ~(I915_TX_DW5_SCALING | I915_TX_DW5_RTERM | I915_TX_DW5_TAP2 | I915_TX_DW5_TAP3)) | I915_TX_DW5_SCALINGVALUE | I915_TX_DW5_RTERMVALUE | I915_TX_DW5_TAP3);
    writereg(I915_PHYB_TX_DW2_GRP, (readreg(I915_PHYB_TX_DW2_LN0) & ~(I915_TX_DW2_SWINGUPPER | I915_TX_DW2_SWINGLOWER | I915_TX_DW2_RCOMP)) | (((I915_HDMI_SWING >> 3) & 1) << 15) | ((I915_HDMI_SWING & 7) << 11) | I915_TX_DW2_RCOMPVALUE);

    for (lane = 0; lane < I915_LANES; lane++)
        modifyreg(I915_PHYB_TX_DW4_LN0 + lane * I915_PHY_LANESTRIDE, I915_TX_DW4_CURSORS, (I915_HDMI_POSTCURSOR << I915_TX_DW4_POST1SHIFT) | I915_HDMI_CURSOR);

    writereg(I915_PHYB_TX_DW7_GRP, (readreg(I915_PHYB_TX_DW7_LN0) & ~I915_TX_DW7_NSCALAR) | (I915_HDMI_NSCALAR << I915_TX_DW7_NSCALARSHIFT));
    writereg(I915_PHYB_TX_DW5_GRP, readreg(I915_PHYB_TX_DW5_LN0) | I915_TX_DW5_TRAINING);

}

static void settimings(struct mode *mode)
{

    writereg(I915_TRANS_HTOTAL_B, ((mode->htotal - 1) << 16) | (mode->hactive - 1));
    writereg(I915_TRANS_HBLANK_B, ((mode->htotal - 1) << 16) | (mode->hactive - 1));
    writereg(I915_TRANS_HSYNC_B, ((mode->hsyncend - 1) << 16) | (mode->hsyncstart - 1));
    writereg(I915_TRANS_VTOTAL_B, ((mode->vtotal - 1) << 16) | (mode->vactive - 1));
    writereg(I915_TRANS_VBLANK_B, ((mode->vtotal - 1) << 16) | (mode->vactive - 1));
    writereg(I915_TRANS_VSYNC_B, ((mode->vsyncend - 1) << 16) | (mode->vsyncstart - 1));
    writereg(I915_TRANS_VSYNCSHIFT_B, 0);
    writereg(I915_PIPESRC_B, ((mode->hactive - 1) << 16) | (mode->vactive - 1));

}

static void setplane(void)
{

    unsigned int level;

    writereg(I915_PIPE_MBUS_DBOX_CTL_B, readreg(I915_PIPE_MBUS_DBOX_CTL_A));
    writereg(I915_WM_LINETIME_B, readreg(I915_WM_LINETIME_A));
    writereg(I915_PLANE_BUF_CFG_1_B, ((I915_DBUFSLICE * 2 - 1) << 16) | I915_DBUFSLICE);

    for (level = 0; level < I915_WMLEVELS; level++)
        writereg(I915_PLANE_WM_1_B + level * 4, readreg(I915_PLANE_WM_1_A + level * 4));

    writereg(I915_PLANE_WM_TRANS_1_B, readreg(I915_PLANE_WM_TRANS_1_A));
    writereg(I915_PLANE_COLOR_CTL_1_B, I915_PLANE_COLOR_GAMMADISABLE);
    writereg(I915_PLANE_POS_1_B, 0);
    writereg(I915_PLANE_OFFSET_1_B, 0);
    writereg(I915_PLANE_CTL_1_B, readreg(I915_PLANE_CTL_1_A));
    setsource(&pipeb, secondinterface.width, secondinterface.height, secondsurface);

}

static unsigned int enablehdmi(void)
{

    if (hdmi.port != 1)
        return fail("port not B");

    if (readreg(I915_TRANS_CONF_B) & I915_TRANS_CONF_ENABLE)
        return fail("transcoder B in use");

    if (!(readreg(I915_PHYB_COMP_DW0) & I915_COMP_INIT))
        return fail("combo PHY B not initialized");

    if (!powerup() || !enablepll())
        return 0;

    setswing();
    writereg(I915_TRANS_CLK_SEL_B, I915_TRANS_CLK_SEL_PORTB);
    settimings(&hdmi.mode);
    modifyreg(I915_PIPEMISC_B, I915_PIPEMISC_BPC, 0);
    writereg(I915_TRANS_DDI_FUNC_CTL_B, I915_DDI_FUNC_ENABLE | I915_DDI_FUNC_PORTB | I915_DDI_FUNC_DVI | hdmi.mode.sync);
    writereg(I915_TRANS_CONF_B, I915_TRANS_CONF_ENABLE);

    if (!waitreg(I915_TRANS_CONF_B, I915_TRANS_CONF_STATE, I915_TRANS_CONF_STATE))
        return fail("transcoder B state");

    writereg(I915_DDI_BUF_CTL_B, I915_DDI_BUF_ENABLE);

    if (!waitreg(I915_DDI_BUF_CTL_B, I915_DDI_BUF_IDLE, 0))
        return fail("DDI B buffer idle");

    pipeb.on = 1;
    pipeb.activewidth = hdmi.mode.hactive;
    pipeb.activeheight = hdmi.mode.vactive;
    setplane();
    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: HDMI on, showing video:1");

    return 1;

}

static void disablehdmi(void)
{

    unsigned int enable = I915_DPLL_ENABLE + hdmi.pll * 4;

    pipeb.on = 0;

    writereg(I915_PLANE_CTL_1_B, 0);
    writereg(I915_PLANE_SURF_1_B, 0);
    modifyreg(I915_TRANS_CONF_B, I915_TRANS_CONF_ENABLE, 0);

    if (!waitreg(I915_TRANS_CONF_B, I915_TRANS_CONF_STATE, 0))
        fail("transcoder B off");

    writereg(I915_TRANS_DDI_FUNC_CTL_B, 0);
    writereg(I915_PS_CTRL_1_A + pipeb.scaler, 0);
    writereg(I915_PS_WIN_POS_1_A + pipeb.scaler, 0);
    writereg(I915_PS_WIN_SZ_1_A + pipeb.scaler, 0);
    modifyreg(I915_DDI_BUF_CTL_B, I915_DDI_BUF_ENABLE, 0);

    if (!waitreg(I915_DDI_BUF_CTL_B, I915_DDI_BUF_IDLE, I915_DDI_BUF_IDLE))
        fail("DDI B buffer off");

    writereg(I915_TRANS_CLK_SEL_B, 0);
    modifyreg(I915_DPCLKA_CFGCR0, 0, I915_DPCLKA_DDIBOFF);
    modifyreg(enable, I915_DPLL_ON, 0);

    if (!waitreg(enable, I915_DPLL_LOCK, 0))
        fail("DPLL unlock");

    modifyreg(enable, I915_DPLL_POWER, 0);

    if (!waitreg(enable, I915_DPLL_POWERSTATE, 0))
        fail("DPLL power off");

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: HDMI off");

}

static void probehdmi(void)
{

    unsigned int i;
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
    hdmi.connected = connected;
    port[0] = 'A' + hdmi.port;
    port[1] = '\0';

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: HDMI on port %s, GMBUS pin %u, %s", port, &pin, (connected) ? "connected" : "not connected");

    for (i = 0; i < I915_EDIDTRIES; i++)
    {

        if (readgmbusedid(pin, 0))
            break;

    }

    if (i < I915_EDIDTRIES)
    {

        logedid("monitor");
        choosemode((edid[126] && readgmbusedid(pin, 1)) ? 2 : 1);

    }

    else
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no EDID over DDC");
        choosemode(0);

    }

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: HDMI mode %ux%u at %u kHz", &hdmi.mode.hactive, &hdmi.mode.vactive, &hdmi.mode.clock);

}

static void checkhdmi(void)
{

    if (isconnected(hdmi.port) == pipeb.on)
        return;

    if (pipeb.on)
    {

        disablehdmi();

    }

    else
    {

        probehdmi();

        if (hdmi.connected)
            enablehdmi();

    }

}

static void handleirq(unsigned int irq)
{

    unsigned int master;

    if (!mmio)
        return;

    master = readreg(I915_GFX_MSTR_IRQ);

    if (!(master & I915_MSTR_DISPLAY))
        return;

    writereg(I915_GFX_MSTR_IRQ, 0);

    if (readreg(I915_DISPLAY_INT_CTL) & I915_DISPLAY_INT_PCH)
    {

        unsigned int status = readreg(I915_SDEIIR);

        writereg(I915_SDEIIR, status);
        writereg(I915_SHOTPLUG_CTL_DDI, readreg(I915_SHOTPLUG_CTL_DDI));

        if (status & (1u << (I915_SDEISR_COMBO + hdmi.port)))
            checkhdmi();

    }

    writereg(I915_GFX_MSTR_IRQ, I915_MSTR_ENABLE);

}

static void enablehotplug(unsigned int id)
{

    unsigned int bit = 1u << (I915_SDEISR_COMBO + hdmi.port);

    writereg(I915_SHPD_FILTER_CNT, I915_SHPD_FILTER);
    writereg(I915_SDEIIR, bit);
    modifyreg(I915_SDEIER, 0, bit);
    modifyreg(I915_SDEIMR, bit, 0);
    modifyreg(I915_DISPLAY_INT_CTL, 0, I915_DISPLAY_INT_ENABLE);
    pci_outw(id, PCI_CONFIG_COMMAND, pci_inw(id, PCI_CONFIG_COMMAND) & ~I915_PCI_INTXDISABLE);
    pic_setroutine(pci_getirq(id), handleirq);
    apic_setroutine(pci_getirq(id), handleirq);
    writereg(I915_GFX_MSTR_IRQ, I915_MSTR_ENABLE);

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
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: firmware %ux%u", &pipea.activewidth, &pipea.activeheight);

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

    return (source >> 16) + 1 == framebuffer->width && (source & 0xFFFF) + 1 == framebuffer->height && pipea.activewidth == framebuffer->width && pipea.activeheight == framebuffer->height;

}

static void videointerface_setmode(unsigned int width, unsigned int height, unsigned int bpp)
{

    if (mmio && bpp == 4 && width >= I915_MINSIZE && height >= I915_MINSIZE && width <= pipea.activewidth && height <= pipea.activeheight && !(width % 16))
    {

        unsigned int surface = readreg(I915_PLANE_SURF_1_A);

        setsource(&pipea, width, height, surface);

        videointerface.width = width;
        videointerface.height = height;

    }

    arch_kmap(framebuffer->address, 0xA0000000, framebuffer->pitch * framebuffer->height, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);

}

static void secondinterface_setmode(unsigned int width, unsigned int height, unsigned int bpp)
{

    if (secondsize && bpp == 4 && width >= I915_MINSIZE && height >= I915_MINSIZE && width * height * 4 <= secondsize && !(width % 16))
    {

        secondinterface.width = width;
        secondinterface.height = height;

        if (pipeb.on)
            setsource(&pipeb, width, height, secondsurface);

    }

}

static unsigned long readpte(unsigned long gsm, unsigned int index)
{

    unsigned long low = *(volatile unsigned int *)(gsm + index * 8);
    unsigned long high = *(volatile unsigned int *)(gsm + index * 8 + 4);

    return (sizeof (unsigned long) > 4) ? low | ((high << 16) << 16) : low;

}

static void writepte(unsigned long gsm, unsigned int index, unsigned long address)
{

    *(volatile unsigned int *)(gsm + index * 8 + 4) = (sizeof (unsigned long) > 4) ? (address >> 16) >> 16 : 0;
    *(volatile unsigned int *)(gsm + index * 8) = (address & 0xFFFFFFFF) | I915_PTE_PRESENT;

}

static unsigned long stolenbase(unsigned int id)
{

    unsigned long base = pci_ind(id, I915_BDSM) & I915_BDSM_MASK;

    if (sizeof (unsigned long) > 4)
        base |= ((unsigned long)pci_ind(id, I915_BDSM + 4) << 16) << 16;

    return (base) ? base : pci_ind(id, I915_BDSMLEGACY) & I915_BDSM_MASK;

}

static unsigned long stolensize(unsigned int id)
{

    unsigned int gms = (pci_inw(id, I915_GMCH_CTRL) >> 8) & 0xFF;

    return (gms < 0xF0) ? gms * 0x2000000UL : (gms - 0xF0 + 1) * 0x400000UL;

}

static void allocatesecond(unsigned int id)
{

    unsigned long gsm = mmio + I915_GSM;
    unsigned int first = readreg(I915_PLANE_SURF_1_A) & ~(I915_PAGESIZE - 1);
    unsigned long table = (gsm + (first / I915_PAGESIZE) * 8) & ~(unsigned long)(I915_PAGESIZE - 1);
    unsigned int fbsize = (framebuffer->pitch * framebuffer->height + I915_SECONDALIGN - 1) & ~(I915_SECONDALIGN - 1);
    unsigned long base = stolenbase(id);
    unsigned long end = base + stolensize(id) - I915_STOLENRESERVE;
    unsigned long physical;
    unsigned int pages;
    unsigned int i;

    arch_kmap(table, table, ((fbsize + I915_SECONDLARGE) / I915_PAGESIZE) * 8 + I915_PAGESIZE * 2, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    physical = (readpte(gsm, first / I915_PAGESIZE) & ~(unsigned long)(I915_PAGESIZE - 1)) + fbsize;

    if (!base || physical < base + fbsize)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: firmware framebuffer not in stolen memory, no video:1");

        return;

    }

    secondsize = (physical + I915_SECONDLARGE <= end) ? I915_SECONDLARGE : (physical + I915_SECONDSMALL <= end) ? I915_SECONDSMALL : 0;

    if (!secondsize)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i915: no stolen memory left, no video:1");

        return;

    }

    secondsurface = first + fbsize;
    pages = secondsize / I915_PAGESIZE;

    for (i = 0; i < pages; i++)
        writepte(gsm, secondsurface / I915_PAGESIZE + i, physical + i * I915_PAGESIZE);

    readpte(gsm, secondsurface / I915_PAGESIZE + pages - 1);
    writereg(I915_GFX_FLSH_CNTL, 1);
    arch_kmap(pci_getbar(id, 2) + secondsurface, I915_SECONDMAP, secondsize, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);
    buffer_clear((void *)I915_SECONDMAP, secondsize);

    {

        unsigned int megabytes = secondsize >> 20;

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i915: video:1 framebuffer at GGTT 0x%H8u, %u MB", &secondsurface, &megabytes);

    }

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, 0xA0000000, 0, videointerface_setmode);

    videointerface.width = framebuffer->width;
    videointerface.height = framebuffer->height;
    videointerface.bpp = 4;

    video_initinterface(&secondinterface, I915_SECONDMAP, 0, secondinterface_setmode);

    secondinterface.width = I915_SECONDWIDTH;
    secondinterface.height = I915_SECONDHEIGHT;
    secondinterface.bpp = 4;

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

    pipea.activewidth = (readreg(I915_TRANS_HTOTAL_A) & 0xFFFF) + 1;
    pipea.activeheight = (readreg(I915_TRANS_VTOTAL_A) & 0xFFFF) + 1;

    if (checkfirmware())
    {

        probepanel();

        if (hdmi.found && hdmi.port < I915_COMBOPORTS)
            allocatesecond(id);

        if (hdmi.connected && secondsize)
            enablehdmi();

    }

    else
    {

        mmio = 0;

    }

}

static void driver_attach(unsigned int id)
{

    video_registerinterface(&videointerface);

    if (secondsize)
    {

        video_registerinterface(&secondinterface);
        enablehotplug(id);

    }

}

static void driver_detach(unsigned int id)
{

    video_unregisterinterface(&videointerface);

    if (secondsize)
    {

        video_unregisterinterface(&secondinterface);
        writereg(I915_GFX_MSTR_IRQ, 0);
        pic_unsetroutine(pci_getirq(id), handleirq);
        apic_unsetroutine(pci_getirq(id), handleirq);

    }

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

