#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/block/block.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "virtio.h"

static struct base_driver driver;
static struct block_interface blockinterface;
static unsigned short io;
static struct virtio_queue vqs[16];
static unsigned char virtqbuffer[3][0x4000];
static unsigned char rxbuffer[0x4000];

static void reset(void)
{

    io_outb(io + VIRTIO_REGISTERSTATUS, 0);
    io_inb(io + VIRTIO_REGISTERSTATUS);

}

static void handlequeue(struct virtio_queue *vq)
{

    struct virtio_usedring *usedring = &vq->usedring[(vq->usedhead->index - 1) % vq->size];
    struct virtio_buffer *buffer = &vq->buffers[usedring->index];

    if (buffer->flags == 2)
    {

        /* NOTIFY */
        vq->availablehead->index = 0;

    }

    vq->lastindex = vq->usedhead->index;

}

static void handleirq(unsigned int irq)
{

    unsigned char status = io_inb(io + VIRTIO_REGISTERISR);

    if (status & 1)
    {

        unsigned int i;

        for (i = 0; i < 3; i++)
        {

            struct virtio_queue *vq = &vqs[i];

            if (vq->lastindex != vq->usedhead->index)
                handlequeue(vq);

        }

    }

}

static unsigned int blockinterface_rdata(void *buffer, unsigned int count, unsigned int offset)
{

    return count;

}

static unsigned int blockinterface_wdata(void *buffer, unsigned int count, unsigned int offset)
{

    return count;

}

static void setqueue(unsigned short index)
{

    struct virtio_queue *vq = &vqs[index];

    io_outw(io + VIRTIO_REGISTERQSELECT, index);

    vq->size = io_inw(io + VIRTIO_REGISTERQSIZE);

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

    io_outd(io + VIRTIO_REGISTERQADDRESS, vq->address >> 12);

}

static void setqueues(void)
{

    unsigned short i;

    for (i = 0; i < 16; i++)
        setqueue(i);

}

static void setfeatures(void)
{

    unsigned int features = io_ind(io + VIRTIO_REGISTERDEVFEATURES);

    /* MODIFY FEATURES */

    io_outd(io + VIRTIO_REGISTERGUESTFEATURES, features);

}

static void setrx(void)
{

    struct virtio_queue *vq = &vqs[0];
    unsigned int bi = vq->numbuffers;

    vq->buffers[bi].address = (unsigned int)&rxbuffer;
    vq->buffers[bi].length = 0x4000;
    vq->buffers[bi].flags = 2;
    vq->availablering[vq->availablehead->index % vq->size].index = bi;
    vq->availablehead->index++;
    vq->numbuffers++;

    io_outw(io + VIRTIO_REGISTERQNOTIFY, 0);

}

static void driver_init(void)
{

    block_initinterface(&blockinterface, blockinterface_rdata, blockinterface_wdata);

}

static unsigned int driver_match(unsigned int id)
{

    char revision = pci_inb(id, PCI_CONFIG_REVISION);

    switch (revision)
    {

    case 0:
        return pci_inw(id, PCI_CONFIG_VENDOR) == VIRTIO_PCIVENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == 0x1001;

    case 1:
        return pci_inw(id, PCI_CONFIG_VENDOR) == VIRTIO_PCIVENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == 0x1042;

    }

    return 0;

}

static void driver_attach(unsigned int id)
{

    unsigned char status;

    io = pci_inw(id, PCI_CONFIG_BAR0) & ~1;

    reset();
    io_outb(io + VIRTIO_REGISTERSTATUS, VIRTIO_STATUSACKNOWLEDGE);
    io_outb(io + VIRTIO_REGISTERSTATUS, VIRTIO_STATUSACKNOWLEDGE | VIRTIO_STATUSDRIVER);
    setfeatures();
    io_outb(io + VIRTIO_REGISTERSTATUS, VIRTIO_STATUSACKNOWLEDGE | VIRTIO_STATUSDRIVER | VIRTIO_STATUSFEATURES);

    status = io_inb(io + VIRTIO_REGISTERSTATUS);

    if (!(status & VIRTIO_STATUSFEATURES))
        return;

    setqueues();
    setrx();
    io_outb(io + VIRTIO_REGISTERSTATUS, VIRTIO_STATUSACKNOWLEDGE | VIRTIO_STATUSDRIVER | VIRTIO_STATUSFEATURES | VIRTIO_STATUSREADY);
    pci_setmaster(id);
    block_registerinterface(&blockinterface, id);
    pic_setroutine(pci_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(pci_getirq(id));

}

void module_init(void)
{

    base_initdriver(&driver, "virtio-block", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

