#ifndef MODULES_ACPI_H
#define MODULES_ACPI_H

#define ACPI_DRIVER_TYPE 0x2010

struct acpi_sdth
{

    char signature[4];
    unsigned int length;
    unsigned char revision;
    unsigned char checksum;
    char oem[6];
    char oemTable[8];
    unsigned int oemRevision;
    unsigned int creator;
    unsigned int creatorRevision;

};

struct acpi_rsdt
{

    struct acpi_sdth base;
    struct acpi_sdth **entries;

};

struct acpi_madt_entry
{

    unsigned char type;
    unsigned char length;

};

struct acpi_madt_proclapic
{

    struct acpi_madt_entry base;
    unsigned char processorId;
    unsigned char id;
    unsigned int flags;

};

struct acpi_madt_procsapic
{

    struct acpi_madt_entry base;
    unsigned char processorId;
    unsigned char id;
    unsigned char eid;
    unsigned char reserved[3];
    unsigned int flags;
    unsigned int uid;
    char *uidName;

};

struct acpi_madt_ioapic
{

    struct acpi_madt_entry base;
    unsigned char id;
    unsigned char reserved;
    unsigned int address;
    unsigned int gsib;

};

struct acpi_madt_iosapic
{

    struct acpi_madt_entry base;
    unsigned char id;
    unsigned char reserved;
    unsigned int gsib;
    unsigned int addressLow;
    unsigned int addressHigh;

};

struct acpi_madt
{

    struct acpi_sdth base;
    unsigned int address;
    unsigned int flags;
    struct acpi_madt_entry *entries;

};

struct acpi_rsdp
{

    char signature[8];
    unsigned char checksum;
    char oem[6];
    unsigned char revision;
    unsigned int rsdt;
    unsigned int length;
    unsigned int xsdtLow;
    unsigned int xsdtHigh;
    unsigned char checksum2;
    unsigned char reserved[2];

};

struct acpi_driver
{

    struct modules_driver base;
    struct acpi_rsdp *rsdp;
    struct mmu_memory memory;

};

void acpi_driver_init(struct acpi_driver *driver);

#endif

