#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/bus.h>
#include <modules/acpi/acpi.h>
#include <modules/pci/pci.h>
#include <modules/cpuid/cpuid.h>
#include "i2c.h"

#define VENDOR_INTEL                    0x8086
#define SUBCLASS_SERIALOTHER            0x80
#define CONTROLLERS                     6
#define TABLES                          32
#define DEVICES                         8
#define MMIOSIZE                        0x1000
#define MMIOMAP                         0xB8008000
#define TIMEOUT                         1000000
#define POWERDELAY                      10000
#define RXFIFO                          8
#define PCI_CAPABILITY_PM               0x01
#define PCI_PMCSR                       0x04
#define PCI_PMCSR_STATE                 0x03
#define PCI_COMMAND_MEMORY              0x0002
#define PCI_HEADERTYPE                  0x0E
#define PCI_HEADERTYPE_MASK             0x7F
#define PCI_HEADERTYPE_BRIDGE           0x01
#define PCI_BARS                        6
#define PCI_BRIDGEBARS                  2
#define PCI_BAR_TYPE                    0x07
#define PCI_BAR_MEMORY64                0x04
#define PCI_BRIDGE_PREFLIMITUPPER       0x2C
#define HOST_TOUUDUPPER                 0xAC
#define HOST_TOUUDUPPER_MASK            0x7F
#define CPUID_EXTENDED                  0x80000000
#define CPUID_ADDRESSSIZE               0x80000008
#define ADDRESSBITS_DEFAULT             36
#define LPSS_RESETS                     0x204
#define LPSS_RESETS_DONE                0x07
#define LPSS_REMAPLOW                   0x240
#define LPSS_REMAPHIGH                  0x244
#define REG_CON                         0x00
#define REG_CON_MASTER                  (1 << 0)
#define REG_CON_STANDARD                (1 << 1)
#define REG_CON_FAST                    (2 << 1)
#define REG_CON_RESTART                 (1 << 5)
#define REG_CON_SLAVEDISABLE            (1 << 6)
#define REG_TAR                         0x04
#define REG_DATACMD                     0x10
#define REG_DATACMD_READ                (1 << 8)
#define REG_DATACMD_STOP                (1 << 9)
#define REG_DATACMD_RESTART             (1 << 10)
#define REG_SSHCNT                      0x14
#define REG_SSLCNT                      0x18
#define REG_FSHCNT                      0x1C
#define REG_FSLCNT                      0x20
#define REG_INTRMASK                    0x30
#define REG_RAWINTRSTAT                 0x34
#define REG_RAWINTRSTAT_TXABRT          (1 << 6)
#define REG_RAWINTRSTAT_STOPDET         (1 << 9)
#define REG_RXTL                        0x38
#define REG_TXTL                        0x3C
#define REG_CLRINTR                     0x40
#define REG_ENABLE                      0x6C
#define REG_ENABLE_ENABLE               (1 << 0)
#define REG_ENABLE_ABORT                (1 << 1)
#define REG_STATUS                      0x70
#define REG_STATUS_TFNF                 (1 << 1)
#define REG_STATUS_RFNE                 (1 << 3)
#define REG_SDAHOLD                     0x7C
#define REG_SDAHOLD_RXSHIFT             16
#define REG_TXABRTSOURCE                0x80
#define REG_ENABLESTATUS                0x9C
#define REG_COMPTYPE                    0xFC
#define COMPTYPE_DESIGNWARE             0x44570140
#define CLOCKKHZ                        120000
#define TSTANDARD_HIGH                  4000
#define TSTANDARD_LOW                   4700
#define TFAST_HIGH                      600
#define TFAST_LOW                       1300
#define TFALL                           300
#define TSDAHOLD                        230
#define AML_DEVICE                      0x82
#define AML_SCOPE                       0x10
#define AML_METHOD                      0x14
#define AML_PACKAGE                     0x12
#define AML_ZERO                        0x00
#define AML_ONE                         0x01
#define AML_BYTEPREFIX                  0x0A
#define AML_WORDPREFIX                  0x0B
#define AML_DWORDPREFIX                 0x0C
#define AML_MULTINAME                   0x2F
#define TOKENS                          3
#define AML_EXTPREFIX                   0x5B
#define AML_DUALNAME                    0x2E
#define AML_STRING                      0x0D
#define RES_I2C                         0x8E

struct scan
{

    unsigned char *aml;
    unsigned int length;

};

struct device
{

    unsigned int start;
    unsigned int end;

};

struct pkglength
{

    unsigned int length;
    unsigned int bytes;

};

struct timing
{

    unsigned int found;
    unsigned int high;
    unsigned int low;
    unsigned int hold;

};

struct mode
{

    unsigned int speed;
    unsigned int hcnt;
    unsigned int lcnt;
    unsigned int high;
    unsigned int low;
    unsigned int hold;

};

struct tokens
{

    unsigned int count;
    unsigned int isname[TOKENS];
    unsigned int offset[TOKENS];
    unsigned int value[TOKENS];

};

struct client
{

    unsigned int address;
    unsigned int controller;
    unsigned int speed;
    unsigned int hid;
    unsigned int hidregister;
    unsigned int haspad;
    unsigned int padgroup;
    unsigned int padnumber;

};

struct controller
{

    unsigned long mmio;
    struct mode mode;
    struct spinlock lock;

};

struct setting
{

    unsigned int controller;
    char *enable;
    unsigned int enablebit;
    char *type;
    char *address;
    char *speed;
    char *hid;
    char *pad;

};

static struct base_bus bus;
static struct client clients[DEVICES];
static unsigned int nclients;

/* Intel's reference firmware has a touchpad (TPD0) under I2C1 and a touch panel (TPL0) under I2C0 whose _INI fills their templates from these; the touchpad comes first so it stays the device the bus drives */
static struct setting settings[] = {
    {1, "SDS1", 0x01, "TPDT", "TPDB", "TPDS", "TPDH", "GPDI"},
    {0, "SDS0", 0x02, "TPLT", "TPLB", "TPLS", "TPLH", "GPLI"}
};
static struct controller controllers[CONTROLLERS];
static struct i2c_transfer check;
static unsigned short devicestgllp[CONTROLLERS] = {0xA0E8, 0xA0E9, 0xA0EA, 0xA0EB, 0xA0C5, 0xA0C6};
static unsigned short devicestglh[CONTROLLERS] = {0x43E8, 0x43E9, 0x43EA, 0x43EB, 0x43AD, 0x43AE};

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

    for (i = offset; i > 0; i--)
    {

        if (getbyte(scan, i - 1) == AML_EXTPREFIX && getbyte(scan, i) == AML_DEVICE)
        {

            struct pkglength pkglength = getpkglength(scan, i + 1);
            unsigned int end = i + 1 + pkglength.length;

            if (end > offset && end <= scan->length)
            {

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

static struct client *addclient(unsigned int address, unsigned int index, unsigned int speed, unsigned int hid)
{

    struct client *client;
    unsigned int i;

    for (i = 0; i < nclients; i++)
    {

        if (clients[i].address == address && clients[i].controller == index)
            return 0;

    }

    if (nclients == DEVICES)
        return 0;

    client = &clients[nclients];
    client->address = address;
    client->controller = index;
    client->speed = speed;
    client->hid = hid;
    client->hidregister = 0;
    client->haspad = 0;
    client->padgroup = 0;
    client->padnumber = 0;
    nclients++;

    return client;

}

static unsigned int getsettingspeed(unsigned int value)
{

    switch (value)
    {

    case 0:
        return 100000;

    case 2:
        return 1000000;

    }

    return 400000;

}

static unsigned int readsettings(void)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < sizeof (settings) / sizeof (struct setting); i++)
    {

        struct setting *setting = &settings[i];
        unsigned int type = acpi_readfield(setting->type).value;
        unsigned int enable = acpi_readfield(setting->enable).value;
        unsigned int address = acpi_readfield(setting->address).value;
        unsigned int speed = getsettingspeed(acpi_readfield(setting->speed).value);
        unsigned int hidregister = acpi_readfield(setting->hid).value;
        struct acpi_field pad = acpi_readfield(setting->pad);
        struct client *client;

        /* what the device's _STA checks */
        if (!type || !(enable & setting->enablebit) || !address)
            continue;

        client = addclient(address, setting->controller, speed, 1);

        if (!client)
            continue;

        client->hidregister = hidregister;
        client->haspad = pad.found;
        client->padgroup = (pad.value >> 16) & 0xFF;
        client->padnumber = pad.value & 0xFFFF;
        count++;

    }

    return count;

}

static void scani2c(struct scan *scan, struct device *device, unsigned int offset)
{

    unsigned int length = getword(scan, offset + 1);
    unsigned int typelength = getword(scan, offset + 10);
    unsigned int source = offset + 12 + typelength;
    unsigned int end = offset + 3 + length;
    unsigned int address;
    unsigned int speed;

    if (getbyte(scan, offset + 5) != 1 || getbyte(scan, offset + 3) < 1 || getbyte(scan, offset + 3) > 2 || typelength < 6 || end > device->end || source >= end || !isstring(scan, source, end))
        return;

    address = getword(scan, offset + 16);
    speed = getdword(scan, offset + 12);

    /* firmware templates leave address 0 (general call) until their methods fill it in */
    if (address)
        addclient(address, getcontroller(scan, source), speed, 1);

}

static void scanresources(struct scan *scan, struct device *device)
{

    unsigned int i;

    for (i = device->start; i < device->end; i++)
    {

        if (getbyte(scan, i) == RES_I2C)
            scani2c(scan, device, i);

    }

}

static unsigned int scantable(struct acpi_sdth *header)
{

    struct scan scan;
    unsigned int hits = 0;
    unsigned int i;

    scan.aml = (unsigned char *)(header + 1);
    scan.length = header->length - sizeof (struct acpi_sdth);

    for (i = 0; i < scan.length; i++)
    {

        if (ishid(&scan, i))
        {

            struct device device = getdevice(&scan, i);

            scanresources(&scan, &device);

            hits++;
            i = device.end;

        }

    }

    return hits;

}

static unsigned int scantables(void)
{

    struct acpi_sdth *dsdt = acpi_findheader("DSDT", 0);
    unsigned int hits = 0;
    unsigned int i;

    if (dsdt)
        hits += scantable(dsdt);

    for (i = 0; i < TABLES; i++)
    {

        struct acpi_sdth *ssdt = acpi_findheader("SSDT", i);

        if (!ssdt)
            break;

        hits += scantable(ssdt);

    }

    return hits;

}

static struct scan getscan(struct acpi_sdth *header)
{

    struct scan scan;

    scan.aml = (unsigned char *)(header + 1);
    scan.length = header->length - sizeof (struct acpi_sdth);

    return scan;

}

static unsigned int isnamechar(unsigned int c, unsigned int lead)
{

    return c == '_' || (c >= 'A' && c <= 'Z') || (!lead && c >= '0' && c <= '9');

}

static unsigned int isnameseg(struct scan *scan, unsigned int offset)
{

    return isnamechar(getbyte(scan, offset), 1) && isnamechar(getbyte(scan, offset + 1), 0) && isnamechar(getbyte(scan, offset + 2), 0) && isnamechar(getbyte(scan, offset + 3), 0);

}

/* the last segment of the NameString at offset, or the table length when there is none */
static unsigned int getlastseg(struct scan *scan, unsigned int offset)
{

    while (getbyte(scan, offset) == '\\' || getbyte(scan, offset) == '^')
        offset++;

    if (getbyte(scan, offset) == AML_DUALNAME)
        offset += 5;

    else if (getbyte(scan, offset) == AML_MULTINAME && getbyte(scan, offset + 1))
        offset += 2 + (getbyte(scan, offset + 1) - 1) * 4;

    return isnameseg(scan, offset) ? offset : scan->length;

}

/* the operands of a short method body like Return (PKG3 (FMH1, FML1, FMD1)) or Return (Package () {0x80, 0xA0, 0x1E}): its last three names or integers */
static struct tokens gettokens(struct scan *scan, unsigned int start, unsigned int end)
{

    struct tokens tokens;
    unsigned int offset = start;

    tokens.count = 0;

    while (offset < end)
    {

        unsigned int b = getbyte(scan, offset);
        unsigned int isname = 0;
        unsigned int value = 0;
        unsigned int size = 0;
        unsigned int i;

        if (b == AML_PACKAGE)
        {

            offset += 1 + getpkglength(scan, offset + 1).bytes + 1;

            continue;

        }

        if (isnameseg(scan, offset))
        {

            isname = 1;
            size = 4;

        }

        else if (b == AML_BYTEPREFIX || b == AML_WORDPREFIX || b == AML_DWORDPREFIX)
        {

            unsigned int bytes = (b == AML_BYTEPREFIX) ? 1 : (b == AML_WORDPREFIX) ? 2 : 4;

            for (i = 0; i < bytes; i++)
                value |= getbyte(scan, offset + 1 + i) << (i * 8);

            size = 1 + bytes;

        }

        else if (b == AML_ZERO || b == AML_ONE)
        {

            value = b;
            size = 1;

        }

        else
        {

            offset++;

            continue;

        }

        if (tokens.count == TOKENS)
        {

            for (i = 1; i < TOKENS; i++)
            {

                tokens.isname[i - 1] = tokens.isname[i];
                tokens.offset[i - 1] = tokens.offset[i];
                tokens.value[i - 1] = tokens.value[i];

            }

            tokens.count--;

        }

        tokens.isname[tokens.count] = isname;
        tokens.offset[tokens.count] = offset;
        tokens.value[tokens.count] = value;
        tokens.count++;

        offset += size;

    }

    return tokens;

}

static unsigned int gettokenvalue(struct scan *scan, struct tokens *tokens, unsigned int index)
{

    char name[5];
    struct acpi_field field;

    if (!tokens->isname[index])
        return tokens->value[index];

    buffer_copy(name, scan->aml + tokens->offset[index], 4);

    name[4] = '\0';
    field = acpi_readfield(name);

    return field.value;

}

/* Method (<method>) inside a Device or Scope whose last name segment is <device> */
static struct timing findtimingtable(struct scan *scan, char *device, char *method)
{

    struct timing timing;
    unsigned int i;

    timing.found = 0;

    for (i = 0; i + 1 < scan->length; i++)
    {

        unsigned int op = (getbyte(scan, i) == AML_EXTPREFIX && getbyte(scan, i + 1) == AML_DEVICE) ? 2 : (getbyte(scan, i) == AML_SCOPE) ? 1 : 0;
        struct pkglength pkglength;
        unsigned int end;
        unsigned int name;
        unsigned int j;

        if (!op)
            continue;

        pkglength = getpkglength(scan, i + op);
        end = i + op + pkglength.length;
        name = getlastseg(scan, i + op + pkglength.bytes);

        if (end > scan->length || name == scan->length || !matches(scan, name, device, 4))
            continue;

        for (j = name + 4; j + 6 < end; j++)
        {

            struct pkglength methodlength;
            unsigned int methodend;
            struct tokens tokens;

            if (getbyte(scan, j) != AML_METHOD)
                continue;

            methodlength = getpkglength(scan, j + 1);
            methodend = j + 1 + methodlength.length;

            if (methodend > end || !matches(scan, j + 1 + methodlength.bytes, method, 4))
                continue;

            tokens = gettokens(scan, j + 1 + methodlength.bytes + 5, methodend);

            if (tokens.count < TOKENS)
                continue;

            timing.found = 1;
            timing.high = gettokenvalue(scan, &tokens, 0);
            timing.low = gettokenvalue(scan, &tokens, 1);
            timing.hold = gettokenvalue(scan, &tokens, 2);

            return timing;

        }

    }

    return timing;

}

static struct timing findtiming(unsigned int controller, char *method)
{

    struct timing timing;
    char device[5];
    unsigned int i;

    buffer_copy(device, "I2C0", 5);

    device[3] = '0' + controller;
    timing.found = 0;

    for (i = 0; i <= TABLES && !timing.found; i++)
    {

        struct acpi_sdth *header = (i) ? acpi_findheader("SSDT", i - 1) : acpi_findheader("DSDT", 0);
        struct scan scan;

        if (!header)
            continue;

        scan = getscan(header);
        timing = findtimingtable(&scan, device, method);

    }

    return timing;

}

static unsigned int read32(struct controller *controller, unsigned int offset)
{

    return *(volatile unsigned int *)(controller->mmio + offset);

}

static void write32(struct controller *controller, unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(controller->mmio + offset) = value;

}

static unsigned int waitbits32(struct controller *controller, unsigned int offset, unsigned int mask, unsigned int value)
{

    unsigned int i;

    for (i = 0; i < TIMEOUT; i++)
    {

        if ((read32(controller, offset) & mask) == value)
            return 1;

    }

    return 0;

}

static unsigned int high32(unsigned long address)
{

    return (address >> 16) >> 16;

}

static unsigned long mapregisters(unsigned long address, unsigned int index)
{

    /* tasks only share the kernel's mappings below 4 GB, and Tiger Lake firmware often puts these above: each controller gets its own page of a fixed window */
    if (high32(address))
    {

        unsigned long window = MMIOMAP + index * MMIOSIZE;

        arch_kmap(address, window, MMIOSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

        return window;

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

static unsigned int getupperlimit(void)
{

    struct cpuid_data data;
    unsigned int bits = ADDRESSBITS_DEFAULT;

    cpuid_getdata(CPUID_EXTENDED, &data);

    if (data.eax >= CPUID_ADDRESSSIZE)
    {

        cpuid_getdata(CPUID_ADDRESSSIZE, &data);

        bits = data.eax & 0xFF;

    }

    return 1 << (bits - 32);

}

static unsigned int max(unsigned int a, unsigned int b)
{

    return (a > b) ? a : b;

}

static unsigned int getupperinuse(void)
{

    unsigned int id = pci_next(0);
    unsigned int upper = 0;

    if (id && pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_BRIDGE && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_BRIDGE_HOST)
        upper = pci_ind(id, HOST_TOUUDUPPER) & HOST_TOUUDUPPER_MASK;

    for (; id; id = pci_next(id))
    {

        unsigned int header = pci_inb(id, PCI_HEADERTYPE) & PCI_HEADERTYPE_MASK;
        unsigned int bars = (header == PCI_HEADERTYPE_BRIDGE) ? PCI_BRIDGEBARS : PCI_BARS;
        unsigned int i;

        for (i = 0; i + 1 < bars; i++)
        {

            if ((pci_ind(id, PCI_CONFIG_BAR0 + i * 4) & PCI_BAR_TYPE) == PCI_BAR_MEMORY64)
            {

                upper = max(upper, pci_ind(id, PCI_CONFIG_BAR0 + (i + 1) * 4));
                i++;

            }

        }

        if (header == PCI_HEADERTYPE_BRIDGE)
            upper = max(upper, pci_ind(id, PCI_BRIDGE_PREFLIMITUPPER));

    }

    return upper;

}

static unsigned int assignregisters(unsigned int id)
{

    unsigned short command = pci_inw(id, PCI_CONFIG_COMMAND);
    unsigned int inuse;
    unsigned int upper;
    unsigned int limit;

    if (sizeof (unsigned long) == 4 || (pci_ind(id, PCI_CONFIG_BAR0) & PCI_BAR_TYPE) != PCI_BAR_MEMORY64)
        return 0;

    /* firmware leaves these unassigned, as Linux shows assigning them; take a 4 GB slot above everything in use */
    inuse = getupperinuse();
    upper = inuse + 2;
    limit = getupperlimit();

    if (upper >= limit)
        return 0;

    pci_outw(id, PCI_CONFIG_COMMAND, command & ~PCI_COMMAND_MEMORY);
    pci_outd(id, PCI_CONFIG_BAR0, 0);
    pci_outd(id, PCI_CONFIG_BAR1, upper);
    pci_outw(id, PCI_CONFIG_COMMAND, command);

    return pci_ind(id, PCI_CONFIG_BAR1) == upper;

}

static unsigned int disable(struct controller *controller)
{

    write32(controller, REG_ENABLE, 0);

    return waitbits32(controller, REG_ENABLESTATUS, 1, 0);

}

static unsigned int getcycles(unsigned int ns)
{

    return (CLOCKKHZ * ns + 500000) / 1000000;

}

/* what Linux computes without FMCN/SSCN: the bus minimum high and low times plus the default 300 ns fall time, less the controller's own cycles, and its 230 ns SDA hold for these Intel controllers */
static struct timing computetiming(unsigned int high, unsigned int low)
{

    struct timing timing;

    timing.found = 1;
    timing.high = getcycles(high + TFALL) - 3;
    timing.low = getcycles(low + TFALL) - 1;
    timing.hold = getcycles(TSDAHOLD);

    return timing;

}

static void setmode(struct controller *controller, unsigned int speed, unsigned int hcnt, unsigned int lcnt, struct timing *timing)
{

    controller->mode.speed = speed;
    controller->mode.hcnt = hcnt;
    controller->mode.lcnt = lcnt;
    controller->mode.high = timing->high;
    controller->mode.low = timing->low;

    /* like Linux: hold the transmitted SDA as the firmware says and the received one a cycle */
    controller->mode.hold = timing->hold | (1 << REG_SDAHOLD_RXSHIFT);

}

/* the counts and SDA hold the firmware gives for the controller (FMCN for 400 kHz, SSCN for 100 kHz), else our own standard mode guess for a 120 MHz clock */
static void choosemode(struct controller *controller, unsigned int index, unsigned int speed)
{

    struct timing fast;
    struct timing standard;

    fast = findtiming(index, "FMCN");
    standard = findtiming(index, "SSCN");

    if (speed >= 400000 && fast.found && fast.high && fast.low)
        setmode(controller, REG_CON_FAST, REG_FSHCNT, REG_FSLCNT, &fast);
    else if (standard.found && standard.high && standard.low)
        setmode(controller, REG_CON_STANDARD, REG_SSHCNT, REG_SSLCNT, &standard);
    else if (speed >= 400000)
    {

        struct timing computed = computetiming(TFAST_HIGH, TFAST_LOW);

        setmode(controller, REG_CON_FAST, REG_FSHCNT, REG_FSLCNT, &computed);

    }

    else
    {

        struct timing computed = computetiming(TSTANDARD_HIGH, TSTANDARD_LOW);

        setmode(controller, REG_CON_STANDARD, REG_SSHCNT, REG_SSLCNT, &computed);

    }

}

static unsigned int setup(struct controller *controller)
{

    if (!disable(controller))
        return 0;

    write32(controller, REG_CON, REG_CON_MASTER | controller->mode.speed | REG_CON_RESTART | REG_CON_SLAVEDISABLE);
    write32(controller, controller->mode.hcnt, controller->mode.high);
    write32(controller, controller->mode.lcnt, controller->mode.low);
    write32(controller, REG_SDAHOLD, controller->mode.hold);
    write32(controller, REG_INTRMASK, 0);
    write32(controller, REG_RXTL, 0);
    write32(controller, REG_TXTL, 0);

    return 1;

}

/* a transfer that runs out of time is still on the bus: like Linux, have the controller finish it with a STOP before turning it off, so the device is not left holding SDA */
static void stoptransfer(struct controller *controller)
{

    write32(controller, REG_ENABLE, REG_ENABLE_ENABLE | REG_ENABLE_ABORT);
    waitbits32(controller, REG_ENABLE, REG_ENABLE_ABORT, 0);
    read32(controller, REG_CLRINTR);
    disable(controller);

}

static unsigned int runtransfer(struct controller *controller, unsigned int address, struct i2c_transfer *transfer)
{

    unsigned int sent = 0;
    unsigned int asked = 0;
    unsigned int received = 0;
    unsigned int i;

    if (!disable(controller))
        return 0;

    write32(controller, REG_TAR, address);
    read32(controller, REG_CLRINTR);
    write32(controller, REG_ENABLE, 1);

    for (i = 0; i < transfer->timeout; i++)
    {

        unsigned int status = read32(controller, REG_STATUS);
        unsigned int raw = read32(controller, REG_RAWINTRSTAT);

        if (raw & REG_RAWINTRSTAT_TXABRT)
        {

            transfer->abort = read32(controller, REG_TXABRTSOURCE);

            read32(controller, REG_CLRINTR);
            disable(controller);

            return 0;

        }

        if (received < transfer->incount && (status & REG_STATUS_RFNE))
        {

            transfer->in[received] = read32(controller, REG_DATACMD);
            received++;

        }

        else if (sent < transfer->outcount && (status & REG_STATUS_TFNF))
        {

            unsigned int command = transfer->out[sent];

            if (sent + 1 == transfer->outcount && !transfer->incount)
                command |= REG_DATACMD_STOP;

            write32(controller, REG_DATACMD, command);

            sent++;

        }

        else if (sent == transfer->outcount && asked < transfer->incount && asked - received < RXFIFO && (status & REG_STATUS_TFNF))
        {

            unsigned int command = REG_DATACMD_READ;

            if (!asked && transfer->outcount)
                command |= REG_DATACMD_RESTART;

            if (asked + 1 == transfer->incount)
                command |= REG_DATACMD_STOP;

            write32(controller, REG_DATACMD, command);

            asked++;

        }

        else if (sent == transfer->outcount && received == transfer->incount && (raw & REG_RAWINTRSTAT_STOPDET))
        {

            disable(controller);

            return 1;

        }

    }

    stoptransfer(controller);

    return 0;

}

unsigned int i2c_getaddress(unsigned int id)
{

    return (id && id <= nclients) ? clients[id - 1].address : 0;

}

unsigned int i2c_gethidregister(unsigned int id)
{

    return (id && id <= nclients) ? clients[id - 1].hidregister : 0;

}

struct i2c_pad i2c_getpad(unsigned int id)
{

    struct i2c_pad pad;

    pad.found = id && id <= nclients && clients[id - 1].haspad;
    pad.group = (pad.found) ? clients[id - 1].padgroup : 0;
    pad.pad = (pad.found) ? clients[id - 1].padnumber : 0;

    return pad;

}

unsigned int i2c_ishid(unsigned int id)
{

    return id && id <= nclients && clients[id - 1].hid;

}

static struct controller *findcontroller(unsigned int id)
{

    unsigned int index = (id && id <= nclients) ? clients[id - 1].controller : CONTROLLERS;

    return (index < CONTROLLERS && controllers[index].mmio) ? &controllers[index] : 0;

}

unsigned int i2c_runtransfer(struct i2c_transfer *transfer)
{

    struct controller *controller = findcontroller(transfer->id);
    unsigned int ok;

    if (!controller)
        return 0;

    spinlock_acquire(&controller->lock);

    ok = runtransfer(controller, clients[transfer->id - 1].address, transfer);

    spinlock_release(&controller->lock);

    return ok;

}

void i2c_wait(unsigned int id, unsigned int count)
{

    struct controller *controller = findcontroller(id);
    unsigned int i;

    if (!controller)
        return;

    for (i = 0; i < count; i++)
        read32(controller, REG_STATUS);

}

static unsigned int countcontrollers(void)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < CONTROLLERS; i++)
    {

        if (controllers[i].mmio)
            count++;

    }

    return count;

}

static unsigned int isused(unsigned int index)
{

    unsigned int i;

    for (i = 0; i < nclients; i++)
    {

        if (clients[i].controller == index || (clients[i].controller == CONTROLLERS && !countcontrollers()))
            return 1;

    }

    return 0;

}

/* the slowest device on the controller sets its speed */
static unsigned int getspeed(unsigned int index)
{

    unsigned int speed = 0;
    unsigned int i;

    for (i = 0; i < nclients; i++)
    {

        if ((clients[i].controller == index || clients[i].controller == CONTROLLERS) && (!speed || clients[i].speed < speed))
            speed = clients[i].speed;

    }

    return speed;

}

static unsigned int ismatch(unsigned int id)
{

    unsigned int index = getindex(id);

    /* without any device the first controller is still set up, for its log */
    return index < CONTROLLERS && !controllers[index].mmio && ((!nclients && !countcontrollers()) || isused(index));

}

static void reset(unsigned int id)
{

    unsigned long address = pci_getbar(id, 0);
    unsigned int index = getindex(id);
    struct controller *controller = &controllers[index];
    unsigned int low;
    unsigned int high;

    if (!address && assignregisters(id))
        address = pci_getbar(id, 0);

    if (!address)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: controller %u has no registers (BAR0 unset or above 4 GB on 32-bit)", &index);

        return;

    }

    low = address;
    high = high32(address);

    poweron(id);

    controller->mmio = mapregisters(address, index);

    write32(controller, LPSS_RESETS, 0);
    write32(controller, LPSS_RESETS, LPSS_RESETS_DONE);
    write32(controller, LPSS_REMAPLOW, low);
    write32(controller, LPSS_REMAPHIGH, high);

    if (read32(controller, REG_COMPTYPE) != COMPTYPE_DESIGNWARE)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: not a DesignWare controller");

        controller->mmio = 0;

        return;

    }

    choosemode(controller, index, getspeed(index));

    if (!setup(controller))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: controller does not disable");

        controller->mmio = 0;

        return;

    }

}

static unsigned int getfirstcontroller(void)
{

    unsigned int i;

    for (i = 0; i < CONTROLLERS; i++)
    {

        if (controllers[i].mmio)
            return i;

    }

    return CONTROLLERS;

}

static void bus_setup(void)
{

    struct resource *current = 0;
    unsigned int i;

    while ((current = resource_foreachtype(current, RESOURCE_BUS)))
    {

        struct base_bus *pcibus = current->data;

        if (pcibus->type == PCI_BUS)
        {

            unsigned int id = 0;

            while ((id = pcibus->next(id)))
            {

                if (ismatch(id))
                    reset(id);

            }

        }

    }

    /* a device whose controller the scan could not name goes on the first one */
    for (i = 0; i < nclients; i++)
    {

        if (clients[i].controller == CONTROLLERS)
            clients[i].controller = getfirstcontroller();

    }

    /* a one-byte read shows whether each device answers at its address before any driver takes it */
    for (i = 0; i < nclients; i++)
    {

        struct controller *controller = findcontroller(i + 1);

        if (!controller)
            continue;

        check.outcount = 0;
        check.incount = 1;
        check.abort = 0;
        check.timeout = TIMEOUT;

        if (!runtransfer(controller, clients[i].address, &check))
            debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: device 0x%H2u on controller %u does not answer, abort 0x%H8u", &clients[i].address, &clients[i].controller, &check.abort);

    }

}

static unsigned int bus_next(unsigned int id)
{

    unsigned int i;

    for (i = id; i < nclients; i++)
    {

        if (findcontroller(i + 1))
            return i + 1;

    }

    return 0;

}

void module_init(void)
{

    unsigned int i;

    for (i = 0; i < CONTROLLERS; i++)
        spinlock_init(&controllers[i].lock);

    if (!readsettings() && !scantables())
        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: no PNP0C50 device in the ACPI tables");
    else if (!nclients)
        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: PNP0C50 device without a static I2C address");

    base_initbus(&bus, I2C_BUS, "i2c", bus_setup, bus_next);

}

void module_register(void)
{

    base_registerbus(&bus);

}

void module_unregister(void)
{

    unsigned int i;

    base_unregisterbus(&bus);

    for (i = 0; i < CONTROLLERS; i++)
    {

        if (controllers[i].mmio)
            disable(&controllers[i]);

    }

}

