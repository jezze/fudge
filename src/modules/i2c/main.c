#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/acpi/acpi.h>
#include <modules/pci/pci.h>

#define VENDOR_INTEL                    0x8086
#define CONTROLLERS                     6
#define TABLES                          32
#define HIDREGISTERS                    2
#define MMIOSIZE                        0x1000
#define MMIOMAP                         0xB8008000
#define TIMEOUT                         1000000
#define POWERDELAY                      10000
#define RXFIFO                          8
#define TRANSFER_OUT                    4
#define TRANSFER_IN                     32
#define PCI_CAPABILITY_PM               0x01
#define PCI_PMCSR                       0x04
#define PCI_PMCSR_STATE                 0x03
#define PCI_COMMAND_MEMORY              0x0002
#define LPSS_RESETS                     0x204
#define LPSS_RESETS_DONE                0x07
#define LPSS_REMAPLOW                   0x240
#define LPSS_REMAPHIGH                  0x244
#define LPSS_CAPS                       0x2FC
#define REG_CON                         0x00
#define REG_CON_MASTER                  (1 << 0)
#define REG_CON_STANDARD                (1 << 1)
#define REG_CON_RESTART                 (1 << 5)
#define REG_CON_SLAVEDISABLE            (1 << 6)
#define REG_TAR                         0x04
#define REG_DATACMD                     0x10
#define REG_DATACMD_READ                (1 << 8)
#define REG_DATACMD_STOP                (1 << 9)
#define REG_DATACMD_RESTART             (1 << 10)
#define REG_SSHCNT                      0x14
#define REG_SSLCNT                      0x18
#define REG_INTRMASK                    0x30
#define REG_RAWINTRSTAT                 0x34
#define REG_RAWINTRSTAT_TXABRT          (1 << 6)
#define REG_RAWINTRSTAT_STOPDET         (1 << 9)
#define REG_RXTL                        0x38
#define REG_TXTL                        0x3C
#define REG_CLRINTR                     0x40
#define REG_ENABLE                      0x6C
#define REG_STATUS                      0x70
#define REG_STATUS_TFNF                 (1 << 1)
#define REG_STATUS_RFNE                 (1 << 3)
#define REG_SDAHOLD                     0x7C
#define REG_TXABRTSOURCE                0x80
#define REG_ENABLESTATUS                0x9C
#define REG_COMPPARAM1                  0xF4
#define REG_COMPTYPE                    0xFC
#define COMPTYPE_DESIGNWARE             0x44570140
#define SSHCNT                          600
#define SSLCNT                          672
#define SDAHOLD                         64
#define HIDDESC_LENGTH                  30
#define HIDDESC_VERSION                 0x0100
#define AML_DEVICE                      0x82
#define AML_EXTPREFIX                   0x5B
#define AML_DUALNAME                    0x2E
#define AML_STRING                      0x0D
#define RES_I2C                         0x8E
#define RES_GPIO                        0x8C
#define RES_IRQ                         0x89

struct scan
{

    unsigned char *aml;
    unsigned int length;
    char signature[5];

};

struct device
{

    unsigned int start;
    unsigned int end;
    char name[5];

};

struct pkglength
{

    unsigned int length;
    unsigned int bytes;

};

struct touchpad
{

    unsigned int found;
    unsigned int address;
    unsigned int controller;

};

struct transfer
{

    unsigned int address;
    unsigned char out[TRANSFER_OUT];
    unsigned int outcount;
    unsigned char in[TRANSFER_IN];
    unsigned int incount;
    unsigned int abort;

};

static struct base_driver driver;
static struct touchpad touchpad;
static unsigned long mmio;
static unsigned short devicestgllp[CONTROLLERS] = {0xA0E8, 0xA0E9, 0xA0EA, 0xA0EB, 0xA0C5, 0xA0C6};
static unsigned short devicestglh[CONTROLLERS] = {0x43E8, 0x43E9, 0x43EA, 0x43EB, 0x43AD, 0x43AE};
static unsigned short hidregisters[HIDREGISTERS] = {0x0001, 0x0020};

static unsigned int getbyte(struct scan *scan, unsigned int offset)
{

    return (offset < scan->length) ? scan->aml[offset] : 0;

}

static unsigned int getword(struct scan *scan, unsigned int offset)
{

    return getbyte(scan, offset) | (getbyte(scan, offset + 1) << 8);

}

static unsigned int getdword(struct scan *scan, unsigned int offset)
{

    return getword(scan, offset) | (getword(scan, offset + 2) << 16);

}

static unsigned int matches(struct scan *scan, unsigned int offset, char *pattern, unsigned int count)
{

    return offset + count <= scan->length && buffer_match(scan->aml + offset, pattern, count);

}

static unsigned int isstring(struct scan *scan, unsigned int start, unsigned int end)
{

    unsigned int i;

    for (i = start; i < end; i++)
    {

        unsigned int c = getbyte(scan, i);

        if (!c)
            return i > start;

        if (c < 0x20 || c > 0x7E)
            return 0;

    }

    return 0;

}

static char *getstring(struct scan *scan, unsigned int offset)
{

    return (char *)(scan->aml + offset);

}

static unsigned int ishid(struct scan *scan, unsigned int offset)
{

    return matches(scan, offset, "\x0C\x41\xD0\x0C\x50", 5) || matches(scan, offset, "PNP0C50", 7) || matches(scan, offset, "ACPI0C50", 8);

}

static struct pkglength getpkglength(struct scan *scan, unsigned int offset)
{

    struct pkglength pkglength;
    unsigned int lead = getbyte(scan, offset);
    unsigned int i;

    pkglength.bytes = 1 + (lead >> 6);
    pkglength.length = (pkglength.bytes == 1) ? lead & 0x3F : lead & 0x0F;

    for (i = 1; i < pkglength.bytes; i++)
        pkglength.length |= getbyte(scan, offset + i) << (i * 8 - 4);

    return pkglength;

}

static struct device getdevice(struct scan *scan, unsigned int offset)
{

    struct device device;
    unsigned int i;

    device.start = 0;
    device.end = scan->length;

    buffer_copy(device.name, "????", 5);

    for (i = offset; i > 0; i--)
    {

        if (getbyte(scan, i - 1) == AML_EXTPREFIX && getbyte(scan, i) == AML_DEVICE)
        {

            struct pkglength pkglength = getpkglength(scan, i + 1);
            unsigned int end = i + 1 + pkglength.length;

            if (end > offset && end <= scan->length)
            {

                unsigned int name = i + 1 + pkglength.bytes;

                while (getbyte(scan, name) == '\\' || getbyte(scan, name) == '^')
                    name++;

                if (getbyte(scan, name) == AML_DUALNAME)
                    name += 5;

                if (name + 4 <= scan->length)
                    buffer_copy(device.name, scan->aml + name, 4);

                device.start = i - 1;
                device.end = end;

                return device;

            }

        }

    }

    return device;

}

static unsigned int getcontroller(struct scan *scan, unsigned int source)
{

    unsigned int last = source;
    unsigned int i;

    for (i = source; getbyte(scan, i); i++)
    {

        if (getbyte(scan, i) == '.' || getbyte(scan, i) == '\\')
            last = i + 1;

    }

    if (matches(scan, last, "I2C", 3) && getbyte(scan, last + 3) >= '0' && getbyte(scan, last + 3) < '0' + CONTROLLERS)
        return getbyte(scan, last + 3) - '0';

    return CONTROLLERS;

}

static unsigned int scani2c(struct scan *scan, struct device *device, unsigned int offset, unsigned int hid)
{

    unsigned int length = getword(scan, offset + 1);
    unsigned int typelength = getword(scan, offset + 10);
    unsigned int source = offset + 12 + typelength;
    unsigned int end = offset + 3 + length;
    unsigned int address;
    unsigned int speed;

    if (getbyte(scan, offset + 5) != 1 || getbyte(scan, offset + 3) < 1 || getbyte(scan, offset + 3) > 2 || typelength < 6 || end > device->end || source >= end || !isstring(scan, source, end))
        return 0;

    address = getword(scan, offset + 16);
    speed = getdword(scan, offset + 12);

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: %s %s address 0x%H2u speed %u on %s", scan->signature, device->name, &address, &speed, getstring(scan, source));

    if (hid && !touchpad.found)
    {

        touchpad.found = 1;
        touchpad.address = address;
        touchpad.controller = getcontroller(scan, source);

    }

    return 1;

}

static unsigned int scangpio(struct scan *scan, struct device *device, unsigned int offset)
{

    unsigned int length = getword(scan, offset + 1);
    unsigned int end = offset + 3 + length;
    unsigned int pins = offset + getword(scan, offset + 14);
    unsigned int source = offset + getword(scan, offset + 17);
    unsigned int flags = getword(scan, offset + 7);
    unsigned int pin;

    if (getbyte(scan, offset + 4) > 1 || end > device->end || pins < offset + 23 || pins >= source || source >= end || !isstring(scan, source, end))
        return 0;

    pin = getword(scan, pins);

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: %s %s gpio %s pin %u flags 0x%H4u on %s", scan->signature, device->name, getbyte(scan, offset + 4) ? "io" : "interrupt", &pin, &flags, getstring(scan, source));

    return 1;

}

static unsigned int scanirq(struct scan *scan, struct device *device, unsigned int offset)
{

    unsigned int length = getword(scan, offset + 1);
    unsigned int count = getbyte(scan, offset + 4);
    unsigned int flags = getbyte(scan, offset + 3);
    unsigned int irq;

    if (count < 1 || length < 2 + 4 * count || offset + 3 + length > device->end || (flags & 0xE0))
        return 0;

    irq = getdword(scan, offset + 5);

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: %s %s irq %u flags 0x%H2u", scan->signature, device->name, &irq, &flags);

    return 1;

}

static void scanresources(struct scan *scan, struct device *device, unsigned int hid)
{

    unsigned int i;

    for (i = device->start; i < device->end; i++)
    {

        switch (getbyte(scan, i))
        {

        case RES_I2C:
            scani2c(scan, device, i, hid);

            break;

        case RES_GPIO:
            if (hid)
                scangpio(scan, device, i);

            break;

        case RES_IRQ:
            if (hid)
                scanirq(scan, device, i);

            break;

        }

    }

}

static void scanhid(struct scan *scan, struct device *device)
{

    unsigned int i;

    for (i = device->start; i < device->end; i++)
    {

        if (matches(scan, i, "_HID", 4) && getbyte(scan, i + 4) == AML_STRING && isstring(scan, i + 5, device->end))
        {

            debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: %s %s hid %s", scan->signature, device->name, getstring(scan, i + 5));

            return;

        }

    }

}

static unsigned int scantable(struct acpi_sdth *header, unsigned int hidonly)
{

    struct scan scan;
    unsigned int hits = 0;
    unsigned int i;

    scan.aml = (unsigned char *)(header + 1);
    scan.length = header->length - sizeof (struct acpi_sdth);

    buffer_copy(scan.signature, header->signature, 4);

    scan.signature[4] = '\0';

    for (i = 0; i < scan.length; i++)
    {

        if (hidonly && ishid(&scan, i))
        {

            struct device device = getdevice(&scan, i);

            scanhid(&scan, &device);
            scanresources(&scan, &device, 1);

            hits++;
            i = device.end;

        }

        if (!hidonly && getbyte(&scan, i) == RES_I2C)
        {

            struct device device = getdevice(&scan, i);

            hits += scani2c(&scan, &device, i, 0);

        }

    }

    return hits;

}

static unsigned int scantables(unsigned int hidonly)
{

    struct acpi_sdth *dsdt = acpi_findheader("DSDT", 0);
    unsigned int hits = 0;
    unsigned int i;

    if (dsdt)
        hits += scantable(dsdt, hidonly);

    for (i = 0; i < TABLES; i++)
    {

        struct acpi_sdth *ssdt = acpi_findheader("SSDT", i);

        if (!ssdt)
            break;

        hits += scantable(ssdt, hidonly);

    }

    return hits;

}

static unsigned int read32(unsigned int offset)
{

    return *(volatile unsigned int *)(mmio + offset);

}

static void write32(unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(mmio + offset) = value;

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

static unsigned int high32(unsigned long address)
{

    return (address >> 16) >> 16;

}

static unsigned long mapregisters(unsigned long address)
{

    /* tasks only share the kernel's mappings below 4 GB, and Tiger Lake firmware often puts these above */
    if (high32(address))
    {

        arch_kmap(address, MMIOMAP, MMIOSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

        return MMIOMAP;

    }

    arch_kmap(address, address, MMIOSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    return address;

}

static unsigned int getindex(unsigned int id)
{

    unsigned short device = pci_inw(id, PCI_CONFIG_DEVICE);
    unsigned int i;

    if (pci_inw(id, PCI_CONFIG_VENDOR) != VENDOR_INTEL)
        return CONTROLLERS;

    for (i = 0; i < CONTROLLERS; i++)
    {

        if (device == devicestgllp[i] || device == devicestglh[i])
            return i;

    }

    return CONTROLLERS;

}

static void poweron(unsigned int id)
{

    unsigned char pm = pci_findcapability(id, PCI_CAPABILITY_PM);

    pci_outw(id, PCI_CONFIG_COMMAND, pci_inw(id, PCI_CONFIG_COMMAND) | PCI_COMMAND_MEMORY);

    if (pm && (pci_inw(id, pm + PCI_PMCSR) & PCI_PMCSR_STATE))
    {

        unsigned int i;

        pci_outw(id, pm + PCI_PMCSR, pci_inw(id, pm + PCI_PMCSR) & ~PCI_PMCSR_STATE);

        /* D3hot to D0 takes 10 ms, a configuration read takes about a microsecond */
        for (i = 0; i < POWERDELAY; i++)
            pci_inw(id, PCI_CONFIG_VENDOR);

    }

}

static unsigned int disable(void)
{

    write32(REG_ENABLE, 0);

    return waitbits32(REG_ENABLESTATUS, 1, 0);

}

static unsigned int setup(void)
{

    if (!disable())
        return 0;

    /* standard mode, counted for a 120 MHz input clock; a faster clock still stays within fast mode */
    write32(REG_CON, REG_CON_MASTER | REG_CON_STANDARD | REG_CON_RESTART | REG_CON_SLAVEDISABLE);
    write32(REG_SSHCNT, SSHCNT);
    write32(REG_SSLCNT, SSLCNT);
    write32(REG_SDAHOLD, SDAHOLD);
    write32(REG_INTRMASK, 0);
    write32(REG_RXTL, 0);
    write32(REG_TXTL, 0);

    return 1;

}

static unsigned int runtransfer(struct transfer *transfer)
{

    unsigned int sent = 0;
    unsigned int asked = 0;
    unsigned int received = 0;
    unsigned int i;

    if (!disable())
        return 0;

    write32(REG_TAR, transfer->address);
    read32(REG_CLRINTR);
    write32(REG_ENABLE, 1);

    for (i = 0; i < TIMEOUT; i++)
    {

        unsigned int status = read32(REG_STATUS);
        unsigned int raw = read32(REG_RAWINTRSTAT);

        if (raw & REG_RAWINTRSTAT_TXABRT)
        {

            transfer->abort = read32(REG_TXABRTSOURCE);

            read32(REG_CLRINTR);
            disable();

            return 0;

        }

        if (received < transfer->incount && (status & REG_STATUS_RFNE))
        {

            transfer->in[received] = read32(REG_DATACMD);
            received++;

        }

        else if (sent < transfer->outcount && (status & REG_STATUS_TFNF))
        {

            unsigned int command = transfer->out[sent];

            if (sent + 1 == transfer->outcount && !transfer->incount)
                command |= REG_DATACMD_STOP;

            write32(REG_DATACMD, command);

            sent++;

        }

        else if (sent == transfer->outcount && asked < transfer->incount && asked - received < RXFIFO && (status & REG_STATUS_TFNF))
        {

            unsigned int command = REG_DATACMD_READ;

            if (!asked && transfer->outcount)
                command |= REG_DATACMD_RESTART;

            if (asked + 1 == transfer->incount)
                command |= REG_DATACMD_STOP;

            write32(REG_DATACMD, command);

            asked++;

        }

        else if (sent == transfer->outcount && received == transfer->incount && (raw & REG_RAWINTRSTAT_STOPDET))
        {

            disable();

            return 1;

        }

    }

    disable();

    return 0;

}

static unsigned int getinword(struct transfer *transfer, unsigned int offset)
{

    return transfer->in[offset] | (transfer->in[offset + 1] << 8);

}

static unsigned int readhiddescriptor(struct transfer *transfer, unsigned int address, unsigned int reg)
{

    transfer->address = address;
    transfer->out[0] = reg;
    transfer->out[1] = reg >> 8;
    transfer->outcount = 2;
    transfer->incount = HIDDESC_LENGTH;
    transfer->abort = 0;

    return runtransfer(transfer) && getinword(transfer, 0) == HIDDESC_LENGTH && getinword(transfer, 2) == HIDDESC_VERSION;

}

static void probe(unsigned int address)
{

    struct transfer transfer;
    unsigned int i;

    for (i = 0; i < HIDREGISTERS; i++)
    {

        unsigned int reg = hidregisters[i];

        if (readhiddescriptor(&transfer, address, reg))
        {

            unsigned int vendor = getinword(&transfer, 20);
            unsigned int product = getinword(&transfer, 22);
            unsigned int reportlength = getinword(&transfer, 4);
            unsigned int reportregister = getinword(&transfer, 6);
            unsigned int inputregister = getinword(&transfer, 8);
            unsigned int inputlength = getinword(&transfer, 10);
            unsigned int commandregister = getinword(&transfer, 16);

            debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: hid 0x%H2u descriptor at 0x%H4u vendor 0x%H4u product 0x%H4u", &address, &reg, &vendor, &product);
            debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: report descriptor %u bytes at 0x%H4u input 0x%H4u max %u command 0x%H4u", &reportlength, &reportregister, &inputregister, &inputlength, &commandregister);

            return;

        }

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: hid 0x%H2u descriptor at 0x%H4u failed, abort 0x%H8u", &address, &reg, &transfer.abort);

    }

}

static void driver_init(unsigned int id)
{

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    unsigned int index;

    if (type != PCI_BUS)
        return 0;

    index = getindex(id);

    return index < CONTROLLERS && (touchpad.controller == CONTROLLERS || index == touchpad.controller);

}

static void driver_reset(unsigned int id)
{

    unsigned long address = pci_getbar(id, 0);
    unsigned int index = getindex(id);
    unsigned int device = pci_inw(id, PCI_CONFIG_DEVICE);
    unsigned int low = address;
    unsigned int high = high32(address);
    unsigned int comptype;
    unsigned int param;
    unsigned int caps;

    if (!address)
        return;

    poweron(id);

    mmio = mapregisters(address);

    write32(LPSS_RESETS, 0);
    write32(LPSS_RESETS, LPSS_RESETS_DONE);
    write32(LPSS_REMAPLOW, low);
    write32(LPSS_REMAPHIGH, high);

    comptype = read32(REG_COMPTYPE);
    param = read32(REG_COMPPARAM1);
    caps = read32(LPSS_CAPS);

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: controller %u device 0x%H4u at 0x%H8u%H8u type 0x%H8u param 0x%H8u caps 0x%H8u", &index, &device, &high, &low, &comptype, &param, &caps);

    if (comptype != COMPTYPE_DESIGNWARE)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: not a DesignWare controller");

        mmio = 0;

        return;

    }

    if (!setup())
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: controller does not disable");

        mmio = 0;

        return;

    }

    if (touchpad.found)
        probe(touchpad.address);

}

static void driver_attach(unsigned int id)
{

}

static void driver_detach(unsigned int id)
{

    if (mmio)
        disable();

}

void module_init(void)
{

    touchpad.found = 0;
    touchpad.address = 0;
    touchpad.controller = CONTROLLERS;

    if (!scantables(1))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: no PNP0C50 device in the ACPI tables, all I2C devices:");
        scantables(0);

    }

    else if (!touchpad.found)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: PNP0C50 device without a static I2C address");

    }

    base_initdriver(&driver, "i2c", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

