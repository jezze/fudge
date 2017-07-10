#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "virtio.h"

#define PCIVENDOR                       0x1AF4
#define TYPENETWORK                     1
#define TYPEBLOCK                       2
#define TYPETERMINAL                    3
#define TYPEENTROPY                     4
#define TYPEBALLOON                     5
#define TYPEIOMEM                       6
#define TYPERPMSG                       7
#define TYPESCSI                        8
#define TYPE9P                          9
#define TYPEWLAN                        10
#define REGISTERDEVFEATURES             0x00
#define REGISTERGUESTFEATURES           0x04
#define REGISTERQADDRESS                0x08
#define REGISTERQSIZE                   0x0C
#define REGISTERQSELECT                 0x0E
#define REGISTERQNOTIFY                 0x10
#define REGISTERSTATUS                  0x12
#define REGISTERISR                     0x13
#define VIRTIO_ACKNOWLEDGE              0x01
#define VIRTIO_DRIVER                   0x02
#define VIRTIO_READY                    0x04
#define VIRTIO_FEATURES                 0x08

struct netheader
{

    unsigned char flags;
    unsigned char segmentation;
    unsigned short hlength;
    unsigned short slength;
    unsigned short checksumstart;
    unsigned short checksumoffset;
    unsigned short buffercount;

};

struct virtqueue
{

    unsigned short size;
    unsigned int address;
    struct virtio_buffer *buffers;
    unsigned int bufferssize;
    struct virtio_availablehead *availablehead;
    struct virtio_availablering *availablering;
    unsigned int availablesize;
    struct virtio_usedhead *usedhead;
    struct virtio_usedring *usedring;
    unsigned int usedsize;

};

static struct base_driver driver;
static struct ethernet_interface ethernetinterface;
static unsigned short io;
static char virtqbuffer[3][0x4000];
static struct virtqueue vqs[16];
static char rxbuffer[0x4000];
static struct netheader nheader;

static void reset(void)
{

    io_outb(io + REGISTERSTATUS, 0);
    io_inb(io + REGISTERSTATUS);

}

static void handleirq(unsigned int irq)
{

    unsigned char status = io_inb(io + REGISTERISR);

    debug_write(DEBUG_INFO, "VIRTIO", "INTERRUPT", status);

    if (status & 1)
    {

        struct virtqueue *vq = &vqs[0];

        vq->availablehead->index = 0;

    }

}

static unsigned int ethernetinterface_send(void *buffer, unsigned int count)
{

    struct virtqueue *vq = &vqs[1];

    nheader.flags = 1;
    nheader.checksumstart = 0;
    nheader.checksumoffset = count;

    vq->buffers[0].address = (unsigned int)&nheader;
    vq->buffers[0].length = sizeof (struct netheader);
    vq->buffers[0].flags = 1;
    vq->buffers[0].next = 1;
    vq->availablering[0].index = 0;
    vq->availablehead->index++;
    vq->buffers[1].address = (unsigned int)&buffer;
    vq->buffers[1].length = count;
    vq->buffers[1].flags = 0;
    vq->buffers[1].next = 0;
    vq->availablering[1].index = 1;
    vq->availablehead->index++;

    io_outw(io + REGISTERQNOTIFY, 1);
    debug_write(DEBUG_INFO, "VIRTIO", "TX", 0);

    return count;

}

static void setqueue(unsigned short index)
{

    struct virtqueue *vq = &vqs[index];

    io_outw(io + REGISTERQSELECT, index);

    vq->size = io_inw(io + REGISTERQSIZE);

    if (!vq->size)
        return;

    vq->address = memory_pagealign((unsigned int)virtqbuffer[index]);
    vq->buffers = (struct virtio_buffer *)vq->address;
    vq->bufferssize = 16 * vq->size;
    vq->availablehead = (struct virtio_availablehead *)(vq->address + vq->bufferssize);
    vq->availablering = (struct virtio_availablering *)(vq->availablehead + 1);
    vq->availablesize = 6 + 2 * vq->size;
    vq->usedhead = (struct virtio_usedhead *)(vq->address + memory_pagealign(vq->bufferssize + vq->availablesize));
    vq->usedring = (struct virtio_usedring *)(vq->usedhead + 1);
    vq->usedsize = 6 + 8 * vq->size;

    io_outd(io + REGISTERQADDRESS, vq->address >> 12);
    debug_write(DEBUG_INFO, "VIRTIO", "QUEUESIZE", vq->size);
    debug_write(DEBUG_INFO, "VIRTIO", "QUEUEORIG", (unsigned int)virtqbuffer[index]);
    debug_write(DEBUG_INFO, "VIRTIO", "QUEUEADDR", vq->address);
    debug_write(DEBUG_INFO, "VIRTIO", "PAGEALIGN", memory_pagealign(vq->bufferssize + vq->availablesize) + memory_pagealign(vq->usedsize));
    debug_write(DEBUG_INFO, "VIRTIO", "PAGECOUNT", memory_pagecount(vq->bufferssize + vq->availablesize) + memory_pagecount(vq->usedsize));

}

static void setqueues()
{

    unsigned short i;

    for (i = 0; i < 16; i++)
        setqueue(i);

}

static void setfeatures()
{

    unsigned int features = io_ind(io + REGISTERDEVFEATURES);

    debug_write(DEBUG_INFO, "VIRTIO", "FEATURES", features);

    /* MODIFY FEATURES */

    io_outd(io + REGISTERGUESTFEATURES, features);

}

static void setrx(void)
{

    struct virtqueue *vq = &vqs[0];

    vq->buffers[0].address = (unsigned int)&rxbuffer;
    vq->buffers[0].length = 0x4000;
    vq->buffers[0].flags = 2;
    vq->buffers[0].next = 0;
    vq->availablering[0].index = 0;
    vq->availablehead->index++;

    io_outw(io + REGISTERQNOTIFY, 0);

}

static void driver_init(void)
{

    ethernet_initinterface(&ethernetinterface, ethernetinterface_send);

}

static unsigned int driver_match(unsigned int id)
{

    char revision = pci_inb(id, PCI_CONFIG_REVISION);

    switch (revision)
    {

    case 0:
        return pci_inw(id, PCI_CONFIG_VENDOR) == PCIVENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == 0x1000;

    case 1:
        return pci_inw(id, PCI_CONFIG_VENDOR) == PCIVENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == 0x1041;

    }

    return 0;

}

static void driver_attach(unsigned int id)
{

    unsigned char status;

    io = pci_inw(id, PCI_CONFIG_BAR0) & ~1;

    reset();
    io_outb(io + REGISTERSTATUS, VIRTIO_ACKNOWLEDGE);
    io_outb(io + REGISTERSTATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER);
    setfeatures();
    io_outb(io + REGISTERSTATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER | VIRTIO_FEATURES);

    status = io_inb(io + REGISTERSTATUS);

    if (!(status & VIRTIO_FEATURES))
        return;

    setqueues();
    setrx();

    ethernetinterface.haddress[0] = io_inb(io + 0x14);
    ethernetinterface.haddress[1] = io_inb(io + 0x15);
    ethernetinterface.haddress[2] = io_inb(io + 0x16);
    ethernetinterface.haddress[3] = io_inb(io + 0x17);
    ethernetinterface.haddress[4] = io_inb(io + 0x18);
    ethernetinterface.haddress[5] = io_inb(io + 0x19);

    io_outb(io + REGISTERSTATUS, VIRTIO_ACKNOWLEDGE | VIRTIO_DRIVER | VIRTIO_FEATURES | VIRTIO_READY);
    pci_setmaster(id);
    ethernet_registerinterface(&ethernetinterface, id);
    pic_setroutine(pci_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    ethernet_unregisterinterface(&ethernetinterface);
    pic_unsetroutine(pci_getirq(id));

}

void module_init(void)
{

    base_initdriver(&driver, "virtio", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

