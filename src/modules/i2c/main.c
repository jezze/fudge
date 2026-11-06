#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/acpi/acpi.h>
#include <modules/mouse.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>
#include <modules/pci/pci.h>
#include <modules/cpuid/cpuid.h>

#define VENDOR_INTEL                    0x8086
#define SUBCLASS_SERIALOTHER            0x80
#define CONTROLLERS                     6
#define TABLES                          32
#define HIDREGISTERS                    2
#define MMIOSIZE                        0x1000
#define MMIOMAP                         0xB8008000
#define TIMEOUT                         1000000
#define POWERDELAY                      10000
#define RXFIFO                          8
#define TRANSFER_OUT                    16
#define TRANSFER_IN                     512
#define POLLTIMEOUT                     50000
#define DRAINS                          4
#define RETRIES                         2
#define STORM                           10000
#define P2SB                            (0x80000000 | (0x1F << 11) | (1 << 8))
#define P2SB_HIDE                       0xE1
#define SBREG_DEFAULT                   0xFD000000
#define SBREG_PORTSIZE                  0x10000
#define GPIO_PORT                       0x6A
#define GPIO_PAD                        62
#define GPIO_GROUP                      3
#define GPIO_GROUPBIT                   7
#define GPIO_IRQ                        14
#define GPIO_PADBAR                     0x0C
#define GPIO_PADSIZE                    16
#define GPIO_PADCFGLOCK                 0x80
#define GPIO_HOSTSWOWN                  0xB0
#define GPIO_GPIIS                      0x100
#define GPIO_GPIIE                      0x120
#define GPIO_DW0_RXSTATE                (1 << 1)
#define GPIO_DW0_TXDIS                  (1 << 8)
#define GPIO_DW0_RXDIS                  (1 << 9)
#define GPIO_DW0_PMODE                  (7 << 10)
#define GPIO_DW0_ROUTES                 (0xF << 17)
#define GPIO_DW0_RXINV                  (1 << 23)
#define GPIO_DW0_RXEVCFG                (3 << 25)
#define GPIO_DW0_RXEVCFG_EDGE           (1 << 25)
#define RESETDELAY                      200000
#define TIMERIRQ                        0
#define USAGES                          16
#define REPORTIDS                       256
#define BUTTONS                         3
#define DUMPLINE                        16
#define HID_POWERON                     0x0800
#define HID_RESET                       0x0100
#define HID_SETREPORT                   0x03
#define HID_REPORTTYPE_FEATURE          0x03
#define HID_INPUTMODE_TOUCHPAD          3
#define HID_SELECT_ALL                  3
#define TAPTICKS                        12
#define PADCOUNTS                       1600
#define WHEELCOUNTS                     24
#define HID_ITEM_SIZE                   0x03
#define HID_ITEM_LONG                   0xFE
#define HID_MAIN_INPUT                  0x80
#define HID_MAIN_COLLECTION             0xA0
#define HID_MAIN_ENDCOLLECTION          0xC0
#define HID_MAIN_FEATURE                0xB0
#define HID_GLOBAL_LOGICALMAX           0x24
#define HID_GLOBAL_USAGEPAGE            0x04
#define HID_GLOBAL_REPORTSIZE           0x74
#define HID_GLOBAL_REPORTID             0x84
#define HID_GLOBAL_REPORTCOUNT          0x94
#define HID_LOCAL_USAGE                 0x08
#define HID_LOCAL_USAGEMIN              0x18
#define HID_LOCAL_USAGEMAX              0x28
#define HID_INPUT_CONSTANT              0x01
#define HID_INPUT_VARIABLE              0x02
#define HID_INPUT_RELATIVE              0x04
#define HID_COLLECTION_APPLICATION      0x01
#define HID_USAGE_MOUSE                 0x00010002
#define HID_USAGE_X                     0x00010030
#define HID_USAGE_Y                     0x00010031
#define HID_USAGE_WHEEL                 0x00010038
#define HID_PAGE_BUTTON                 0x0009
#define HID_USAGE_BUTTON1               0x00090001
#define HID_USAGE_TOUCHPAD              0x000D0005
#define HID_USAGE_FINGER                0x000D0022
#define HID_USAGE_TIPSWITCH             0x000D0042
#define HID_USAGE_CONTACTID             0x000D0051
#define HID_USAGE_INPUTMODE             0x000D0052
#define HID_USAGE_CONTACTCOUNT          0x000D0054
#define HID_USAGE_SURFACESWITCH         0x000D0057
#define HID_USAGE_BUTTONSWITCH          0x000D0058
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
#define HIDDESC_LENGTH                  30
#define HIDDESC_VERSION                 0x0100
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

struct bus
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

struct touchpad
{

    unsigned int found;
    unsigned int address;
    unsigned int controller;
    unsigned int speed;

};

struct transfer
{

    unsigned int address;
    unsigned char out[TRANSFER_OUT];
    unsigned int outcount;
    unsigned char in[TRANSFER_IN];
    unsigned int incount;
    unsigned int abort;
    unsigned int timeout;

};

struct hid
{

    unsigned int address;
    unsigned int reportregister;
    unsigned int reportlength;
    unsigned int inputregister;
    unsigned int inputlength;
    unsigned int commandregister;
    unsigned int dataregister;

};

struct field
{

    unsigned int found;
    unsigned int offset;
    unsigned int size;
    unsigned int relative;

};

struct hidmouse
{

    unsigned int found;
    unsigned int usesids;
    unsigned int reportid;
    unsigned int buttoncount;
    unsigned int oldbuttons;
    unsigned int length;
    struct field buttons;
    struct field x;
    struct field y;
    struct field wheel;

};

struct hidpad
{

    unsigned int found;
    unsigned int reportid;
    unsigned int modereport;
    unsigned int selectreport;
    unsigned int xmax;
    unsigned int ymax;
    struct field tip;
    struct field contactid;
    struct field x;
    struct field y;
    struct field contactcount;
    struct field button;

};

struct gpiopad
{

    unsigned long community;
    unsigned long dw0;
    unsigned int found;
    unsigned int interrupt;
    unsigned int spurious;

};

struct parser
{

    unsigned int usagepage;
    unsigned int reportsize;
    unsigned int reportcount;
    unsigned int reportid;
    unsigned int usages[USAGES];
    unsigned int nusages;
    unsigned int usagemin;
    unsigned int usagemax;
    unsigned int depth;
    unsigned int mousedepth;
    unsigned int logicalmax;
    unsigned int application;
    unsigned int fingerdepth;
    unsigned int fingerdone;
    unsigned int offsets[REPORTIDS];

};

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static struct touchpad touchpad;
static struct bus bus;
static struct transfer transfer;
static unsigned int unread;
static struct {int x; int y; int wheel;} motion;
static struct hid hid;
static struct hidmouse hidmouse;
static struct hidpad hidpad;
static unsigned int padmode;
static unsigned int readlength;
static unsigned int ticks;
static struct {unsigned int active; unsigned int id; int lastx; int lasty; unsigned int downtick; unsigned int travel; unsigned int fingers; unsigned int button; int accx; int accy; int accw;} touch;
static struct parser parser;
static struct gpiopad gpiopad;
static struct spinlock spinlock;
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

    /* firmware templates leave address 0 (general call) until their methods fill it in */
    if (hid && address && !touchpad.found)
    {

        touchpad.found = 1;
        touchpad.address = address;
        touchpad.controller = getcontroller(scan, source);
        touchpad.speed = speed;

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

static void setbus(unsigned int speed, unsigned int hcnt, unsigned int lcnt, struct timing *timing)
{

    bus.speed = speed;
    bus.hcnt = hcnt;
    bus.lcnt = lcnt;
    bus.high = timing->high;
    bus.low = timing->low;

    /* like Linux: hold the transmitted SDA as the firmware says and the received one a cycle */
    bus.hold = timing->hold | (1 << REG_SDAHOLD_RXSHIFT);

}

/* the counts and SDA hold the firmware gives for the controller (FMCN for 400 kHz, SSCN for 100 kHz), else our own standard mode guess for a 120 MHz clock */
static void choosebus(unsigned int controller)
{

    struct timing fast;
    struct timing standard;

    fast = findtiming(controller, "FMCN");
    standard = findtiming(controller, "SSCN");

    if (touchpad.speed >= 400000 && fast.found && fast.high && fast.low)
    {

        setbus(REG_CON_FAST, REG_FSHCNT, REG_FSLCNT, &fast);
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: fast mode from FMCN");

    }

    else if (standard.found && standard.high && standard.low)
    {

        setbus(REG_CON_STANDARD, REG_SSHCNT, REG_SSLCNT, &standard);
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: standard mode from SSCN");

    }

    else if (touchpad.speed >= 400000)
    {

        struct timing computed = computetiming(TFAST_HIGH, TFAST_LOW);

        setbus(REG_CON_FAST, REG_FSHCNT, REG_FSLCNT, &computed);
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: no timing from the firmware, fast mode computed for 120 MHz");

    }

    else
    {

        struct timing computed = computetiming(TSTANDARD_HIGH, TSTANDARD_LOW);

        setbus(REG_CON_STANDARD, REG_SSHCNT, REG_SSLCNT, &computed);
        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: no timing from the firmware, standard mode computed for 120 MHz");

    }

}

static unsigned int setup(void)
{

    if (!disable())
        return 0;

    write32(REG_CON, REG_CON_MASTER | bus.speed | REG_CON_RESTART | REG_CON_SLAVEDISABLE);
    write32(bus.hcnt, bus.high);
    write32(bus.lcnt, bus.low);
    write32(REG_SDAHOLD, bus.hold);
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
    transfer->timeout = TIMEOUT;

    return runtransfer(transfer) && getinword(transfer, 0) == HIDDESC_LENGTH && getinword(transfer, 2) == HIDDESC_VERSION;

}

static unsigned int sendcommand(struct transfer *transfer, unsigned int command)
{

    transfer->address = hid.address;
    transfer->out[0] = hid.commandregister;
    transfer->out[1] = hid.commandregister >> 8;
    transfer->out[2] = command;
    transfer->out[3] = command >> 8;
    transfer->outcount = 4;
    transfer->incount = 0;
    transfer->abort = 0;
    transfer->timeout = TIMEOUT;

    return runtransfer(transfer);

}

/* SET_REPORT of a one-byte feature report: command register with opcode and report, then the data register, the length, the report id and the value */
static unsigned int setfeature(struct transfer *transfer, unsigned int reportid, unsigned int value)
{

    unsigned int n = 0;

    transfer->address = hid.address;
    transfer->out[n++] = hid.commandregister;
    transfer->out[n++] = hid.commandregister >> 8;

    if (reportid < 15)
    {

        transfer->out[n++] = (HID_REPORTTYPE_FEATURE << 4) | reportid;
        transfer->out[n++] = HID_SETREPORT;

    }

    else
    {

        transfer->out[n++] = (HID_REPORTTYPE_FEATURE << 4) | 15;
        transfer->out[n++] = HID_SETREPORT;
        transfer->out[n++] = reportid;

    }

    transfer->out[n++] = hid.dataregister;
    transfer->out[n++] = hid.dataregister >> 8;
    transfer->out[n++] = 4;
    transfer->out[n++] = 0;
    transfer->out[n++] = reportid;
    transfer->out[n++] = value;
    transfer->outcount = n;
    transfer->incount = 0;
    transfer->abort = 0;
    transfer->timeout = TIMEOUT;

    return runtransfer(transfer);

}

static unsigned int readreportdescriptor(struct transfer *transfer)
{

    transfer->address = hid.address;
    transfer->out[0] = hid.reportregister;
    transfer->out[1] = hid.reportregister >> 8;
    transfer->outcount = 2;
    transfer->incount = hid.reportlength;
    transfer->abort = 0;
    transfer->timeout = TIMEOUT;

    return runtransfer(transfer);

}

static unsigned int readinput(struct transfer *transfer)
{

    transfer->address = hid.address;
    transfer->outcount = 0;
    transfer->incount = readlength;
    transfer->abort = 0;
    transfer->timeout = POLLTIMEOUT;

    return runtransfer(transfer);

}

static void delay(unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        read32(REG_STATUS);

}

static unsigned int getitemdata(struct transfer *transfer, unsigned int offset, unsigned int size)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < size; i++)
        value |= transfer->in[offset + i] << (i * 8);

    return value;

}

static unsigned int getusage(struct parser *parser, unsigned int index)
{

    if (parser->nusages)
        return parser->usages[(index < parser->nusages) ? index : parser->nusages - 1];

    if (parser->usagemax >= parser->usagemin)
        return (parser->usagemin + index <= parser->usagemax) ? parser->usagemin + index : parser->usagemax;

    return 0;

}

static void setfield(struct field *field, unsigned int offset, unsigned int size, unsigned int flags)
{

    if (field->found)
        return;

    field->found = 1;
    field->offset = offset;
    field->size = size;
    field->relative = (flags & HID_INPUT_RELATIVE) ? 1 : 0;

}

/* the touchpad collection of a Precision Touchpad: one finger per report (hybrid reporting), its tip switch, contact id and absolute X/Y, the contact count and the button */
static void parsepadinput(struct parser *parser, struct hidpad *pad, unsigned int base, unsigned int flags)
{

    unsigned int i;

    if (parser->application != HID_USAGE_TOUCHPAD || (flags & HID_INPUT_CONSTANT))
        return;

    if (pad->x.found && pad->reportid != parser->reportid)
        return;

    for (i = 0; i < parser->reportcount; i++)
    {

        unsigned int usage = getusage(parser, i);
        unsigned int offset = base + i * parser->reportsize;

        if (parser->fingerdepth)
        {

            if (usage == HID_USAGE_TIPSWITCH)
                setfield(&pad->tip, offset, parser->reportsize, flags);

            if (usage == HID_USAGE_CONTACTID)
                setfield(&pad->contactid, offset, parser->reportsize, flags);

            if (usage == HID_USAGE_X && !pad->x.found)
            {

                setfield(&pad->x, offset, parser->reportsize, flags);

                pad->xmax = parser->logicalmax;

            }

            if (usage == HID_USAGE_Y && !pad->y.found)
            {

                setfield(&pad->y, offset, parser->reportsize, flags);

                pad->ymax = parser->logicalmax;

            }

        }

        if (usage == HID_USAGE_CONTACTCOUNT)
            setfield(&pad->contactcount, offset, parser->reportsize, flags);

        if (usage == HID_USAGE_BUTTON1)
            setfield(&pad->button, offset, parser->reportsize, flags);

        pad->reportid = parser->reportid;

    }

}

static void parsepadfeature(struct parser *parser, struct hidpad *pad)
{

    unsigned int i;

    for (i = 0; i < parser->reportcount; i++)
    {

        unsigned int usage = getusage(parser, i);

        if (usage == HID_USAGE_INPUTMODE && !pad->modereport)
            pad->modereport = parser->reportid;

        if ((usage == HID_USAGE_SURFACESWITCH || usage == HID_USAGE_BUTTONSWITCH) && !pad->selectreport)
            pad->selectreport = parser->reportid;

    }

}

static void parseinput(struct parser *parser, struct hidmouse *mouse, unsigned int flags)
{

    unsigned int base = parser->offsets[parser->reportid];
    unsigned int i;

    parser->offsets[parser->reportid] += parser->reportsize * parser->reportcount;

    parsepadinput(parser, &hidpad, base, flags);

    if (!parser->mousedepth || (flags & HID_INPUT_CONSTANT))
        return;

    if (mouse->x.found && mouse->reportid != parser->reportid)
        return;

    for (i = 0; i < parser->reportcount; i++)
    {

        unsigned int usage = getusage(parser, i);
        unsigned int offset = base + i * parser->reportsize;

        if ((usage >> 16) == HID_PAGE_BUTTON && parser->reportsize == 1 && (flags & HID_INPUT_VARIABLE))
        {

            if (!mouse->buttons.found)
                setfield(&mouse->buttons, offset, 1, flags);

            if (mouse->buttoncount < BUTTONS && offset == mouse->buttons.offset + mouse->buttoncount)
                mouse->buttoncount++;

        }

        if (usage == HID_USAGE_X)
            setfield(&mouse->x, offset, parser->reportsize, flags);

        if (usage == HID_USAGE_Y)
            setfield(&mouse->y, offset, parser->reportsize, flags);

        if (usage == HID_USAGE_WHEEL)
            setfield(&mouse->wheel, offset, parser->reportsize, flags);

        mouse->reportid = parser->reportid;

    }

}

static void parsereportdescriptor(struct transfer *transfer, struct parser *parser, struct hidmouse *mouse)
{

    unsigned int offset = 0;

    buffer_clear(parser, sizeof (struct parser));
    buffer_clear(mouse, sizeof (struct hidmouse));
    buffer_clear(&hidpad, sizeof (struct hidpad));

    while (offset < transfer->incount)
    {

        unsigned int prefix = transfer->in[offset];
        unsigned int size = ((prefix & HID_ITEM_SIZE) == HID_ITEM_SIZE) ? 4 : prefix & HID_ITEM_SIZE;
        unsigned int data;

        if (prefix == HID_ITEM_LONG)
        {

            offset += 3 + ((offset + 1 < transfer->incount) ? transfer->in[offset + 1] : 0);

            continue;

        }

        if (offset + 1 + size > transfer->incount)
            break;

        data = getitemdata(transfer, offset + 1, size);

        switch (prefix & ~HID_ITEM_SIZE)
        {

        case HID_GLOBAL_USAGEPAGE:
            parser->usagepage = data;

            break;

        case HID_GLOBAL_REPORTSIZE:
            parser->reportsize = data;

            break;

        case HID_GLOBAL_REPORTCOUNT:
            parser->reportcount = data;

            break;

        case HID_GLOBAL_LOGICALMAX:
            parser->logicalmax = data;

            break;

        case HID_GLOBAL_REPORTID:
            parser->reportid = data % REPORTIDS;
            mouse->usesids = 1;

            break;

        case HID_LOCAL_USAGE:
            if (parser->nusages < USAGES)
            {

                parser->usages[parser->nusages] = (size == 4) ? data : (parser->usagepage << 16) | data;
                parser->nusages++;

            }

            break;

        case HID_LOCAL_USAGEMIN:
            parser->usagemin = (size == 4) ? data : (parser->usagepage << 16) | data;

            break;

        case HID_LOCAL_USAGEMAX:
            parser->usagemax = (size == 4) ? data : (parser->usagepage << 16) | data;

            break;

        case HID_MAIN_INPUT:
            parseinput(parser, mouse, data);

            break;

        case HID_MAIN_FEATURE:
            parsepadfeature(parser, &hidpad);

            break;

        case HID_MAIN_COLLECTION:
            parser->depth++;

            if (parser->depth == 1)
                parser->application = getusage(parser, 0);

            if (parser->application == HID_USAGE_TOUCHPAD && !parser->fingerdepth && !parser->fingerdone && getusage(parser, 0) == HID_USAGE_FINGER)
                parser->fingerdepth = parser->depth;

            if (!parser->mousedepth && !mouse->x.found && data == HID_COLLECTION_APPLICATION && getusage(parser, 0) == HID_USAGE_MOUSE)
                parser->mousedepth = parser->depth;

            break;

        case HID_MAIN_ENDCOLLECTION:
            if (parser->depth == parser->mousedepth)
                parser->mousedepth = 0;

            if (parser->depth == parser->fingerdepth)
            {

                parser->fingerdepth = 0;
                parser->fingerdone = 1;

            }

            if (parser->depth == 1)
                parser->application = 0;

            if (parser->depth)
                parser->depth--;

            break;

        }

        if ((prefix & 0x0C) == 0x00)
        {

            parser->nusages = 0;
            parser->usagemin = 0;
            parser->usagemax = 0;

        }

        offset += 1 + size;

    }

    mouse->found = mouse->x.found && mouse->y.found && mouse->x.relative && mouse->y.relative;
    hidpad.found = hidpad.modereport && hidpad.tip.found && hidpad.x.found && hidpad.y.found && !hidpad.x.relative && hidpad.xmax && hidpad.ymax;
    mouse->length = 2 + mouse->usesids + (parser->offsets[mouse->reportid] + 7) / 8;

    /* reading just the mouse report keeps each poll short; the length word says what the device had */
    if (mouse->length > hid.inputlength || mouse->length > TRANSFER_IN)
        mouse->length = (hid.inputlength < TRANSFER_IN) ? hid.inputlength : TRANSFER_IN;

}

static void dumpreportdescriptor(struct transfer *transfer)
{

    unsigned int offset;

    for (offset = 0; offset < transfer->incount; offset += DUMPLINE)
    {

        char line[DUMPLINE * 3 + 1];
        unsigned int count = 0;
        unsigned int i;

        for (i = offset; i < offset + DUMPLINE && i < transfer->incount; i++)
        {

            count += cstring_write_value(line, DUMPLINE * 3, transfer->in[i], 16, 2, count);
            count += buffer_write(line, DUMPLINE * 3, " ", 1, count);

        }

        line[count] = '\0';

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: %u: %s", &offset, line);

    }

}

static int getbits(struct transfer *transfer, unsigned int start, struct field *field, unsigned int index)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < field->size; i++)
    {

        unsigned int bit = start * 8 + field->offset + index * field->size + i;

        if ((bit >> 3) < transfer->incount && (transfer->in[bit >> 3] & (1 << (bit & 7))))
            value |= 1 << i;

    }

    if (field->size > 1 && field->size < 32 && (value & (1 << (field->size - 1))))
        return (int)value - (1 << field->size);

    return value;

}

static char clamp(int value)
{

    return (value > 127) ? 127 : (value < -127) ? -127 : value;

}

static unsigned int readpad(unsigned long address)
{

    return *(volatile unsigned int *)address;

}

static void writepad(unsigned long address, unsigned int value)
{

    *(volatile unsigned int *)address = value;

}

static unsigned int ispadasserted(void)
{

    /* the line is active low; without the pad, read as before and let the device answer empty */
    return !gpiopad.found || !(readpad(gpiopad.dw0) & GPIO_DW0_RXSTATE);

}

static void notifymove(int x, int y)
{

    while (x || y)
    {

        char relx = clamp(x);
        char rely = clamp(y);

        mouse_notifymove(&mouseinterface, relx, rely);

        x -= relx;
        y -= rely;

    }

}

/* motion is sent at most once a tick: a task's mailbox holds 8 messages and kernel_notify drops what does not fit, so a stream of moves would push out presses and releases */
static void flushmotion(void)
{

    notifymove(motion.x, motion.y);

    if (motion.wheel)
        mouse_notifyscroll(&mouseinterface, -clamp(motion.wheel));

    motion.x = 0;
    motion.y = 0;
    motion.wheel = 0;

}

/* one report at a time, so a press and release that arrive in the same drain are both seen; motion before a button change is sent first to keep the order */
static void handlereport(struct transfer *transfer, unsigned int start)
{

    unsigned int buttons = 0;
    unsigned int i;

    for (i = 0; i < hidmouse.buttoncount; i++)
    {

        if (getbits(transfer, start, &hidmouse.buttons, i))
            buttons |= 1 << i;

    }

    if (buttons != hidmouse.oldbuttons)
        flushmotion();

    motion.x += getbits(transfer, start, &hidmouse.x, 0);
    motion.y += getbits(transfer, start, &hidmouse.y, 0);

    if (hidmouse.wheel.found && hidmouse.wheel.relative)
        motion.wheel += getbits(transfer, start, &hidmouse.wheel, 0);

    for (i = 0; i < hidmouse.buttoncount; i++)
    {

        unsigned int bit = 1 << i;

        if (!(hidmouse.oldbuttons & bit) && (buttons & bit))
            mouse_notifypress(&mouseinterface, i + 1);

        if ((hidmouse.oldbuttons & bit) && !(buttons & bit))
            mouse_notifyrelease(&mouseinterface, i + 1);

    }

    hidmouse.oldbuttons = buttons;

}

/* touchpad counts to pointer counts, the whole width being PADCOUNTS; the remainder is kept so slow movement is not lost */
static int scale(int delta, int *accumulator, unsigned int range, unsigned int counts)
{

    int quotient;

    *accumulator += delta * (int)counts;
    quotient = (*accumulator >= 0) ? *accumulator / (int)range : -(-*accumulator / (int)range);
    *accumulator -= quotient * (int)range;

    return quotient;

}

static void click(unsigned int button)
{

    flushmotion();
    mouse_notifypress(&mouseinterface, button);
    mouse_notifyrelease(&mouseinterface, button);

}

/* one finger moves the pointer, two scroll; a short touch that barely moves is a tap: one finger left click, two right click; the pad's own button is the left button */
static void handlepadreport(struct transfer *transfer, unsigned int start)
{

    unsigned int count = (hidpad.contactcount.found) ? getbits(transfer, start, &hidpad.contactcount, 0) : 0;
    unsigned int button = (hidpad.button.found) ? getbits(transfer, start, &hidpad.button, 0) & 1 : 0;
    unsigned int tip = getbits(transfer, start, &hidpad.tip, 0) & 1;
    unsigned int id = (hidpad.contactid.found) ? getbits(transfer, start, &hidpad.contactid, 0) & 0xFF : 0;
    int x = getbits(transfer, start, &hidpad.x, 0) & 0xFFFF;
    int y = getbits(transfer, start, &hidpad.y, 0) & 0xFFFF;

    if (button != touch.button)
    {

        flushmotion();

        if (button)
            mouse_notifypress(&mouseinterface, 1);
        else
            mouse_notifyrelease(&mouseinterface, 1);

        touch.button = button;

    }

    if (touch.active && count > touch.fingers)
        touch.fingers = count;

    if (!touch.active)
    {

        if (tip)
        {

            touch.active = 1;
            touch.id = id;
            touch.lastx = x;
            touch.lasty = y;
            touch.downtick = ticks;
            touch.travel = 0;
            touch.fingers = (count) ? count : 1;
            touch.accx = 0;
            touch.accy = 0;
            touch.accw = 0;

        }

        return;

    }

    if (id != touch.id)
        return;

    if (tip)
    {

        int dx = x - touch.lastx;
        int dy = y - touch.lasty;

        touch.lastx = x;
        touch.lasty = y;
        touch.travel += ((dx < 0) ? -dx : dx) + ((dy < 0) ? -dy : dy);

        if (touch.fingers >= 2)
            motion.wheel -= scale(dy, &touch.accw, hidpad.ymax, WHEELCOUNTS);

        else
        {

            motion.x += scale(dx, &touch.accx, hidpad.xmax, PADCOUNTS);
            motion.y += scale(dy, &touch.accy, hidpad.xmax, PADCOUNTS);

        }

        return;

    }

    if (ticks - touch.downtick <= TAPTICKS && touch.travel <= hidpad.xmax / 50 && !touch.button)
        click((touch.fingers >= 2) ? 2 : 1);

    touch.active = 0;

}

static unsigned int poll(unsigned int force)
{

    unsigned int start = (hidmouse.usesids) ? 3 : 2;
    unsigned int count = 0;
    unsigned int i;

    /* the touchpad drops a report nobody reads soon after its interrupt, so take all it has now; its line can already be released when the interrupt arrives, so an interrupt reads once regardless */
    for (i = 0; i < DRAINS && ((force && !i) || ispadasserted()); i++)
    {

        unsigned int tries;
        unsigned int ok = 0;

        /* the touchpad announced a report: a read that fails leaves it unread, and it waits for it before announcing more; a lost arbitration is retried at once, a timeout (already long) waits for the next tick */
        for (tries = 0; tries <= RETRIES && !ok; tries++)
        {

            ok = readinput(&transfer);

            if (!ok && !transfer.abort)
                break;

        }

        unread = !ok;

        if (!ok || getinword(&transfer, 0) < start + 1)
            break;

        if (padmode && transfer.in[2] == hidpad.reportid)
        {

            handlepadreport(&transfer, start);

            count++;

            continue;

        }

        if (padmode || (hidmouse.usesids && transfer.in[2] != hidmouse.reportid))
            continue;

        handlereport(&transfer, start);

        count++;

    }

    return count;

}

static void handleirq(unsigned int irq)
{

    spinlock_acquire(&spinlock);
    poll(unread);
    flushmotion();
    spinlock_release(&spinlock);

    ticks++;

    gpiopad.spurious = 0;

}

static void stopinterrupt(void)
{

    unsigned long ie = gpiopad.community + GPIO_GPIIE + GPIO_GROUP * 4;

    writepad(ie, readpad(ie) & ~(1 << GPIO_GROUPBIT));

    gpiopad.interrupt = 0;

}

static void handlegpio(unsigned int irq)
{

    unsigned long is = gpiopad.community + GPIO_GPIIS + GPIO_GROUP * 4;
    unsigned int found = 0;

    if (!gpiopad.interrupt)
        return;

    if (readpad(is) & (1 << GPIO_GROUPBIT))
    {

        /* clear first: the next announcement, even one made while we read, latches again and is not wiped out */
        writepad(is, 1 << GPIO_GROUPBIT);
        spinlock_acquire(&spinlock);

        found = poll(1);

        spinlock_release(&spinlock);

    }

    /* a line that keeps firing without reports is set up wrong (pad or apic polarity): give up on it and keep polling */
    gpiopad.spurious = (found) ? 0 : gpiopad.spurious + 1;

    if (gpiopad.spurious == STORM)
    {

        stopinterrupt();
        apic_unsetroutine(GPIO_IRQ, handlegpio);
        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: touchpad interrupt fires without reports, polling instead");

    }

}

static unsigned long getsbreg(void)
{

    unsigned long sbreg;

    /* the sideband registers sit behind the hidden P2SB device: show it, read its BAR, hide it again */
    pci_outb(P2SB, P2SB_HIDE, 0);

    sbreg = pci_ind(P2SB, PCI_CONFIG_BAR0) & 0xFFFFFFF0;

    pci_outb(P2SB, P2SB_HIDE, 1);

    return (!sbreg || sbreg == 0xFFFFFFF0) ? SBREG_DEFAULT : sbreg;

}

static void findpad(void)
{

    unsigned long community = getsbreg() + (GPIO_PORT << 16);
    unsigned int padbar;
    unsigned int dw0;
    unsigned int lock;
    unsigned int owner;

    arch_kmap(community, community, SBREG_PORTSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    padbar = readpad(community + GPIO_PADBAR);

    if (!padbar || padbar >= SBREG_PORTSIZE - (GPIO_PAD + 1) * GPIO_PADSIZE)
        return;

    gpiopad.community = community;
    gpiopad.dw0 = community + padbar + GPIO_PAD * GPIO_PADSIZE;

    dw0 = readpad(gpiopad.dw0);
    lock = readpad(community + GPIO_PADCFGLOCK + GPIO_GROUP * 8);
    owner = readpad(community + GPIO_HOSTSWOWN + GPIO_GROUP * 4);

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: touchpad interrupt pad dw0 0x%H8u lock 0x%H8u owner 0x%H8u", &dw0, &lock, &owner);

    /* the touchpad's line is a GPIO input; anything else means this is not the pin */
    if ((dw0 & GPIO_DW0_PMODE) || (dw0 & GPIO_DW0_RXDIS) || !(dw0 & GPIO_DW0_TXDIS))
        return;

    gpiopad.found = 1;

}

static void startinterrupt(void)
{

    unsigned long is = gpiopad.community + GPIO_GPIIS + GPIO_GROUP * 4;
    unsigned long ie = gpiopad.community + GPIO_GPIIE + GPIO_GROUP * 4;
    unsigned long owner = gpiopad.community + GPIO_HOSTSWOWN + GPIO_GROUP * 4;
    unsigned int dw0 = readpad(gpiopad.dw0);

    /* edge, inverted so each falling edge of the active low line (one announced report) latches the status once, routed to the GPIO controller's shared interrupt instead of SCI, SMI, NMI or the IOxAPIC */
    dw0 &= ~(GPIO_DW0_ROUTES | GPIO_DW0_RXEVCFG);
    dw0 |= GPIO_DW0_RXINV | GPIO_DW0_RXEVCFG_EDGE;

    writepad(gpiopad.dw0, dw0);
    writepad(owner, readpad(owner) | (1 << GPIO_GROUPBIT));
    writepad(is, 1 << GPIO_GROUPBIT);

    if (!apic_setlevel(GPIO_IRQ, 1))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: no apic line for the GPIO interrupt, polling");

        return;

    }

    gpiopad.interrupt = 1;

    apic_setroutine(GPIO_IRQ, handlegpio);
    writepad(ie, readpad(ie) | (1 << GPIO_GROUPBIT));
    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: touchpad interrupt on, pad dw0 0x%H8u", &dw0);

}

static void setupmouse(void)
{

    if (!sendcommand(&transfer, HID_POWERON) || !sendcommand(&transfer, HID_RESET))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: hid power on or reset failed, abort 0x%H8u", &transfer.abort);

        return;

    }

    delay(RESETDELAY);

    if (hid.reportlength > TRANSFER_IN || !readreportdescriptor(&transfer))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: report descriptor failed, abort 0x%H8u", &transfer.abort);

        return;

    }

    parsereportdescriptor(&transfer, &parser, &hidmouse);

    readlength = hidmouse.length;

    /* like Linux, put a Precision Touchpad into touchpad mode and do pointer motion and taps ourselves; its mouse emulation recognises taps poorly */
    if (hidpad.found && setfeature(&transfer, hidpad.modereport, HID_INPUTMODE_TOUCHPAD))
    {

        if (hidpad.selectreport)
            setfeature(&transfer, hidpad.selectreport, HID_SELECT_ALL);

        padmode = 1;
        readlength = (hid.inputlength < TRANSFER_IN) ? hid.inputlength : TRANSFER_IN;

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: touchpad mode, report %u, x max %u y max %u", &hidpad.reportid, &hidpad.xmax, &hidpad.ymax);

        return;

    }

    if (!hidmouse.found)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c: no relative mouse in the report descriptor:");
        dumpreportdescriptor(&transfer);

        return;

    }

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: mouse report %u length %u buttons %u at bit %u x %u bits at %u y %u bits at %u wheel %u", &hidmouse.reportid, &hidmouse.length, &hidmouse.buttoncount, &hidmouse.buttons.offset, &hidmouse.x.size, &hidmouse.x.offset, &hidmouse.y.size, &hidmouse.y.offset, &hidmouse.wheel.found);

}

static void probe(unsigned int address)
{

    unsigned int i;

    for (i = 0; i < HIDREGISTERS; i++)
    {

        unsigned int reg = hidregisters[i];

        if (readhiddescriptor(&transfer, address, reg))
        {

            unsigned int vendor = getinword(&transfer, 20);
            unsigned int product = getinword(&transfer, 22);

            hid.address = address;
            hid.reportlength = getinword(&transfer, 4);
            hid.reportregister = getinword(&transfer, 6);
            hid.inputregister = getinword(&transfer, 8);
            hid.inputlength = getinword(&transfer, 10);
            hid.commandregister = getinword(&transfer, 16);
            hid.dataregister = getinword(&transfer, 18);

            debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: hid 0x%H2u descriptor at 0x%H4u vendor 0x%H4u product 0x%H4u", &address, &reg, &vendor, &product);
            debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: report descriptor %u bytes at 0x%H4u input 0x%H4u max %u command 0x%H4u", &hid.reportlength, &hid.reportregister, &hid.inputregister, &hid.inputlength, &hid.commandregister);
            setupmouse();

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

    if (pci_inw(id, PCI_CONFIG_VENDOR) == VENDOR_INTEL && pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_SERIAL && pci_inb(id, PCI_CONFIG_SUBCLASS) == SUBCLASS_SERIALOTHER)
    {

        unsigned int device = pci_inw(id, PCI_CONFIG_DEVICE);
        unsigned int bar = pci_ind(id, PCI_CONFIG_BAR0);
        unsigned int bus = (id >> 16) & 0xFF;
        unsigned int slot = (id >> 11) & 0x1F;
        unsigned int function = (id >> 8) & 0x07;

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: serial io %H2u:%H2u.%u device 0x%H4u bar 0x%H8u index %u", &bus, &slot, &function, &device, &bar, &index);

    }

    return index < CONTROLLERS && (touchpad.controller == CONTROLLERS || index == touchpad.controller);

}

static void driver_reset(unsigned int id)
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

    choosebus(index);

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

    if (hidmouse.found || padmode)
    {

        mouse_registerinterface(&mouseinterface);
        pic_setroutine(TIMERIRQ, handleirq);
        apic_setroutine(TIMERIRQ, handleirq);
        findpad();

        if (gpiopad.found)
            startinterrupt();

    }

}

static void driver_detach(unsigned int id)
{

    if (hidmouse.found || padmode)
    {

        if (gpiopad.interrupt)
        {

            stopinterrupt();
            apic_unsetroutine(GPIO_IRQ, handlegpio);

        }

        pic_unsetroutine(TIMERIRQ, handleirq);
        apic_unsetroutine(TIMERIRQ, handleirq);
        mouse_unregisterinterface(&mouseinterface);

    }

    if (mmio)
        disable();

}

void module_init(void)
{

    mouse_initinterface(&mouseinterface);
    spinlock_init(&spinlock);

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

    else
    {

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: using address 0x%H2u on controller %u", &touchpad.address, &touchpad.controller);

    }

    base_initdriver(&driver, "i2c", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);
    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c: done");

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

