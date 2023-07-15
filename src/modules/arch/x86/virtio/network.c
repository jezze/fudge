#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>
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
#if 0
static unsigned char txbuffer[0x4000];
#endif

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

    unsigned char status = io_inb(io + VIRTIO_REG_ISR);

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

#if 0
static unsigned int ethernetinterface_send(void *buffer, unsigned int count)
{

    struct virtio_queue *vq = &vqs[1];
    unsigned int bi = vq->numbuffers;
    struct header nheader;

    nheader.flags = 1;
    nheader.checksumstart = 0;
    nheader.checksumoffset = count;

    buffer_copy(txbuffer, &nheader, sizeof (struct header));
    buffer_copy(txbuffer + sizeof (struct header), buffer, count);

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

    io_outw(io + VIRTIO_REG_QNOTIFY, 1);

    return count;

}
#endif

static void setqueues(void)
{

    unsigned short i;

    for (i = 0; i < 16; i++)
        virtio_setqueue(io, i, &vqs[i], (unsigned int)virtqbuffer[i]);

}

static void setfeatures(void)
{

    unsigned int features = io_ind(io + VIRTIO_REG_DEVFEATURES);

    /* MODIFY FEATURES */

    io_outd(io + VIRTIO_REG_GUESTFEATURES, features);

}

static unsigned int ethernetinterface_readaddr(void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char address[ETHERNET_ADDRSIZE];

    address[0] = io_inb(io + 0x14);
    address[1] = io_inb(io + 0x15);
    address[2] = io_inb(io + 0x16);
    address[3] = io_inb(io + 0x17);
    address[4] = io_inb(io + 0x18);
    address[5] = io_inb(io + 0x19);

    return buffer_read(buffer, count, address, ETHERNET_ADDRSIZE, offset);

}

static void driver_init(unsigned int id)
{

    ethernet_initinterface(&ethernetinterface, id);

    ethernetinterface.addr.operations.read = ethernetinterface_readaddr;

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

    virtio_reset(io);
    io_outb(io + VIRTIO_REG_STATUS, VIRTIO_REG_STATUS_ACKNOWLEDGE);
    io_outb(io + VIRTIO_REG_STATUS, VIRTIO_REG_STATUS_ACKNOWLEDGE | VIRTIO_REG_STATUS_DRIVER);
    setfeatures();
    io_outb(io + VIRTIO_REG_STATUS, VIRTIO_REG_STATUS_ACKNOWLEDGE | VIRTIO_REG_STATUS_DRIVER | VIRTIO_REG_STATUS_FEATURES);

    status = io_inb(io + VIRTIO_REG_STATUS);

    if (!(status & VIRTIO_REG_STATUS_FEATURES))
        return;

    setqueues();
    virtio_setrx(io, &vqs[0], rxbuffer);
    io_outb(io + VIRTIO_REG_STATUS, VIRTIO_REG_STATUS_ACKNOWLEDGE | VIRTIO_REG_STATUS_DRIVER | VIRTIO_REG_STATUS_FEATURES | VIRTIO_REG_STATUS_READY);
    pci_setmaster(id);

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = pci_getirq(id);

    ethernet_registerinterface(&ethernetinterface);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = pci_getirq(id);

    ethernet_unregisterinterface(&ethernetinterface);
    pic_unsetroutine(irq);
    apic_unsetroutine(irq);

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

