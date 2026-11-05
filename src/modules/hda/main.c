#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/audio.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>
#include <modules/pci/pci.h>

#define VENDOR_INTEL                    0x8086
#define DEVICE_TGLLP                    0xA0C8
#define DEVICE_TGLH                     0x43C8
#define SUBCLASS_HDA                    0x03
#define PCI_TCSEL                       0x44
#define PCI_CGCTL                       0x48
#define PCI_CGCTL_MISCBDCGE             (1 << 6)
#define PCI_DEVC                        0x78
#define PCI_DEVC_NOSNOOP                (1 << 11)
#define MMIOSIZE                        0x4000
#define MMIOMAP                         0xB8000000
#define REG_GCAP                        0x00
#define REG_GCAP_ISSSHIFT               8
#define REG_GCAP_OSSSHIFT               12
#define REG_GCTL                        0x08
#define REG_GCTL_CRST                   (1 << 0)
#define REG_STATESTS                    0x0E
#define REG_INTCTL                      0x20
#define REG_INTCTL_GIE                  0x80000000
#define REG_CORBLBASE                   0x40
#define REG_CORBUBASE                   0x44
#define REG_CORBWP                      0x48
#define REG_CORBRP                      0x4A
#define REG_CORBRP_RST                  (1 << 15)
#define REG_CORBCTL                     0x4C
#define REG_CORBSIZE                    0x4E
#define REG_RIRBLBASE                   0x50
#define REG_RIRBUBASE                   0x54
#define REG_RIRBWP                      0x58
#define REG_RIRBWP_RST                  (1 << 15)
#define REG_RINTCNT                     0x5A
#define REG_RIRBCTL                     0x5C
#define REG_RIRBCTL_RINTCTL             (1 << 0)
#define REG_RIRBSTS                     0x5D
#define REG_RIRBSTS_CLEAR               0x05
#define REG_RIRBSIZE                    0x5E
#define REG_RINGRUN                     (1 << 1)
#define REG_SIZE256                     0x40
#define REG_SIZE16                      0x20
#define REG_SD                          0x80
#define REG_SDSIZE                      0x20
#define SD_CTL                          0x00
#define SD_CTL_SRST                     (1 << 0)
#define SD_CTL_RUN                      (1 << 1)
#define SD_CTL_IOCE                     (1 << 2)
#define SD_CTL_TAGSHIFT                 20
#define SD_STS                          0x03
#define SD_STS_CLEAR                    0x1C
#define SD_LPIB                         0x04
#define SD_CBL                          0x08
#define SD_LVI                          0x0C
#define SD_FMT                          0x12
#define SD_BDPL                         0x18
#define SD_BDPU                         0x1C
#define VERB_GETPARAMETER               0xF0000
#define VERB_GETCONNECTIONS             0xF0200
#define VERB_GETCONFIG                  0xF1C00
#define VERB_SETCONNECTION              0x70100
#define VERB_SETPOWER                   0x70500
#define VERB_SETSTREAM                  0x70600
#define VERB_SETPINCONTROL              0x70700
#define VERB_SETEAPD                    0x70C00
#define VERB_SETFORMAT                  0x20000
#define VERB_SETAMP                     0x30000
#define PARAM_VENDOR                    0x00
#define PARAM_SUBNODES                  0x04
#define PARAM_FUNCTION                  0x05
#define PARAM_WIDGETCAPS                0x09
#define PARAM_PCMCAPS                   0x0A
#define PARAM_PINCAPS                   0x0C
#define PARAM_INAMPCAPS                 0x0D
#define PARAM_CONNECTIONS               0x0E
#define PARAM_OUTAMPCAPS                0x12
#define FUNCTION_AUDIO                  0x01
#define WIDGETCAPS_TYPESHIFT            20
#define WIDGETCAPS_INAMP                (1 << 1)
#define WIDGETCAPS_OUTAMP               (1 << 2)
#define WIDGETCAPS_AMPOVERRIDE          (1 << 3)
#define WIDGETCAPS_DIGITAL              (1 << 9)
#define WIDGET_OUTPUT                   0x0
#define WIDGET_MIXER                    0x2
#define WIDGET_SELECTOR                 0x3
#define WIDGET_PIN                      0x4
#define PINCAPS_OUTPUT                  (1 << 4)
#define PINCAPS_EAPD                    (1 << 16)
#define CONFIG_CONNECTIVITYSHIFT        30
#define CONFIG_CONNECTIVITY_NONE        0x1
#define CONFIG_DEVICESHIFT              20
#define CONFIG_DEVICE_LINEOUT           0x0
#define CONFIG_DEVICE_SPEAKER           0x1
#define CONFIG_DEVICE_HEADPHONE         0x2
#define CONNECTIONS_LONG                (1 << 7)
#define CONNECTIONS_LENGTH              0x7F
#define PINCONTROL_OUT                  0x40
#define PINCONTROL_HEADPHONE            0x80
#define EAPD_ENABLE                     0x02
#define AMP_OUT                         0xB000
#define AMP_IN                          0x7000
#define AMP_INDEXSHIFT                  8
#define AMPCAPS_OFFSET                  0x7F
#define PCMCAPS_44100                   (1 << 5)
#define PCMCAPS_48000                   (1 << 6)
#define PCMCAPS_16BIT                   (1 << 17)
#define FORMAT_44100                    0x4011
#define FORMAT_48000                    0x0011
#define RIRB_UNSOLICITED                (1 << 4)
#define CODECS                          15
#define DEPTH                           5
#define OUTPUTS                         8
#define STREAMTAG                       1
#define CORBENTRIES                     256
#define CHUNKS                          16
#define CHUNKSIZE                       MESSAGE_SIZE
#define RINGSIZE                        (CHUNKS * CHUNKSIZE)
#define GAP                             1024
#define IDLE                            (RINGSIZE / 2)
#define ALIGN                           128
#define TIMEOUT                         100000

struct bdlentry
{

    unsigned int low;
    unsigned int high;
    unsigned int length;
    unsigned int flags;

};

struct dma
{

    volatile unsigned int corb[CORBENTRIES];
    volatile unsigned int rirb[CORBENTRIES * 2];
    volatile struct bdlentry bdl[CHUNKS];
    unsigned char ring[RINGSIZE];

};

struct link
{

    unsigned int corbentries;
    unsigned int rirbentries;
    unsigned int corbwp;
    unsigned int rirbrp;

};

struct codec
{

    unsigned int address;
    unsigned int afg;
    unsigned int ampout;
    unsigned int ampin;
    unsigned int pcmcaps;
    unsigned int dacs[OUTPUTS];
    unsigned int ndacs;

};

struct stream
{

    unsigned int base;
    unsigned int index;
    unsigned int format;
    unsigned int running;
    unsigned int position;
    unsigned int tail;
    unsigned int queued;
    unsigned int idle;

};

static struct base_driver driver;
static struct audio_interface audiointerface;
static struct spinlock spinlock;
static unsigned long mmio;
static unsigned int irq;
static unsigned char memory[sizeof (struct dma) + ALIGN];
static struct dma *dma;
static struct link link;
static struct codec codec;
static struct stream stream;

static unsigned int read8(unsigned int offset)
{

    return *(volatile unsigned char *)(mmio + offset);

}

static unsigned int read16(unsigned int offset)
{

    return *(volatile unsigned short *)(mmio + offset);

}

static unsigned int read32(unsigned int offset)
{

    return *(volatile unsigned int *)(mmio + offset);

}

static void write8(unsigned int offset, unsigned int value)
{

    *(volatile unsigned char *)(mmio + offset) = value;

}

static void write16(unsigned int offset, unsigned int value)
{

    *(volatile unsigned short *)(mmio + offset) = value;

}

static void write32(unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(mmio + offset) = value;

}

static unsigned int waitbits16(unsigned int offset, unsigned int mask, unsigned int value)
{

    unsigned int i;

    for (i = 0; i < TIMEOUT; i++)
    {

        if ((read16(offset) & mask) == value)
            return 1;

    }

    return 0;

}

static unsigned int waitbits32(unsigned int offset, unsigned int mask, unsigned int value)
{

    unsigned int i;

    for (i = 0; i < TIMEOUT; i++)
    {

        if ((read32(offset) & mask) == value)
            return 1;

    }

    return 0;

}

static unsigned int low32(void *address)
{

    return (unsigned long)address;

}

static unsigned int high32(void *address)
{

    return ((unsigned long)address >> 16) >> 16;

}

static unsigned int command(unsigned int address, unsigned int nid, unsigned int verb)
{

    unsigned int i;

    link.corbwp = (link.corbwp + 1) % link.corbentries;
    dma->corb[link.corbwp] = (address << 28) | ((nid & 0xFF) << 20) | verb;

    write16(REG_CORBWP, link.corbwp);

    for (i = 0; i < TIMEOUT; i++)
    {

        unsigned int wp = read16(REG_RIRBWP) % link.rirbentries;

        while (link.rirbrp != wp)
        {

            link.rirbrp = (link.rirbrp + 1) % link.rirbentries;

            write8(REG_RIRBSTS, REG_RIRBSTS_CLEAR);

            if (!(dma->rirb[link.rirbrp * 2 + 1] & RIRB_UNSOLICITED))
                return dma->rirb[link.rirbrp * 2];

        }

    }

    return 0;

}

static unsigned int getparameter(unsigned int nid, unsigned int parameter)
{

    return command(codec.address, nid, VERB_GETPARAMETER | parameter);

}

static unsigned int getconnection(unsigned int nid, unsigned int connections, unsigned int index)
{

    if (connections & CONNECTIONS_LONG)
        return (command(codec.address, nid, VERB_GETCONNECTIONS | (index & ~1u)) >> ((index & 1) * 16)) & 0x7FFF;
    else
        return (command(codec.address, nid, VERB_GETCONNECTIONS | (index & ~3u)) >> ((index & 3) * 8)) & 0x7F;

}

static unsigned int getampoffset(unsigned int nid, unsigned int caps, unsigned int parameter, unsigned int fallback)
{

    return ((caps & WIDGETCAPS_AMPOVERRIDE) ? getparameter(nid, parameter) : fallback) & AMPCAPS_OFFSET;

}

static void unmute(unsigned int nid, unsigned int caps, unsigned int index)
{

    if (caps & WIDGETCAPS_OUTAMP)
        command(codec.address, nid, VERB_SETAMP | AMP_OUT | getampoffset(nid, caps, PARAM_OUTAMPCAPS, codec.ampout));

    if (caps & WIDGETCAPS_INAMP)
        command(codec.address, nid, VERB_SETAMP | AMP_IN | (index << AMP_INDEXSHIFT) | getampoffset(nid, caps, PARAM_INAMPCAPS, codec.ampin));

}

static void adddac(unsigned int nid)
{

    unsigned int pcmcaps = getparameter(nid, PARAM_PCMCAPS);
    unsigned int i;

    for (i = 0; i < codec.ndacs; i++)
    {

        if (codec.dacs[i] == nid)
            return;

    }

    if (codec.ndacs == OUTPUTS)
        return;

    codec.dacs[codec.ndacs] = nid;
    codec.ndacs++;
    codec.pcmcaps &= (pcmcaps) ? pcmcaps : getparameter(codec.afg, PARAM_PCMCAPS);

}

static unsigned int route(unsigned int nid, unsigned int depth)
{

    unsigned int caps = getparameter(nid, PARAM_WIDGETCAPS);
    unsigned int type = (caps >> WIDGETCAPS_TYPESHIFT) & 0xF;
    unsigned int connections;
    unsigned int i;

    if (type == WIDGET_OUTPUT)
    {

        if (caps & WIDGETCAPS_DIGITAL)
            return 0;

        command(codec.address, nid, VERB_SETPOWER);
        unmute(nid, caps, 0);

        return nid;

    }

    if (depth == DEPTH || (type != WIDGET_PIN && type != WIDGET_MIXER && type != WIDGET_SELECTOR))
        return 0;

    connections = getparameter(nid, PARAM_CONNECTIONS);

    for (i = 0; i < (connections & CONNECTIONS_LENGTH); i++)
    {

        unsigned int dac = route(getconnection(nid, connections, i), depth + 1);

        if (dac)
        {

            command(codec.address, nid, VERB_SETPOWER);

            /* a mixer sums its inputs, the others pick one; a pin's input amp is for recording */
            if (type == WIDGET_MIXER)
                unmute(nid, caps, i);
            else
                unmute(nid, caps & ~WIDGETCAPS_INAMP, 0);

            if (type != WIDGET_MIXER && (connections & CONNECTIONS_LENGTH) > 1)
                command(codec.address, nid, VERB_SETCONNECTION | i);

            return dac;

        }

    }

    return 0;

}

static void setuppin(unsigned int nid)
{

    unsigned int caps = getparameter(nid, PARAM_WIDGETCAPS);
    unsigned int pincaps;
    unsigned int config;
    unsigned int device;
    unsigned int dac;

    if (((caps >> WIDGETCAPS_TYPESHIFT) & 0xF) != WIDGET_PIN || (caps & WIDGETCAPS_DIGITAL))
        return;

    pincaps = getparameter(nid, PARAM_PINCAPS);

    if (!(pincaps & PINCAPS_OUTPUT))
        return;

    config = command(codec.address, nid, VERB_GETCONFIG);
    device = (config >> CONFIG_DEVICESHIFT) & 0xF;

    if ((config >> CONFIG_CONNECTIVITYSHIFT) == CONFIG_CONNECTIVITY_NONE)
        return;

    if (device != CONFIG_DEVICE_LINEOUT && device != CONFIG_DEVICE_SPEAKER && device != CONFIG_DEVICE_HEADPHONE)
        return;

    dac = route(nid, 0);

    if (!dac)
        return;

    command(codec.address, nid, VERB_SETPINCONTROL | PINCONTROL_OUT | ((device == CONFIG_DEVICE_HEADPHONE) ? PINCONTROL_HEADPHONE : 0));

    if (pincaps & PINCAPS_EAPD)
        command(codec.address, nid, VERB_SETEAPD | EAPD_ENABLE);

    adddac(dac);
    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "hda: codec %u pin 0x%H2u device %u through dac 0x%H2u", &codec.address, &nid, &device, &dac);

}

static unsigned int setupcodec(unsigned int address)
{

    unsigned int vendor;
    unsigned int subnodes;
    unsigned int nid;

    codec.address = address;
    codec.afg = 0;
    codec.ndacs = 0;
    codec.pcmcaps = 0xFFFFFFFF;

    vendor = getparameter(0, PARAM_VENDOR);
    subnodes = getparameter(0, PARAM_SUBNODES);

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "hda: codec %u vendor 0x%H8u", &address, &vendor);

    for (nid = (subnodes >> 16) & 0xFF; nid < ((subnodes >> 16) & 0xFF) + (subnodes & 0xFF); nid++)
    {

        if ((getparameter(nid, PARAM_FUNCTION) & 0xFF) == FUNCTION_AUDIO)
        {

            codec.afg = nid;

            break;

        }

    }

    if (!codec.afg)
        return 0;

    command(codec.address, codec.afg, VERB_SETPOWER);

    codec.ampout = getparameter(codec.afg, PARAM_OUTAMPCAPS);
    codec.ampin = getparameter(codec.afg, PARAM_INAMPCAPS);
    subnodes = getparameter(codec.afg, PARAM_SUBNODES);

    for (nid = (subnodes >> 16) & 0xFF; nid < ((subnodes >> 16) & 0xFF) + (subnodes & 0xFF); nid++)
        setuppin(nid);

    return codec.ndacs && (codec.pcmcaps & PCMCAPS_16BIT) && (codec.pcmcaps & (PCMCAPS_44100 | PCMCAPS_48000));

}

static void setdacs(void)
{

    unsigned int i;

    for (i = 0; i < codec.ndacs; i++)
    {

        command(codec.address, codec.dacs[i], VERB_SETFORMAT | stream.format);
        command(codec.address, codec.dacs[i], VERB_SETSTREAM | (STREAMTAG << 4));

    }

}

static unsigned int resetcontroller(void)
{

    unsigned int codecs = 0;
    unsigned int i;

    write8(REG_CORBCTL, 0);
    write8(REG_RIRBCTL, 0);
    write32(REG_INTCTL, 0);
    write32(REG_GCTL, read32(REG_GCTL) & ~REG_GCTL_CRST);

    if (!waitbits32(REG_GCTL, REG_GCTL_CRST, 0))
        return 0;

    write32(REG_GCTL, read32(REG_GCTL) | REG_GCTL_CRST);

    if (!waitbits32(REG_GCTL, REG_GCTL_CRST, REG_GCTL_CRST))
        return 0;

    /* codecs announce themselves within 521 us of the reset, give slow ones the same time again */
    for (i = 0; i < TIMEOUT && !codecs; i++)
        codecs = read16(REG_STATESTS);

    for (i = 0; i < TIMEOUT / 10; i++)
        codecs |= read16(REG_STATESTS);

    write16(REG_STATESTS, codecs);

    return codecs;

}

static unsigned int getringsize(unsigned int size)
{

    if (size & REG_SIZE256)
        return 256;

    if (size & REG_SIZE16)
        return 16;

    return 2;

}

static unsigned int getringselect(unsigned int entries)
{

    return (entries == 256) ? 2 : (entries == 16) ? 1 : 0;

}

static void setuplink(void)
{

    link.corbentries = getringsize(read8(REG_CORBSIZE));
    link.rirbentries = getringsize(read8(REG_RIRBSIZE));
    link.corbwp = 0;
    link.rirbrp = 0;

    write8(REG_CORBSIZE, getringselect(link.corbentries));
    write32(REG_CORBLBASE, low32((void *)dma->corb));
    write32(REG_CORBUBASE, high32((void *)dma->corb));
    write16(REG_CORBRP, REG_CORBRP_RST);
    waitbits16(REG_CORBRP, REG_CORBRP_RST, REG_CORBRP_RST);
    write16(REG_CORBRP, 0);
    waitbits16(REG_CORBRP, REG_CORBRP_RST, 0);
    write16(REG_CORBWP, 0);
    write8(REG_RIRBSIZE, getringselect(link.rirbentries));
    write32(REG_RIRBLBASE, low32((void *)dma->rirb));
    write32(REG_RIRBUBASE, high32((void *)dma->rirb));
    write16(REG_RIRBWP, REG_RIRBWP_RST);
    write16(REG_RINTCNT, 1);
    write8(REG_CORBCTL, REG_RINGRUN);
    /* without the response interrupt status, qemu stops after RINTCNT responses; INTCTL keeps it from the cpu */
    write8(REG_RIRBCTL, REG_RINGRUN | REG_RIRBCTL_RINTCTL);

}

static void stopstream(void)
{

    write32(stream.base + SD_CTL, read32(stream.base + SD_CTL) & 0x00FFFFFF & ~(SD_CTL_RUN | SD_CTL_IOCE));
    waitbits32(stream.base + SD_CTL, SD_CTL_RUN, 0);
    write8(stream.base + SD_STS, SD_STS_CLEAR);

    /* the next start begins at the top of the ring, and anything played after the queued data must be silence */
    if (stream.running)
        buffer_clear(dma->ring, RINGSIZE);

    stream.running = 0;
    stream.position = 0;
    stream.tail = 0;
    stream.queued = 0;
    stream.idle = 0;

}

static void startstream(void)
{

    write32(stream.base + SD_CTL, SD_CTL_SRST);
    waitbits32(stream.base + SD_CTL, SD_CTL_SRST, SD_CTL_SRST);
    write32(stream.base + SD_CTL, 0);
    waitbits32(stream.base + SD_CTL, SD_CTL_SRST, 0);
    write32(stream.base + SD_CBL, RINGSIZE);
    write16(stream.base + SD_LVI, CHUNKS - 1);
    write16(stream.base + SD_FMT, stream.format);
    write32(stream.base + SD_BDPL, low32((void *)dma->bdl));
    write32(stream.base + SD_BDPU, high32((void *)dma->bdl));
    write8(stream.base + SD_STS, SD_STS_CLEAR);
    write32(stream.base + SD_CTL, (STREAMTAG << SD_CTL_TAGSHIFT) | SD_CTL_RUN | SD_CTL_IOCE);

    stream.running = 1;

}

static void update(void)
{

    unsigned int position;
    unsigned int played;

    if (!stream.running)
        return;

    position = read32(stream.base + SD_LPIB) % RINGSIZE;
    played = (position + RINGSIZE - stream.position) % RINGSIZE;

    /* silence what was played, so the ring never comes round to old sound */
    if (stream.position + played <= RINGSIZE)
    {

        buffer_clear(dma->ring + stream.position, played);

    }

    else
    {

        buffer_clear(dma->ring + stream.position, RINGSIZE - stream.position);
        buffer_clear(dma->ring, played - (RINGSIZE - stream.position));

    }

    stream.position = position;

    if (played < stream.queued)
    {

        stream.queued -= played;

        return;

    }

    /* play some silence before stopping, a stop drops what the codec still holds */
    stream.idle += played - stream.queued;
    stream.queued = 0;

    if (stream.idle >= IDLE)
        stopstream();

}

static void handleirq(unsigned int line)
{

    unsigned int status;

    if (!mmio || !stream.base)
        return;

    status = read8(stream.base + SD_STS);

    if (!(status & SD_STS_CLEAR))
        return;

    write8(stream.base + SD_STS, status);
    spinlock_acquire(&spinlock);
    update();
    spinlock_release(&spinlock);

}

static unsigned int audiointerface_setformat(unsigned int rate, unsigned int channels, unsigned int bits)
{

    unsigned int format;

    if (channels != 2 || bits != 16)
        return 0;

    if (rate == 48000 && (codec.pcmcaps & PCMCAPS_48000))
        format = FORMAT_48000;
    else if (rate == 44100 && (codec.pcmcaps & PCMCAPS_44100))
        format = FORMAT_44100;
    else
        return 0;

    spinlock_acquire(&spinlock);
    stopstream();

    stream.format = format;

    setdacs();

    audiointerface.rate = rate;

    spinlock_release(&spinlock);

    return 1;

}

static unsigned int audiointerface_write(void *buffer, unsigned int count)
{

    unsigned int first;

    /* whole frames only: two channels of 16 bits */
    count &= ~3;

    if (!count)
        return 1;

    spinlock_acquire(&spinlock);
    update();

    /* the ring ran dry: continue a little ahead of the engine, which has fetched past the position */
    if (stream.running && !stream.queued)
    {

        stream.tail = (stream.position + GAP) % RINGSIZE;
        stream.queued = GAP;

    }

    if (stream.queued + count > RINGSIZE - CHUNKSIZE)
    {

        spinlock_release(&spinlock);

        return 0;

    }

    first = (stream.tail + count <= RINGSIZE) ? count : RINGSIZE - stream.tail;

    buffer_copy(dma->ring + stream.tail, buffer, first);
    buffer_copy(dma->ring, (unsigned char *)buffer + first, count - first);

    stream.tail = (stream.tail + count) % RINGSIZE;
    stream.queued += count;
    stream.idle = 0;

    if (!stream.running)
        startstream();

    spinlock_release(&spinlock);

    return 1;

}

static unsigned int istigerlake(unsigned int id)
{

    unsigned short device = pci_inw(id, PCI_CONFIG_DEVICE);

    return pci_inw(id, PCI_CONFIG_VENDOR) == VENDOR_INTEL && (device == DEVICE_TGLLP || device == DEVICE_TGLH);

}

static unsigned long mapregisters(unsigned long address)
{

    /* tasks only share the kernel's mappings below 4 GB */
    if (high32((void *)address))
    {

        arch_kmap(address, MMIOMAP, MMIOSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

        return MMIOMAP;

    }

    arch_kmap(address, address, MMIOSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    return address;

}

static unsigned int setup(unsigned int id)
{

    unsigned int codecs;
    unsigned int cgctl = pci_ind(id, PCI_CGCTL);
    unsigned int i;

    if (pci_inw(id, PCI_CONFIG_VENDOR) == VENDOR_INTEL)
    {

        pci_outb(id, PCI_TCSEL, pci_inb(id, PCI_TCSEL) & ~0x07);
        pci_outw(id, PCI_DEVC, pci_inw(id, PCI_DEVC) & ~PCI_DEVC_NOSNOOP);

    }

    /* Skylake and later lose codecs in the reset with this clock gating on */
    if (istigerlake(id))
        pci_outd(id, PCI_CGCTL, cgctl & ~PCI_CGCTL_MISCBDCGE);

    codecs = resetcontroller();

    if (istigerlake(id))
        pci_outd(id, PCI_CGCTL, cgctl);

    if (!codecs)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "hda: no codecs");

        return 0;

    }

    setuplink();

    for (i = 0; i < CODECS; i++)
    {

        if ((codecs & (1 << i)) && setupcodec(i))
            break;

    }

    if (i == CODECS)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "hda: no analog output");

        return 0;

    }

    if (!((read16(REG_GCAP) >> REG_GCAP_OSSSHIFT) & 0xF))
        return 0;

    stream.index = (read16(REG_GCAP) >> REG_GCAP_ISSSHIFT) & 0xF;
    stream.base = REG_SD + stream.index * REG_SDSIZE;
    stream.format = (codec.pcmcaps & PCMCAPS_48000) ? FORMAT_48000 : FORMAT_44100;
    audiointerface.rate = (codec.pcmcaps & PCMCAPS_48000) ? 48000 : 44100;

    stopstream();
    setdacs();

    return 1;

}

static void driver_init(unsigned int id)
{

    unsigned int i;

    /* the device wants its rings and buffer list on 128 byte boundaries, and the module loader doesn't keep the alignment of static data */
    dma = (struct dma *)(((unsigned long)memory + ALIGN - 1) & ~(unsigned long)(ALIGN - 1));

    for (i = 0; i < CHUNKS; i++)
    {

        dma->bdl[i].low = low32(dma->ring + i * CHUNKSIZE);
        dma->bdl[i].high = high32(dma->ring + i * CHUNKSIZE);
        dma->bdl[i].length = CHUNKSIZE;
        dma->bdl[i].flags = 1;

    }

    audio_initinterface(&audiointerface, audiointerface_setformat, audiointerface_write);

    audiointerface.rate = 0;
    audiointerface.channels = 2;
    audiointerface.bits = 16;

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    if (type != PCI_BUS || pci_inb(id, PCI_CONFIG_CLASS) != PCI_CLASS_MEDIA)
        return 0;

    /* Tiger Lake says audio rather than HD audio when its DSP is enabled, the HD audio registers still work */
    return pci_inb(id, PCI_CONFIG_SUBCLASS) == SUBCLASS_HDA || istigerlake(id);

}

static void driver_reset(unsigned int id)
{

    unsigned long address = pci_getbar(id, 0);

    if (!address)
        return;

    mmio = mapregisters(address);

    pci_setmaster(id);

    if (!setup(id))
    {

        audiointerface.rate = 0;

        return;

    }

    irq = pci_getirq(id);

    pci_outw(id, PCI_CONFIG_COMMAND, pci_inw(id, PCI_CONFIG_COMMAND) & ~PCI_COMMAND_INTXDISABLE);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);
    write32(REG_INTCTL, REG_INTCTL_GIE | (1 << stream.index));

}

static void driver_attach(unsigned int id)
{

    if (audiointerface.rate)
        audio_registerinterface(&audiointerface);

}

static void driver_detach(unsigned int id)
{

    if (audiointerface.rate)
    {

        audio_unregisterinterface(&audiointerface);
        write32(REG_INTCTL, 0);
        pic_unsetroutine(irq, handleirq);
        apic_unsetroutine(irq, handleirq);
        spinlock_acquire(&spinlock);
        stopstream();
        spinlock_release(&spinlock);
        write8(REG_CORBCTL, 0);
        write8(REG_RIRBCTL, 0);

    }

}

void module_init(void)
{

    base_initdriver(&driver, "hda", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

