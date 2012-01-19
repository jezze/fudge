#ifndef MODULES_ACPI_H
#define MODULES_ACPI_H

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

#endif

