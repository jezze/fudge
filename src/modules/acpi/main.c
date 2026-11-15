#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/io/io.h>
#include <modules/acpi/acpi.h>

#define TABLES                          32
#define FADT_XDSDT                      140
#define FADT_SMICMD                     48
#define FADT_ACPIENABLE                 52
#define FADT_PM1ACNT                    64
#define FADT_PM1BCNT                    68
#define FADT_FLAGS                      112
#define FADT_RESETREG                   116
#define FADT_RESETVALUE                 128
#define FADT_XPM1ACNT                   172
#define FADT_XPM1BCNT                   184
#define FADT_FLAG_RESETREG              0x00000400
#define FADT_FLAG_HWREDUCED             0x00100000
#define GAS_SPACE_IO                    0x01
#define PM1_SCIEN                       0x0001
#define PM1_SLPTYP                      0x1C00
#define PM1_SLPEN                       0x2000
#define PM1_WAIT                        100000
#define SCI_WAIT                        1000000
#define AML_EXTPREFIX                   0x5B
#define AML_OPREGION                    0x80
#define AML_FIELD                       0x81
#define AML_BYTEPREFIX                  0x0A
#define AML_WORDPREFIX                  0x0B
#define AML_DWORDPREFIX                 0x0C
#define AML_QWORDPREFIX                 0x0E
#define AML_SPACE_MEMORY                0x00
#define AML_NAME                        0x08
#define AML_PACKAGE                     0x12
#define AML_ZERO                        0x00
#define AML_ONE                         0x01
#define AML_ONES                        0xFF
#define AML_DUALNAME                    0x2E
#define AML_MULTINAME                   0x2F
#define FIELD_RESERVED                  0x00
#define FIELD_ACCESS                    0x01
#define FIELD_EXTENDEDACCESS            0x03

struct aml
{

    unsigned char *data;
    unsigned int length;

};

struct pkglength
{

    unsigned int length;
    unsigned int bytes;

};

struct integer
{

    unsigned int found;
    unsigned long value;
    unsigned int bytes;

};

struct namestring
{

    unsigned int seg;
    unsigned int bytes;

};

struct region
{

    unsigned int found;
    unsigned long address;
    unsigned long length;

};

struct fieldunit
{

    unsigned int found;
    unsigned int bitoffset;
    unsigned int bitwidth;
    char region[5];

};

static unsigned long sdt;
static unsigned int sdtentrysize;
static struct acpi_sdth *dsdt;

static unsigned int high32(unsigned long address)
{

    return (address >> 16) >> 16;

}

static unsigned long readaddress(unsigned char *data, unsigned int size)
{

    unsigned long low = data[0] | (data[1] << 8) | (data[2] << 16) | ((unsigned long)data[3] << 24);
    unsigned long high = (size == 8) ? data[4] | (data[5] << 8) | (data[6] << 16) | ((unsigned long)data[7] << 24) : 0;

    if (!high)
        return low;

    return (sizeof (unsigned long) > 4) ? low | ((high << 16) << 16) : 0;

}

static unsigned int validate(void *address, unsigned int length)
{

    unsigned char *x = address;
    unsigned char sum = 0;
    unsigned int i;

    for (i = 0; i < length; i++)
        sum += x[i];

    return !sum;

}

static struct acpi_rsdp *findrsdp(void)
{

    struct acpi_rsdp *firmware = (struct acpi_rsdp *)ARCH_FIRMWARE_ACPI;
    char *signature = "RSD PTR ";

    if (!buffer_match(firmware->signature, signature, 8))
    {

        unsigned long address;

        arch_kmap(0x000E0000, 0x000E0000, 0x00020000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

        for (address = 0x000E0000; address < 0x00100000; address += 0x10)
        {

            struct acpi_rsdp *rsdp = (struct acpi_rsdp *)address;

            if (buffer_match(rsdp->signature, signature, 8))
                return rsdp;

        }

        return 0;

    }

    return firmware;

}

static unsigned int getcount(void)
{

    struct acpi_sdth *sdth = (struct acpi_sdth *)sdt;

    return (sdth->length - sizeof (struct acpi_sdth)) / sdtentrysize;

}

static unsigned long getentry(unsigned int index)
{

    unsigned char *entries = (unsigned char *)((struct acpi_sdth *)sdt + 1);

    return readaddress(entries + index * sdtentrysize, sdtentrysize);

}

static void maptable(unsigned long address)
{

    struct acpi_sdth *header = (struct acpi_sdth *)address;

    arch_kmap(address, address, 0x00010000, MMAP_FLAG_GLOBAL);

    if (header->length > 0x00010000)
        arch_kmap(address, address, header->length, MMAP_FLAG_GLOBAL);

}

struct acpi_sdth *acpi_findheader(char *name, unsigned int index)
{

    if (buffer_match(name, "DSDT", 4))
        return (index) ? 0 : dsdt;

    if (sdt)
    {

        unsigned int i;

        for (i = 0; i < getcount(); i++)
        {

            struct acpi_sdth *entry = (struct acpi_sdth *)getentry(i);

            if (entry && buffer_match(entry->signature, name, 4))
            {

                if (!index)
                    return entry;

                index--;

            }

        }

    }

    return 0;

}

static unsigned int getbyte(struct aml *aml, unsigned int offset)
{

    return (offset < aml->length) ? aml->data[offset] : 0;

}

static struct pkglength getpkglength(struct aml *aml, unsigned int offset)
{

    struct pkglength pkglength;
    unsigned int lead = getbyte(aml, offset);
    unsigned int i;

    pkglength.bytes = 1 + (lead >> 6);
    pkglength.length = (pkglength.bytes == 1) ? lead & 0x3F : lead & 0x0F;

    for (i = 1; i < pkglength.bytes; i++)
        pkglength.length |= getbyte(aml, offset + i) << (i * 8 - 4);

    return pkglength;

}

static unsigned int skipprefixes(struct aml *aml, unsigned int offset)
{

    while (getbyte(aml, offset) == '\\' || getbyte(aml, offset) == '^')
        offset++;

    return offset;

}

static unsigned int matchname(struct aml *aml, unsigned int offset, char *name)
{

    return offset + 4 <= aml->length && buffer_match(aml->data + offset, name, 4);

}

static struct integer getinteger(struct aml *aml, unsigned int offset)
{

    struct integer integer;
    unsigned int size;
    unsigned int i;

    switch (getbyte(aml, offset))
    {

    case AML_BYTEPREFIX:
        size = 1;

        break;

    case AML_WORDPREFIX:
        size = 2;

        break;

    case AML_DWORDPREFIX:
        size = 4;

        break;

    case AML_QWORDPREFIX:
        size = 8;

        break;

    default:
        size = 0;

        break;

    }

    integer.found = (size != 0);
    integer.value = 0;
    integer.bytes = 1 + size;

    for (i = 0; i < size && i < sizeof (unsigned long); i++)
        integer.value |= (unsigned long)getbyte(aml, offset + 1 + i) << (i * 8);

    return integer;

}

static struct aml getaml(struct acpi_sdth *header)
{

    struct aml aml;

    aml.data = (unsigned char *)(header + 1);
    aml.length = header->length - sizeof (struct acpi_sdth);

    return aml;

}

static struct acpi_sdth *gettable(unsigned int index)
{

    return (index) ? acpi_findheader("SSDT", index - 1) : dsdt;

}

/* where the last NameSeg of the NameString at offset is, and how many bytes the NameString takes */
static struct namestring getnamestring(struct aml *aml, unsigned int offset)
{

    struct namestring namestring;
    unsigned int start = offset;

    offset = skipprefixes(aml, offset);

    if (getbyte(aml, offset) == AML_DUALNAME)
    {

        namestring.seg = offset + 5;
        namestring.bytes = offset + 9 - start;

    }

    else if (getbyte(aml, offset) == AML_MULTINAME && getbyte(aml, offset + 1))
    {

        namestring.seg = offset + 2 + (getbyte(aml, offset + 1) - 1) * 4;
        namestring.bytes = offset + 2 + getbyte(aml, offset + 1) * 4 - start;

    }

    else
    {

        namestring.seg = offset;
        namestring.bytes = offset + 4 - start;

    }

    return namestring;

}

/* Name (<name>, <integer>) anywhere in the tables, the way firmware stores addresses it patches at boot */
static struct integer findnamedinteger(struct aml *name, unsigned int nameoffset)
{

    struct integer integer;
    unsigned int t;

    integer.found = 0;

    for (t = 0; t < TABLES; t++)
    {

        struct acpi_sdth *header = gettable(t);
        struct aml aml;
        unsigned int i;

        if (!header)
            continue;

        aml = getaml(header);

        for (i = 0; i + 5 < aml.length; i++)
        {

            struct namestring namestring;

            if (getbyte(&aml, i) != AML_NAME)
                continue;

            namestring = getnamestring(&aml, i + 1);

            if (namestring.seg + 4 <= aml.length && buffer_match(aml.data + namestring.seg, name->data + nameoffset, 4))
            {

                integer = getinteger(&aml, i + 1 + namestring.bytes);

                if (integer.found)
                    return integer;

            }

        }

    }

    return integer;

}

/* an integer operand: a constant, or a name holding one */
static struct integer getoperand(struct aml *aml, unsigned int offset)
{

    struct integer integer = getinteger(aml, offset);
    unsigned int b = getbyte(aml, offset);

    if (integer.found)
        return integer;

    if (b == AML_ZERO || b == AML_ONE || b == AML_ONES)
    {

        integer.found = 1;
        integer.value = (b == AML_ONES) ? ~0UL : b;
        integer.bytes = 1;

        return integer;

    }

    if (b == '\\' || b == '^' || b == '_' || (b >= 'A' && b <= 'Z') || b == AML_DUALNAME || b == AML_MULTINAME)
    {

        struct namestring namestring = getnamestring(aml, offset);

        integer = findnamedinteger(aml, namestring.seg);
        integer.bytes = namestring.bytes;

        return integer;

    }

    return integer;

}

static struct region findregion(char *name)
{

    struct region region;
    unsigned int t;

    region.found = 0;

    for (t = 0; t < TABLES; t++)
    {

        struct acpi_sdth *header = gettable(t);
        struct aml aml;
        unsigned int i;

        if (!header)
            continue;

        aml = getaml(header);

        for (i = 0; i + 1 < aml.length; i++)
        {

            if (getbyte(&aml, i) == AML_EXTPREFIX && getbyte(&aml, i + 1) == AML_OPREGION)
            {

                struct namestring namestring = getnamestring(&aml, i + 2);
                unsigned int offset = i + 2 + namestring.bytes;

                if (matchname(&aml, namestring.seg, name) && getbyte(&aml, offset) == AML_SPACE_MEMORY)
                {

                    struct integer address = getoperand(&aml, offset + 1);
                    struct integer length = getoperand(&aml, offset + 1 + address.bytes);

                    if (address.found && length.found)
                    {

                        region.found = 1;
                        region.address = address.value;
                        region.length = length.value;

                        return region;

                    }

                }

            }

        }

    }

    return region;

}

static struct fieldunit findfieldunit(struct aml *aml, char *name)
{

    struct fieldunit unit;
    unsigned int i;

    unit.found = 0;

    for (i = 0; i + 1 < aml->length; i++)
    {

        if (getbyte(aml, i) == AML_EXTPREFIX && getbyte(aml, i + 1) == AML_FIELD)
        {

            struct pkglength pkglength = getpkglength(aml, i + 2);
            unsigned int end = i + 2 + pkglength.length;
            struct namestring namestring = getnamestring(aml, i + 2 + pkglength.bytes);
            unsigned int offset = i + 2 + pkglength.bytes + namestring.bytes;
            unsigned int bits = 0;

            if (end > aml->length || namestring.seg + 4 > aml->length)
                continue;

            /* field list: named and reserved units add their bit widths, access entries take no bits */
            for (offset += 1; offset < end;)
            {

                unsigned int b = getbyte(aml, offset);

                if (b == FIELD_RESERVED)
                {

                    struct pkglength width = getpkglength(aml, offset + 1);

                    bits += width.length;
                    offset += 1 + width.bytes;

                }

                else if (b == FIELD_ACCESS)
                {

                    offset += 3;

                }

                else if (b == FIELD_EXTENDEDACCESS)
                {

                    offset += 4;

                }

                else if (b == '_' || (b >= 'A' && b <= 'Z'))
                {

                    struct pkglength width = getpkglength(aml, offset + 4);

                    if (matchname(aml, offset, name))
                    {

                        unit.found = 1;
                        unit.bitoffset = bits;
                        unit.bitwidth = width.length;

                        buffer_copy(unit.region, aml->data + namestring.seg, 4);

                        unit.region[4] = '\0';

                        return unit;

                    }

                    bits += width.length;
                    offset += 4 + width.bytes;

                }

                else
                {

                    break;

                }

            }

            i = end - 1;

        }

    }

    return unit;

}

static unsigned int readbits(unsigned long address, struct fieldunit *unit)
{

    unsigned char *bytes = (unsigned char *)address;
    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < unit->bitwidth && i < 32; i++)
    {

        unsigned int bit = unit->bitoffset + i;

        if (bytes[bit >> 3] & (1 << (bit & 7)))
            value |= 1 << i;

    }

    return value;

}

struct acpi_field acpi_readfield(char *name)
{

    struct acpi_field field;
    unsigned int t;

    field.found = 0;
    field.value = 0;

    for (t = 0; t < TABLES; t++)
    {

        struct acpi_sdth *header = gettable(t);
        struct fieldunit unit;
        struct region region;
        struct aml aml;

        if (!header)
            continue;

        aml = getaml(header);
        unit = findfieldunit(&aml, name);

        if (!unit.found)
            continue;

        region = findregion(unit.region);

        if (!region.found || !region.address || high32(region.address) || (unit.bitoffset + unit.bitwidth + 7) / 8 > region.length)
            return field;

        arch_kmap(region.address, region.address, region.length, MMAP_FLAG_GLOBAL);

        field.found = 1;
        field.value = readbits(region.address, &unit);

        return field;

    }

    return field;

}

static struct integer findpackageinteger(char *name, unsigned int index)
{

    struct integer integer;
    unsigned int t;

    integer.found = 0;

    for (t = 0; t < TABLES; t++)
    {

        struct acpi_sdth *header = gettable(t);
        struct aml aml;
        unsigned int i;

        if (!header)
            continue;

        aml = getaml(header);

        for (i = 0; i + 5 < aml.length; i++)
        {

            struct namestring namestring;
            unsigned int offset;
            unsigned int count;
            unsigned int e;

            if (getbyte(&aml, i) != AML_NAME)
                continue;

            namestring = getnamestring(&aml, i + 1);

            if (!matchname(&aml, namestring.seg, name))
                continue;

            offset = i + 1 + namestring.bytes;

            if (getbyte(&aml, offset) != AML_PACKAGE)
                continue;

            offset += 1 + getpkglength(&aml, offset + 1).bytes;
            count = getbyte(&aml, offset);
            offset++;

            for (e = 0; e < count; e++)
            {

                integer = getoperand(&aml, offset);

                if (!integer.found || e == index)
                    break;

                offset += integer.bytes;

            }

            if (e == index && integer.found)
                return integer;

            integer.found = 0;

        }

    }

    return integer;

}

static unsigned int readfadt(struct acpi_sdth *fadt, unsigned int offset, unsigned int size)
{

    unsigned char *data = (unsigned char *)fadt;
    unsigned int value = 0;
    unsigned int i;

    if (fadt->length < offset + size)
        return 0;

    for (i = 0; i < size; i++)
        value |= data[offset + i] << (i * 8);

    return value;

}

static unsigned int getport(struct acpi_sdth *fadt, unsigned int xoffset, unsigned int offset)
{

    if (fadt->length >= xoffset + 12 && readfadt(fadt, xoffset, 1) == GAS_SPACE_IO)
    {

        unsigned long address = readaddress((unsigned char *)fadt + xoffset + 4, 8);

        if (address && address <= 0xFFFF)
            return address;

    }

    return readfadt(fadt, offset, 4) & 0xFFFF;

}

static void waitport(unsigned int port)
{

    unsigned int i;

    for (i = 0; i < PM1_WAIT; i++)
        io_inw(port);

}

static void enableacpi(struct acpi_sdth *fadt, unsigned int pm1a)
{

    unsigned int smicmd = readfadt(fadt, FADT_SMICMD, 4) & 0xFFFF;
    unsigned int acpienable = readfadt(fadt, FADT_ACPIENABLE, 1);
    unsigned int i;

    if (!smicmd || !acpienable || (io_inw(pm1a) & PM1_SCIEN))
        return;

    io_outb(smicmd, acpienable);

    for (i = 0; i < SCI_WAIT; i++)
    {

        if (io_inw(pm1a) & PM1_SCIEN)
            break;

    }

}

void acpi_reset(void)
{

    struct acpi_sdth *fadt = acpi_findheader("FACP", 0);

    if (!fadt || !(readfadt(fadt, FADT_FLAGS, 4) & FADT_FLAG_RESETREG) || fadt->length < FADT_RESETVALUE + 1)
        return;

    if (readfadt(fadt, FADT_RESETREG, 1) == GAS_SPACE_IO)
    {

        unsigned long address = readaddress((unsigned char *)fadt + FADT_RESETREG + 4, 8);

        if (address && address <= 0xFFFF)
        {

            io_outb(address, readfadt(fadt, FADT_RESETVALUE, 1));
            waitport(address);

        }

    }

}

void acpi_poweroff(void)
{

    struct acpi_sdth *fadt = acpi_findheader("FACP", 0);
    struct integer typea = findpackageinteger("_S5_", 0);
    struct integer typeb = findpackageinteger("_S5_", 1);
    unsigned int pm1a;
    unsigned int pm1b;
    unsigned int a;
    unsigned int b = 0;

    if (!fadt || !typea.found || (readfadt(fadt, FADT_FLAGS, 4) & FADT_FLAG_HWREDUCED))
        return;

    pm1a = getport(fadt, FADT_XPM1ACNT, FADT_PM1ACNT);
    pm1b = getport(fadt, FADT_XPM1BCNT, FADT_PM1BCNT);

    if (!pm1a)
        return;

    enableacpi(fadt, pm1a);

    a = (io_inw(pm1a) & ~(PM1_SLPTYP | PM1_SLPEN)) | ((typea.value << 10) & PM1_SLPTYP);

    if (pm1b)
        b = (io_inw(pm1b) & ~(PM1_SLPTYP | PM1_SLPEN)) | ((((typeb.found) ? typeb.value : typea.value) << 10) & PM1_SLPTYP);

    io_outw(pm1a, a);

    if (pm1b)
        io_outw(pm1b, b);

    io_outw(pm1a, a | PM1_SLPEN);

    if (pm1b)
        io_outw(pm1b, b | PM1_SLPEN);

    waitport(pm1a);

}

void module_init(void)
{

    struct acpi_rsdp *rsdp = findrsdp();

    if (rsdp)
    {

        if (rsdp->revision == 0)
        {

            if (validate(rsdp, sizeof (struct acpi_rsdp)))
            {

                unsigned long address = (rsdp->rsdt[0] << 0) | (rsdp->rsdt[1] << 8) | (rsdp->rsdt[2] << 16) | (rsdp->rsdt[3] << 24);
                struct acpi_rsdt *rsdt = (struct acpi_rsdt *)address;

                arch_kmap(address, address, 0x00010000, MMAP_FLAG_GLOBAL);

                if (validate(rsdt, rsdt->base.length))
                {

                    sdt = address;
                    sdtentrysize = 4;

                }

            }

        }

        if (rsdp->revision == 2)
        {

            struct acpi_xsdp *xsdp = (struct acpi_xsdp *)(rsdp + 1);

            if (validate(rsdp, sizeof (struct acpi_rsdp)) && validate(xsdp, sizeof (struct acpi_xsdp)))
            {

                unsigned long address = readaddress(xsdp->xsdt, 8);
                struct acpi_xsdt *xsdt = (struct acpi_xsdt *)address;

                if (address)
                    arch_kmap(address, address, 0x00010000, MMAP_FLAG_GLOBAL);

                if (address && validate(xsdt, xsdt->base.length))
                {

                    sdt = address;
                    sdtentrysize = 8;

                }

            }

        }

    }

    if (sdt)
    {

        struct acpi_fadt *fadt;
        unsigned int i;

        for (i = 0; i < getcount(); i++)
        {

            if (getentry(i))
                maptable(getentry(i));

        }

        fadt = (struct acpi_fadt *)acpi_findheader("FACP", 0);

        if (fadt)
        {

            unsigned long address = (fadt->base.length >= FADT_XDSDT + 8) ? readaddress((unsigned char *)fadt + FADT_XDSDT, 8) : 0;

            if (!address)
                address = fadt->dsdt;

            if (address)
            {

                maptable(address);

                dsdt = (struct acpi_sdth *)address;

            }

        }

    }

}

