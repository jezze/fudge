#ifndef ARCH_PCI_H
#define ARCH_PCI_H

#define PCI_ADDRESS 0xCF8
#define PCI_DATA    0xCFC

#define PCI_CLASS_UNKNOWN   0x00
#define PCI_CLASS_STORAGE   0x01
#define PCI_CLASS_NETWORK   0x02
#define PCI_CLASS_DISPLAY   0x03
#define PCI_CLASS_MEDIA     0x04
#define PCI_CLASS_MEMORY    0x05
#define PCI_CLASS_BRIDGE    0x06
#define PCI_CLASS_SIMPLE    0x07
#define PCI_CLASS_BASE      0x08
#define PCI_CLASS_INPUT     0x09
#define PCI_CLASS_DOCK      0x0A
#define PCI_CLASS_PROCESSOR 0x0B
#define PCI_CLASS_SERIAL    0x0C
#define PCI_CLASS_WIRELESS  0x0D
#define PCI_CLASS_IO        0x0E
#define PCI_CLASS_SATELLITE 0x0F
#define PCI_CLASS_CRYPT     0x10
#define PCI_CLASS_SIGNAL    0x11
#define PCI_CLASS_OTHER     0xFF

struct pci_device
{

    struct modules_device base;
    unsigned short vendor;
    unsigned short device;
    unsigned short command;
    unsigned short status;
    unsigned char revision;
    unsigned char interface;
    unsigned char subclass;
    unsigned char classcode;
    unsigned char cachelinesize;
    unsigned char latencytimer;
    unsigned char headertype;
    unsigned char bist;

};

extern void pci_init();

#endif

