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

struct acpi_madt
{

    struct acpi_sdth base;
    unsigned int lica;
    unsigned int flags;

};

struct acpi_madt_entry
{

    unsigned char type;
    unsigned char length;

};

struct acpi_madt_lapic
{

    struct acpi_madt_entry base;
    unsigned char processor;
    unsigned char id;
    unsigned int flags;

};

struct acpi_madt_ioapic
{

    struct acpi_madt_entry base;
    unsigned char id;
    unsigned char reserved;
    unsigned int address;
    unsigned int gsibase;

};

struct acpi_madt_ioapic_iso
{

    struct acpi_madt_entry base;
    unsigned char bus;
    unsigned char irq;
    unsigned int gsi;
    unsigned short flags;

};

struct acpi_madt_lapic_nmi
{

    struct acpi_madt_entry base;
    unsigned char processor;
    unsigned short flags;
    unsigned char lint;

};

struct acpi_srat_entry
{

    unsigned char type;
    unsigned char length;

};

struct acpi_srat_apic
{

    struct acpi_srat_entry base;
    char domain;
    char id;
    unsigned int flags;

};

struct acpi_srat
{

    struct acpi_sdth base;
    unsigned int reserved0;
    unsigned int reserved1;
    unsigned int reserved2;

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

struct acpi_sdth *acpi_findheader(char *name);
