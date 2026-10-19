#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/virt/virt.h>
#include "../virtio.h"

/* virtio-mmio: device->common is the register base, device->modern means version 2 */
#define REG_MAGIC                       0x000
#define REG_MAGIC_VALUE                 0x74726976
#define REG_VERSION                     0x004
#define REG_DEVICEID                    0x008
#define REG_DEVICEFEATURES              0x010
#define REG_DEVICEFEATURESSELECT        0x014
#define REG_DRIVERFEATURES              0x020
#define REG_DRIVERFEATURESSELECT        0x024
#define REG_GUESTPAGESIZE               0x028
#define REG_QUEUESELECT                 0x030
#define REG_QUEUESIZEMAX                0x034
#define REG_QUEUESIZE                   0x038
#define REG_QUEUEALIGN                  0x03C
#define REG_QUEUEPFN                    0x040
#define REG_QUEUEREADY                  0x044
#define REG_QUEUENOTIFY                 0x050
#define REG_INTERRUPTSTATUS             0x060
#define REG_INTERRUPTACK                0x064
#define REG_STATUS                      0x070
#define REG_QUEUEDESC                   0x080
#define REG_QUEUEDRIVER                 0x090
#define REG_QUEUEDEVICE                 0x0A0
#define REG_CONFIG                      0x100
#define FEATURE_VERSION1                (1 << 0)
#define PAGESIZE                        0x1000

/* the registers are little-endian whatever the cpu is */
static unsigned int read32(unsigned long base, unsigned int offset)
{

    return virtio_le32(*(volatile unsigned int *)(base + offset));

}

static void write32(unsigned long base, unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(base + offset) = virtio_le32(value);

}

static unsigned int readreg(struct virtio_device *device, unsigned int offset)
{

    return read32(device->common, offset);

}

static void writereg(struct virtio_device *device, unsigned int offset, unsigned int value)
{

    write32(device->common, offset, value);

}

static void write64(struct virtio_device *device, unsigned int offset, unsigned long value)
{

    writereg(device, offset, value);
    writereg(device, offset + 4, (value >> 16) >> 16);

}

/* only mapped slots with a device behind them match */
unsigned int virtio_match(unsigned int id, unsigned int type)
{

    unsigned long base = virt_getbase(id);

    if (id < VIRT_VIRTIO || !base)
        return 0;

    arch_kmap(base, base, PAGESIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    return read32(base, REG_MAGIC) == REG_MAGIC_VALUE && read32(base, REG_DEVICEID) == type;

}

void virtio_registerdriver(struct base_driver *driver)
{

    base_registerdriver(driver, VIRT_BUS);

}

void virtio_unregisterdriver(struct base_driver *driver)
{

    base_unregisterdriver(driver, VIRT_BUS);

}

unsigned int virtio_setup(struct virtio_device *device, unsigned int id)
{

    unsigned int version;

    buffer_clear(device, sizeof (struct virtio_device));

    device->id = id;
    device->common = virt_getbase(id);

    version = readreg(device, REG_VERSION);
    device->modern = (version == 2);

    /* a legacy device keeps its rings in the cpu's byte order, which only matches the little-endian rings on little-endian cpus */
    return version == 2 || (version == 1 && virtio_le16(1) == 1);

}

void virtio_reset(struct virtio_device *device)
{

    writereg(device, REG_STATUS, 0);

}

void virtio_setstatus(struct virtio_device *device, unsigned char status)
{

    writereg(device, REG_STATUS, readreg(device, REG_STATUS) | status);

}

unsigned int virtio_negotiate(struct virtio_device *device, unsigned int features)
{

    virtio_reset(device);
    virtio_setstatus(device, VIRTIO_STATUS_ACKNOWLEDGE);
    virtio_setstatus(device, VIRTIO_STATUS_DRIVER);

    writereg(device, REG_DEVICEFEATURESSELECT, 0);

    features &= readreg(device, REG_DEVICEFEATURES);

    writereg(device, REG_DRIVERFEATURESSELECT, 0);
    writereg(device, REG_DRIVERFEATURES, features);

    if (device->modern)
    {

        writereg(device, REG_DEVICEFEATURESSELECT, 1);
        writereg(device, REG_DRIVERFEATURESSELECT, 1);
        writereg(device, REG_DRIVERFEATURES, readreg(device, REG_DEVICEFEATURES) & FEATURE_VERSION1);

        virtio_setstatus(device, VIRTIO_STATUS_FEATURESOK);

        if (!(readreg(device, REG_STATUS) & VIRTIO_STATUS_FEATURESOK))
        {

            device->failed = 1;

            virtio_setstatus(device, VIRTIO_STATUS_FAILED);

        }

    }

    else
    {

        writereg(device, REG_GUESTPAGESIZE, PAGESIZE);

    }

    return features;

}

unsigned char virtio_getisr(struct virtio_device *device)
{

    unsigned int status = readreg(device, REG_INTERRUPTSTATUS);

    writereg(device, REG_INTERRUPTACK, status);

    return status;

}

unsigned char virtio_getconfig(struct virtio_device *device, unsigned int offset)
{

    return *(volatile unsigned char *)(device->common + REG_CONFIG + offset);

}

void virtio_setconfig(struct virtio_device *device, unsigned int offset, unsigned char value)
{

    *(volatile unsigned char *)(device->common + REG_CONFIG + offset) = value;

}

unsigned int virtio_setqueue(struct virtio_queue *queue, struct virtio_device *device, unsigned short index, void *memory)
{

    unsigned int size;

    if (device->failed)
        return 0;

    writereg(device, REG_QUEUESELECT, index);

    size = readreg(device, REG_QUEUESIZEMAX);

    if (size > VIRTIO_QUEUESIZE)
        size = VIRTIO_QUEUESIZE;

    if (!virtio_initqueue(queue, device, index, size, memory))
        return 0;

    writereg(device, REG_QUEUESIZE, size);

    if (device->modern)
    {

        write64(device, REG_QUEUEDESC, (unsigned long)queue->descs);
        write64(device, REG_QUEUEDRIVER, (unsigned long)queue->avail);
        write64(device, REG_QUEUEDEVICE, (unsigned long)queue->used);

        writereg(device, REG_QUEUEREADY, 1);

    }

    else
    {

        writereg(device, REG_QUEUEALIGN, PAGESIZE);
        writereg(device, REG_QUEUEPFN, (unsigned long)queue->descs / PAGESIZE);

    }

    return 1;

}

void virtio_notify(struct virtio_queue *queue)
{

    writereg(queue->device, REG_QUEUENOTIFY, queue->index);

}

/* the slot's line on the PLIC; there is no MSI, so this returns 0 like a PCI device without MSI-X */
unsigned int virtio_setinterrupt(struct virtio_device *device, unsigned int queues, void (*routine)(unsigned int irq))
{

    virt_setroutine(virt_getirq(device->id), routine);

    return 0;

}

void virtio_unsetinterrupt(struct virtio_device *device, unsigned int msi, void (*routine)(unsigned int irq))
{

    virt_unsetroutine(virt_getirq(device->id), routine);

}

