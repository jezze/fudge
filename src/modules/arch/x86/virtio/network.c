#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "virtio.h"

struct header
{

    unsigned short flags;
    unsigned short hlength;
    unsigned short slength;
    unsigned short checksumstart;
    unsigned short checksumoffset;

};

static struct base_driver driver;
static struct ethernet_interface ethernetinterface;
static unsigned short io;
static struct virtio_queue vqs[16];
static unsigned char virtqbuffer[3][0x4000];
static unsigned char rxbuffer[0x4000];
static unsigned char txbuffer[0x4000];

static void reset(void)
{

    io_outb(io + VIRTIO_REGISTERSTATUS, 0);
    io_inb(io + VIRTIO_REGISTERSTATUS);

}

static void handlequeue(struct virtio_queue *vq)
{

    struct virtio_usedring *usedring = &vq->usedring[(vq->usedhead->index - 1) % vq->size];
    struct virtio_buffer *buffer = &vq->buffers[usedring->index];
    struct header *header = (struct header *)buffer->address;

    if (buffer->flags == 2)
    {

        ethernet_notify(&ethernetinterface, header + 1, usedring->length);
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

static unsigned int ethernetinterface_getaddress(void *buffer)
{

    unsigned char *haddress = buffer;

    haddress[0] = io_inb(io + 0x14);
    haddress[1] = io_inb(io + 0x15);
    haddress[2] = io_inb(io + 0x16);
    haddress[3] = io_inb(io + 0x17);
    haddress[4] = io_inb(io + 0x18);
    haddress[5] = io_inb(io + 0x19);

    return ETHERNET_ADDRSIZE;

}

static unsigned int ethernetinterface_send(void *buffer, unsigned int count)
{

    struct virtio_queue *vq = &vqs[1];
    unsigned int bi = vq->numbuffers;
    struct header nheader;

    nheader.flags = 1;
    nheader.checksumstart = 0;
    nheader.checksumoffset = count;

    memory_copy(txbuffer, &nheader, sizeof (struct header));
    memory_copy(txbuffer + sizeof (struct header), buffer, count);

    vq->buffers[bi].address = (unsigned int)&txbuffer;
    vq->buffers[bi].length = sizeof (struct header) + count;
    vq->buffers[bi].flags = 0;
    vq->availablering[vq->availablehead->index % vq->size].index = bi;
    vq->availablehead->index++;
    vq->numbuffers++;

/*
    vq->buffers[1].address = (unsigned int)&buffer;
    vq->buffers[1].length = count;
    vq->buffers[1].flags = 0;
    vq->buffers[1].next = 0;
    vq->availablering[vq->availablehead->index % vq->size].index = 1;
    vq->availablehead->index++;
*/

    io_outw(io + VIRTIO_REGISTERQNOTIFY, 1);

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

static unsigned int ethernetinterface_addrread(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char address[ETHERNET_ADDRSIZE];

    ethernetinterface.getaddress(address);

    return memory_read(buffer, count, address, ETHERNET_ADDRSIZE, offset);

}

static void driver_init(void)
{

    ethernet_initinterface(&ethernetinterface, ethernetinterface_getaddress, ethernetinterface_send);

    ethernetinterface.addr.read = ethernetinterface_addrread;

}

static unsigned int driver_match(unsigned int id)
{

    char revision = pci_inb(id, PCI_CONFIG_REVISION);

    switch (revision)
    {

    case 0:
        return pci_inw(id, PCI_CONFIG_VENDOR) == VIRTIO_PCIVENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == 0x1000;

    case 1:
        return pci_inw(id, PCI_CONFIG_VENDOR) == VIRTIO_PCIVENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == 0x1041;

    }

    return 0;

}

static void driver_reset(unsigned int id)
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

}

static void driver_attach(unsigned int id)
{

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

    base_initdriver(&driver, "virtio-network", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

