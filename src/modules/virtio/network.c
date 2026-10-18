#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/ethernet.h>
#include "virtio.h"

#define FEATURE_MAC                     (1 << 5)
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
static struct virtio_device device;
static unsigned int headersize;
static unsigned int ready;
static unsigned int msi;
static unsigned int txnext;
static unsigned int txpending;

static void handleirq(unsigned int irq)
{

    if (!msi)
        virtio_getisr(&device);

    while (virtio_pending(&rxqueue))
    {

        unsigned int length;
        unsigned int id = virtio_pop(&rxqueue, &length);

        if (id >= RXBUFFERS)
            continue;

        if (length > headersize)
            ethernet_notifydata(&ethernetinterface, rxbuffers[id] + headersize, length - headersize);

        virtio_push(&rxqueue, id);

    }

    virtio_notify(&rxqueue);

}

static void ethernetinterface_getmac(unsigned char address[6])
{

    buffer_copy(address, mac, 6);

}

static unsigned int ethernetinterface_send(void *buffer, unsigned int count)
{

    unsigned int length;

    if (!ready || count > BUFFERSIZE - headersize)
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

    buffer_clear(txbuffers[txnext], headersize);
    buffer_copy(txbuffers[txnext] + headersize, buffer, count);
    virtio_setdesc(&txqueue, txnext, txbuffers[txnext], headersize + count, 0, 0);
    virtio_push(&txqueue, txnext);
    virtio_notify(&txqueue);

    txnext = (txnext + 1) % TXBUFFERS;
    txpending++;

    spinlock_release(&spinlock);

    return count;

}

static void driver_init(unsigned int id)
{

    ethernet_initinterface(&ethernetinterface, id, ethernetinterface_getmac, ethernetinterface_send);

}

static unsigned int driver_match(unsigned int id)
{

    return virtio_match(id, VIRTIO_TYPE_NETWORK);

}

static void driver_reset(unsigned int id)
{

    unsigned int i;

    if (!virtio_setup(&device, id))
        return;

    headersize = (device.modern) ? 12 : 10;

    if (!(virtio_negotiate(&device, FEATURE_MAC) & FEATURE_MAC))
        return;

    for (i = 0; i < 6; i++)
        mac[i] = virtio_getconfig(&device, i);

    if (!virtio_setqueue(&rxqueue, &device, QUEUE_RX, rxmemory) || rxqueue.size < RXBUFFERS)
        return;

    if (!virtio_setqueue(&txqueue, &device, QUEUE_TX, txmemory) || txqueue.size < TXBUFFERS)
        return;

    for (i = 0; i < RXBUFFERS; i++)
    {

        virtio_setdesc(&rxqueue, i, rxbuffers[i], BUFFERSIZE, VIRTIO_DESC_WRITE, 0);
        virtio_push(&rxqueue, i);

    }

    ready = 1;

}

static void driver_attach(unsigned int id)
{

    if (!ready)
        return;

    msi = virtio_setinterrupt(&device, 2, handleirq);

    virtio_setstatus(&device, VIRTIO_STATUS_DRIVEROK);
    virtio_notify(&rxqueue);
    ethernet_registerinterface(&ethernetinterface);

}

static void driver_detach(unsigned int id)
{

    if (!ready)
        return;

    ethernet_unregisterinterface(&ethernetinterface);
    virtio_reset(&device);
    virtio_unsetinterrupt(&device, msi, handleirq);

    ready = 0;

}

void module_init(void)
{

    base_initdriver(&driver, "virtio-network", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    virtio_registerdriver(&driver);

}

void module_unregister(void)
{

    virtio_unregisterdriver(&driver);

}

