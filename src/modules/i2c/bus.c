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
#define LPSS_CAPS                       0x2FC
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
#define REG_COMPPARAM1                  0xF4
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

};

static struct base_bus bus;
static struct client clients[DEVICES];
static unsigned int nclients;
static struct mode mode;
static struct spinlock buslock;
static unsigned long mmio;
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

static void addclient(unsigned int address, unsigned int index, unsigned int speed, unsigned int hid)
{

    unsigned int i;

    for (i = 0; i < nclients; i++)
    {

        if (clients[i].address == address && clients[i].controller == index)
            return;

    }

    if (nclients == DEVICES)
        return;

    clients[nclients].address = address;
    clients[nclients].controller = index;
    clients[nclients].speed = speed;
    clients[nclients].hid = hid;
    nclients++;

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

    /* firmware templates leave address 0 (general call) until their methods fill it in */
    if (hid && address)
        addclient(address, getcontroller(scan, source), speed, hid);

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

static struct scan getscan(struct acpi_sdth *header)
{

    struct scan scan;

    scan.aml = (unsigned char *)(header + 1);
    scan.length = header->length - sizeof (struct acpi_sdth);

    buffer_copy(scan.signature, header->signature, 4);

    scan.signature[4] = '\0';

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

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: %s.%s %s 0x%H4u", scan->signature, name, field.found ? "is" : "not found, taking", &field.value);

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

    if (timing.found)
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: %s.%s high %u low %u hold %u", device, method, &timing.high, &timing.low, &timing.hold);

    return timing;

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

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: highest address in use 0x%H2u00000000, assigning 0x%H2u00000000, limit 0x%H2u00000000", &inuse, &upper, &limit);

    if (upper >= limit)
        return 0;

    pci_outw(id, PCI_CONFIG_COMMAND, command & ~PCI_COMMAND_MEMORY);
    pci_outd(id, PCI_CONFIG_BAR0, 0);
    pci_outd(id, PCI_CONFIG_BAR1, upper);
    pci_outw(id, PCI_CONFIG_COMMAND, command);

    return pci_ind(id, PCI_CONFIG_BAR1) == upper;

}

static unsigned int disable(void)
{

    write32(REG_ENABLE, 0);

    return waitbits32(REG_ENABLESTATUS, 1, 0);

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

static void setmode(unsigned int speed, unsigned int hcnt, unsigned int lcnt, struct timing *timing)
{

    mode.speed = speed;
    mode.hcnt = hcnt;
    mode.lcnt = lcnt;
    mode.high = timing->high;
    mode.low = timing->low;

    /* like Linux: hold the transmitted SDA as the firmware says and the received one a cycle */
    mode.hold = timing->hold | (1 << REG_SDAHOLD_RXSHIFT);

}

/* the counts and SDA hold the firmware gives for the controller (FMCN for 400 kHz, SSCN for 100 kHz), else our own standard mode guess for a 120 MHz clock */
static void choosemode(unsigned int index, unsigned int speed)
{

    struct timing fast;
    struct timing standard;

    fast = findtiming(index, "FMCN");
    standard = findtiming(index, "SSCN");

    if (speed >= 400000 && fast.found && fast.high && fast.low)
    {

        setmode(REG_CON_FAST, REG_FSHCNT, REG_FSLCNT, &fast);
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: fast mode from FMCN");

    }

    else if (standard.found && standard.high && standard.low)
    {

        setmode(REG_CON_STANDARD, REG_SSHCNT, REG_SSLCNT, &standard);
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: standard mode from SSCN");

    }

    else if (speed >= 400000)
    {

        struct timing computed = computetiming(TFAST_HIGH, TFAST_LOW);

        setmode(REG_CON_FAST, REG_FSHCNT, REG_FSLCNT, &computed);
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: no timing from the firmware, fast mode computed for 120 MHz");

    }

    else
    {

        struct timing computed = computetiming(TSTANDARD_HIGH, TSTANDARD_LOW);

        setmode(REG_CON_STANDARD, REG_SSHCNT, REG_SSLCNT, &computed);
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: no timing from the firmware, standard mode computed for 120 MHz");

    }

}

static unsigned int setup(void)
{

    if (!disable())
        return 0;

    write32(REG_CON, REG_CON_MASTER | mode.speed | REG_CON_RESTART | REG_CON_SLAVEDISABLE);
    write32(mode.hcnt, mode.high);
    write32(mode.lcnt, mode.low);
    write32(REG_SDAHOLD, mode.hold);
    write32(REG_INTRMASK, 0);
    write32(REG_RXTL, 0);
    write32(REG_TXTL, 0);

    return 1;

}

/* a transfer that runs out of time is still on the bus: like Linux, have the controller finish it with a STOP before turning it off, so the device is not left holding SDA */
static void stoptransfer(void)
{

    write32(REG_ENABLE, REG_ENABLE_ENABLE | REG_ENABLE_ABORT);
    waitbits32(REG_ENABLE, REG_ENABLE_ABORT, 0);
    read32(REG_CLRINTR);
    disable();

}

static unsigned int runtransfer(struct i2c_transfer *transfer)
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

    for (i = 0; i < transfer->timeout; i++)
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

    stoptransfer();

    return 0;

}

unsigned int i2c_getaddress(unsigned int id)
{

    return (id && id <= nclients) ? clients[id - 1].address : 0;

}

unsigned int i2c_ishid(unsigned int id)
{

    return id && id <= nclients && clients[id - 1].hid;

}

unsigned int i2c_runtransfer(struct i2c_transfer *transfer)
{

    unsigned int ok;

    if (!mmio)
        return 0;

    spinlock_acquire(&buslock);

    ok = runtransfer(transfer);

    spinlock_release(&buslock);

    return ok;

}

void i2c_wait(unsigned int count)
{

    unsigned int i;

    if (!mmio)
        return;

    for (i = 0; i < count; i++)
        read32(REG_STATUS);

}

static unsigned int ismatch(unsigned int id)
{

    unsigned int index = getindex(id);

    if (pci_inw(id, PCI_CONFIG_VENDOR) == VENDOR_INTEL && pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_SERIAL && pci_inb(id, PCI_CONFIG_SUBCLASS) == SUBCLASS_SERIALOTHER)
    {

        unsigned int device = pci_inw(id, PCI_CONFIG_DEVICE);
        unsigned int bar = pci_ind(id, PCI_CONFIG_BAR0);
        unsigned int number = (id >> 16) & 0xFF;
        unsigned int slot = (id >> 11) & 0x1F;
        unsigned int function = (id >> 8) & 0x07;

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: serial io %H2u:%H2u.%u device 0x%H4u bar 0x%H8u index %u", &number, &slot, &function, &device, &bar, &index);

    }

    return index < CONTROLLERS && (!nclients || clients[0].controller == CONTROLLERS || index == clients[0].controller);

}

static void reset(unsigned int id)
{

    unsigned long address = pci_getbar(id, 0);
    unsigned int index = getindex(id);
    unsigned int device = pci_inw(id, PCI_CONFIG_DEVICE);
    unsigned int low;
    unsigned int high;
    unsigned int comptype;
    unsigned int param;
    unsigned int caps;

    if (!address && assignregisters(id))
        address = pci_getbar(id, 0);

    if (!address)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: controller %u has no registers (BAR0 unset or above 4 GB on 32-bit)", &index);

        return;

    }

    low = address;
    high = high32(address);

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: controller %u device 0x%H4u at 0x%H8u%H8u", &index, &device, &high, &low);
    poweron(id);

    mmio = mapregisters(address);

    write32(LPSS_RESETS, 0);
    write32(LPSS_RESETS, LPSS_RESETS_DONE);
    write32(LPSS_REMAPLOW, low);
    write32(LPSS_REMAPHIGH, high);

    comptype = read32(REG_COMPTYPE);
    param = read32(REG_COMPPARAM1);
    caps = read32(LPSS_CAPS);

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: type 0x%H8u param 0x%H8u caps 0x%H8u", &comptype, &param, &caps);

    if (comptype != COMPTYPE_DESIGNWARE)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: not a DesignWare controller");

        mmio = 0;

        return;

    }

    choosemode(index, (nclients) ? clients[0].speed : 0);

    if (!setup())
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: controller does not disable");

        mmio = 0;

        return;

    }

}

static unsigned int findcontroller(void)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_BUS)))
    {

        struct base_bus *pcibus = current->data;

        if (pcibus->type == PCI_BUS)
        {

            unsigned int id = 0;

            while ((id = pcibus->next(id)))
            {

                if (ismatch(id))
                    return id;

            }

        }

    }

    return 0;

}

static void bus_setup(void)
{

    unsigned int id = findcontroller();

    if (id)
        reset(id);

}

static unsigned int bus_next(unsigned int id)
{

    unsigned int i;

    if (!mmio)
        return 0;

    for (i = id; i < nclients; i++)
    {

        if (clients[i].controller == clients[0].controller)
            return i + 1;

    }

    return 0;

}

void module_init(void)
{

    spinlock_init(&buslock);

    if (!scantables(1))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: no PNP0C50 device in the ACPI tables, all I2C devices:");
        scantables(0);

    }

    else if (!nclients)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: PNP0C50 device without a static I2C address");

    }

    else
    {

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: using address 0x%H2u on controller %u", &clients[0].address, &clients[0].controller);

    }

    base_initbus(&bus, I2C_BUS, "i2c", bus_setup, bus_next);

}

void module_register(void)
{

    base_registerbus(&bus);
    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: done");

}

void module_unregister(void)
{

    base_unregisterbus(&bus);

    if (mmio)
        disable();

}

