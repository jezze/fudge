#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/ethernet.h>
#include <modules/io/io.h>
#include <modules/pci/pci.h>
#include "virtio.h"

#define DEVICE                          0x1000
#define FEATURE_MAC                     (1 << 5)
#define HEADERSIZE                      10
#define BUFFERSIZE                      2048
#define RXBUFFERS                       16
#define TXBUFFERS                       16
#define QUEUE_RX                        0
#define QUEUE_TX                        1

static struct base_driver driver;
static struct ethernet_interface ethernetinterface;
static struct spinlock spinlock;
static struct virtio_queue rxqueue;
static struct virtio_queue txqueue;
static unsigned char rxmemory[VIRTIO_QUEUEMEMORY];
static unsigned char txmemory[VIRTIO_QUEUEMEMORY];
static unsigned char rxbuffers[RXBUFFERS][BUFFERSIZE];
static unsigned char txbuffers[TXBUFFERS][BUFFERSIZE];
static unsigned char mac[6];
static unsigned short io;
static unsigned int ready;
static unsigned int msi;
static unsigned int txnext;
static unsigned int txpending;

static void handleirq(unsigned int irq)
{

    if (!msi)
        io_inb(io + VIRTIO_REG_ISR);

    while (virtio_pending(&rxqueue))
    {

        unsigned int length;
        unsigned int id = virtio_pop(&rxqueue, &length);

        if (id >= RXBUFFERS)
            continue;

        if (length > HEADERSIZE)
            ethernet_notifydata(&ethernetinterface, rxbuffers[id] + HEADERSIZE, length - HEADERSIZE);

        virtio_push(&rxqueue, id);

    }

    virtio_notify(&rxqueue);

}

static void ethernetinterface_oninfo(struct event_ethernetinfo *ethernetinfo)
{

    buffer_copy(ethernetinfo->address, mac, 6);

}

static unsigned int ethernetinterface_ondata(void *buffer, unsigned int count)
{

    unsigned int length;

    if (!ready || count > BUFFERSIZE - HEADERSIZE)
        return 0;

    spinlock_acquire(&spinlock);

    while (virtio_pending(&txqueue))
    {

        virtio_pop(&txqueue, &length);

        txpending--;

    }

    if (txpending == TXBUFFERS)
    {

        spinlock_release(&spinlock);

        return 0;

    }

    buffer_clear(txbuffers[txnext], HEADERSIZE);
    buffer_copy(txbuffers[txnext] + HEADERSIZE, buffer, count);
    virtio_setdesc(&txqueue, txnext, txbuffers[txnext], HEADERSIZE + count, 0, 0);
    virtio_push(&txqueue, txnext);
    virtio_notify(&txqueue);

    txnext = (txnext + 1) % TXBUFFERS;
    txpending++;

    spinlock_release(&spinlock);

    return count;

}

static void driver_init(unsigned int id)
{

    ethernet_initinterface(&ethernetinterface, id, ethernetinterface_oninfo, ethernetinterface_ondata);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == VIRTIO_VENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == DEVICE;

}

static void driver_reset(unsigned int id)
{

    unsigned int i;

    io = pci_getbar(id, 0);

    if (!(virtio_negotiate(io, FEATURE_MAC) & FEATURE_MAC))
        return;

    for (i = 0; i < 6; i++)
        mac[i] = io_inb(io + VIRTIO_REG_CONFIG + i);

    if (!virtio_setqueue(&rxqueue, io, QUEUE_RX, rxmemory) || rxqueue.size < RXBUFFERS)
        return;

    if (!virtio_setqueue(&txqueue, io, QUEUE_TX, txmemory) || txqueue.size < TXBUFFERS)
        return;

    for (i = 0; i < RXBUFFERS; i++)
    {

        virtio_setdesc(&rxqueue, i, rxbuffers[i], BUFFERSIZE, VIRTIO_DESC_WRITE, 0);
        virtio_push(&rxqueue, i);

    }

    pci_setmaster(id);

    ready = 1;

}

static void driver_attach(unsigned int id)
{

    if (!ready)
        return;

    msi = virtio_setinterrupt(id, io, 2, handleirq);

    virtio_setstatus(io, VIRTIO_STATUS_DRIVEROK);
    virtio_notify(&rxqueue);
    ethernet_registerinterface(&ethernetinterface);

}

static void driver_detach(unsigned int id)
{

    if (!ready)
        return;

    ethernet_unregisterinterface(&ethernetinterface);
    virtio_reset(io);
    virtio_unsetinterrupt(id, msi);

    ready = 0;

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

