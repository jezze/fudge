#ifndef ARCH_PCI_H
#define ARCH_PCI_H

#define PCI_ADDRESS 0xCF8
#define PCI_DATA    0xCFC

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

