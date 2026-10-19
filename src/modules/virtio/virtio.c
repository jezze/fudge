#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/io/io.h>
#include <modules/pci/pci.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>
#include "virtio.h"

#define CAP_VENDOR                      0x09
#define CAP_COMMON                      1
#define CAP_NOTIFY                      2
#define CAP_ISR                         3
#define CAP_DEVICE                      4
#define COMMON_DEVICEFEATURESELECT      0x00
#define COMMON_DEVICEFEATURE            0x04
#define COMMON_DRIVERFEATURESELECT      0x08
#define COMMON_DRIVERFEATURE            0x0C
#define COMMON_CONFIGVECTOR             0x10
#define COMMON_STATUS                   0x14
#define COMMON_QUEUESELECT              0x16
#define COMMON_QUEUESIZE                0x18
#define COMMON_QUEUEVECTOR              0x1A
#define COMMON_QUEUEENABLE              0x1C
#define COMMON_QUEUENOTIFYOFF           0x1E
#define COMMON_QUEUEDESC                0x20
#define COMMON_QUEUEDRIVER              0x28
#define COMMON_QUEUEDEVICE              0x30
#define FEATURE_VERSION1                (1 << 0)
#define DEVICE_LEGACYNETWORK            0x1000
#define DEVICE_LEGACYBLOCK              0x1001
#define DEVICE_MODERN                   0x1040

static unsigned long pagealign(unsigned long value)
{

    return (value + 0xFFF) & ~0xFFFUL;

}

static unsigned char read8(unsigned long address)
{

    return *(volatile unsigned char *)address;

}

static unsigned short read16(unsigned long address)
{

    return *(volatile unsigned short *)address;

}

static unsigned int read32(unsigned long address)
{

    return *(volatile unsigned int *)address;

}

static void write8(unsigned long address, unsigned char value)
{

    *(volatile unsigned char *)address = value;

}

static void write16(unsigned long address, unsigned short value)
{

    *(volatile unsigned short *)address = value;

}

static void write32(unsigned long address, unsigned int value)
{

    *(volatile unsigned int *)address = value;

}

static void write64(unsigned long address, unsigned long value)
{

    write32(address, value);
    write32(address + 4, (value >> 16) >> 16);

}

static unsigned long mapcapability(unsigned int id, unsigned char offset)
{

    unsigned int bar = pci_inb(id, offset + 4);
    unsigned int start = pci_ind(id, offset + 8);
    unsigned int length = pci_ind(id, offset + 12);
    unsigned long base;
    unsigned long address;
    unsigned long page;

    if (bar > 5)
        return 0;

    /* a 32 bit kernel can't reach a bar the firmware put above 4 GB */
    if (sizeof (unsigned long) == 4 && bar < 5 && (pci_ind(id, PCI_CONFIG_BAR0 + bar * 4) & 0x06) == 0x04 && pci_ind(id, PCI_CONFIG_BAR0 + bar * 4 + 4))
        return 0;

    base = pci_getbar(id, bar);

    if (!base)
        return 0;

    address = base + start;
    page = address & ~0xFFFUL;

    arch_kmap(page, page, pagealign(address + length) - page, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    return address;

}

static unsigned int findmodern(struct virtio_device *device, unsigned int id)
{

    unsigned char offset = pci_inb(id, PCI_CONFIG_CAPABILITIES) & ~3;

    while (offset)
    {

        if (pci_inb(id, offset) == CAP_VENDOR)
        {

            switch (pci_inb(id, offset + 3))
            {

            case CAP_COMMON:
                if (!device->common)
                    device->common = mapcapability(id, offset);

                break;

            case CAP_NOTIFY:
                if (!device->notify)
                {

                    device->notify = mapcapability(id, offset);
                    device->multiplier = pci_ind(id, offset + 16);

                }

                break;

            case CAP_ISR:
                if (!device->isr)
                    device->isr = mapcapability(id, offset);

                break;

            case CAP_DEVICE:
                if (!device->config)
                    device->config = mapcapability(id, offset);

                break;

            }

        }

        offset = pci_inb(id, offset + 1) & ~3;

    }

    return device->common && device->notify && device->isr;

}

unsigned int virtio_match(unsigned int id, unsigned int type)
{

    unsigned short device = pci_inw(id, PCI_CONFIG_DEVICE);

    if (pci_inw(id, PCI_CONFIG_VENDOR) != VIRTIO_VENDOR)
        return 0;

    return device == DEVICE_MODERN + type || (type == VIRTIO_TYPE_NETWORK && device == DEVICE_LEGACYNETWORK) || (type == VIRTIO_TYPE_BLOCK && device == DEVICE_LEGACYBLOCK);

}

void virtio_registerdriver(struct base_driver *driver)
{

    base_registerdriver(driver, PCI_BUS);

}

void virtio_unregisterdriver(struct base_driver *driver)
{

    base_unregisterdriver(driver, PCI_BUS);

}

unsigned int virtio_setup(struct virtio_device *device, unsigned int id)
{

    buffer_clear(device, sizeof (struct virtio_device));
    pci_setmaster(id);

    device->id = id;

    if (pci_ind(id, PCI_CONFIG_BAR0) & 1)
    {

        device->io = pci_getbar(id, 0);

        return device->io != 0;

    }

    device->modern = findmodern(device, id);

    return device->modern;

}

void virtio_reset(struct virtio_device *device)
{

    if (device->modern)
    {

        unsigned int i;

        write8(device->common + COMMON_STATUS, 0);

        for (i = 0; i < 0x100000 && read8(device->common + COMMON_STATUS); i++);

    }

    else
    {

        io_outb(device->io + VIRTIO_REG_STATUS, 0);
        io_inb(device->io + VIRTIO_REG_STATUS);

    }

}

void virtio_setstatus(struct virtio_device *device, unsigned char status)
{

    if (device->modern)
        write8(device->common + COMMON_STATUS, read8(device->common + COMMON_STATUS) | status);
    else
        io_outb(device->io + VIRTIO_REG_STATUS, io_inb(device->io + VIRTIO_REG_STATUS) | status);

}

unsigned int virtio_negotiate(struct virtio_device *device, unsigned int features)
{

    virtio_reset(device);
    virtio_setstatus(device, VIRTIO_STATUS_ACKNOWLEDGE);
    virtio_setstatus(device, VIRTIO_STATUS_DRIVER);

    if (device->modern)
    {

        unsigned long common = device->common;

        write32(common + COMMON_DEVICEFEATURESELECT, 0);

        features &= read32(common + COMMON_DEVICEFEATURE);

        write32(common + COMMON_DEVICEFEATURESELECT, 1);

        if (!(read32(common + COMMON_DEVICEFEATURE) & FEATURE_VERSION1))
            device->failed = 1;

        write32(common + COMMON_DRIVERFEATURESELECT, 0);
        write32(common + COMMON_DRIVERFEATURE, features);
        write32(common + COMMON_DRIVERFEATURESELECT, 1);
        write32(common + COMMON_DRIVERFEATURE, FEATURE_VERSION1);
        virtio_setstatus(device, VIRTIO_STATUS_FEATURESOK);

        if (!(read8(common + COMMON_STATUS) & VIRTIO_STATUS_FEATURESOK))
            device->failed = 1;

    }

    else
    {

        features &= io_ind(device->io + VIRTIO_REG_HOSTFEATURES);

        io_outd(device->io + VIRTIO_REG_GUESTFEATURES, features);

    }

    return (device->failed) ? 0 : features;

}

unsigned char virtio_getisr(struct virtio_device *device)
{

    return (device->modern) ? read8(device->isr) : io_inb(device->io + VIRTIO_REG_ISR);

}

unsigned char virtio_getconfig(struct virtio_device *device, unsigned int offset)
{

    if (device->modern)
        return (device->config) ? read8(device->config + offset) : 0;

    return io_inb(device->io + VIRTIO_REG_CONFIG + offset);

}

void virtio_setconfig(struct virtio_device *device, unsigned int offset, unsigned char value)
{

    if (device->modern)
    {

        if (device->config)
            write8(device->config + offset, value);

    }

    else
    {

        io_outb(device->io + VIRTIO_REG_CONFIG + offset, value);

    }

}

unsigned int virtio_setqueue(struct virtio_queue *queue, struct virtio_device *device, unsigned short index, void *memory)
{

    unsigned short size;

    if (device->failed)
        return 0;

    if (device->modern)
    {

        write16(device->common + COMMON_QUEUESELECT, index);

        size = read16(device->common + COMMON_QUEUESIZE);

        if (size > VIRTIO_QUEUESIZE)
        {

            size = VIRTIO_QUEUESIZE;

            write16(device->common + COMMON_QUEUESIZE, size);

        }

    }

    else
    {

        io_outw(device->io + VIRTIO_REG_QUEUESELECT, index);

        size = io_inw(device->io + VIRTIO_REG_QUEUESIZE);

    }

    if (!virtio_initqueue(queue, device, index, size, memory))
        return 0;

    if (device->modern)
    {

        queue->notifyaddress = device->notify + read16(device->common + COMMON_QUEUENOTIFYOFF) * device->multiplier;

        write64(device->common + COMMON_QUEUEDESC, (unsigned long)queue->descs);
        write64(device->common + COMMON_QUEUEDRIVER, (unsigned long)queue->avail);
        write64(device->common + COMMON_QUEUEDEVICE, (unsigned long)queue->used);
        write16(device->common + COMMON_QUEUEENABLE, 1);

    }

    else
    {

        io_outd(device->io + VIRTIO_REG_QUEUEADDRESS, (unsigned long)queue->descs >> 12);

    }

    return 1;

}

void virtio_notify(struct virtio_queue *queue)
{

    if (queue->device->modern)
        write16(queue->notifyaddress, queue->index);
    else
        io_outw(queue->device->io + VIRTIO_REG_QUEUENOTIFY, queue->index);

}

static unsigned int setvectors(struct virtio_device *device, unsigned int queues)
{

    unsigned short i;

    if (device->modern)
    {

        write16(device->common + COMMON_CONFIGVECTOR, VIRTIO_NOVECTOR);

        for (i = 0; i < queues; i++)
        {

            write16(device->common + COMMON_QUEUESELECT, i);
            write16(device->common + COMMON_QUEUEVECTOR, 0);

            if (read16(device->common + COMMON_QUEUEVECTOR) != 0)
                return 0;

        }

    }

    else
    {

        io_outw(device->io + VIRTIO_REG_CONFIGVECTOR, VIRTIO_NOVECTOR);

        for (i = 0; i < queues; i++)
        {

            io_outw(device->io + VIRTIO_REG_QUEUESELECT, i);
            io_outw(device->io + VIRTIO_REG_QUEUEVECTOR, 0);

            if (io_inw(device->io + VIRTIO_REG_QUEUEVECTOR) != 0)
                return 0;

        }

    }

    return 1;

}

unsigned int virtio_setinterrupt(struct virtio_device *device, unsigned int queues, void (*routine)(unsigned int irq))
{

    unsigned int msi = apic_setmsiroutine(routine);

    if (msi)
    {

        if (pci_setmsix(device->id, 0, apic_getmsiaddress(), apic_getmsidata(msi)) && setvectors(device, queues))
            return msi;

        pci_unsetmsix(device->id);
        apic_unsetroutine(msi, routine);

    }

    pic_setroutine(pci_getirq(device->id), routine);
    apic_setroutine(pci_getirq(device->id), routine);

    return 0;

}

void virtio_unsetinterrupt(struct virtio_device *device, unsigned int msi, void (*routine)(unsigned int irq))
{

    if (msi)
    {

        pci_unsetmsix(device->id);
        apic_unsetroutine(msi, routine);

    }

    else
    {

        pic_unsetroutine(pci_getirq(device->id), routine);
        apic_unsetroutine(pci_getirq(device->id), routine);

    }

}

