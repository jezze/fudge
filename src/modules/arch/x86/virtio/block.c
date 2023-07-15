#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/block/block.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "virtio.h"

static struct base_driver driver;
static struct block_interface blockinterface;
static unsigned short io;
static struct virtio_queue vqs[16];
static unsigned char virtqbuffer[3][0x4000];
static unsigned char rxbuffer[0x4000];

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

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id);

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

    block_registerinterface(&blockinterface);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = pci_getirq(id);

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(irq);
    apic_unsetroutine(irq);

}

void module_init(void)
{

    base_initdriver(&driver, "virtio-block", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

