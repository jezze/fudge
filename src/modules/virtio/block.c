#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/block.h>
#include <modules/pci/pci.h>
#include "virtio.h"

#define DEVICE                          0x1001
#define DEVICEMODERN                    0x1042
#define BLOCKSIZE                       512
#define REQUEST_READ                    0
#define REQUEST_WRITE                   1
#define FEATURE_RO                      (1 << 5)

struct request
{

    unsigned int type;
    unsigned int reserved;
    unsigned int sectorlow;
    unsigned int sectorhigh;

};

static struct base_driver driver;
static struct block_interface blockinterface;
static struct spinlock spinlock;
static struct virtio_queue queue;
static unsigned char memory[VIRTIO_QUEUEMEMORY];
static struct request request;
static volatile unsigned char status;
static struct virtio_device device;
static unsigned int ready;
static unsigned int msi;
static unsigned int readonly;

static void handleirq(unsigned int irq)
{

    struct block_session *session = block_getsession(&blockinterface);

    if (!msi)
        virtio_getisr(&device);

    spinlock_acquire(&spinlock);

    while (virtio_pending(&queue))
    {

        unsigned int length;

        virtio_pop(&queue, &length);

        if (session->source)
        {

            if (status)
                session->count = 0;

            block_session_done(&blockinterface, session);

        }

    }

    spinlock_release(&spinlock);

}

static void blockinterface_startsession(struct block_session *session)
{

    void *buffer = (void *)kernel_getbuffer(session->source);
    unsigned int count = (session->count > TASK_BUFFERSIZE) ? TASK_BUFFERSIZE : session->count;

    count -= count % BLOCKSIZE;

    if (!buffer || !count || (readonly && session->type == BLOCK_TYPE_WRITE))
    {

        session->count = 0;

        block_session_done(&blockinterface, session);

        return;

    }

    spinlock_acquire(&spinlock);

    session->count = count;
    request.type = (session->type == BLOCK_TYPE_WRITE) ? REQUEST_WRITE : REQUEST_READ;
    request.sectorlow = session->start / BLOCKSIZE;
    request.sectorhigh = 0;
    status = 0xFF;

    virtio_setdesc(&queue, 0, &request, sizeof (struct request), VIRTIO_DESC_NEXT, 1);
    virtio_setdesc(&queue, 1, buffer, count, VIRTIO_DESC_NEXT | ((session->type == BLOCK_TYPE_WRITE) ? 0 : VIRTIO_DESC_WRITE), 2);
    virtio_setdesc(&queue, 2, (void *)&status, 1, VIRTIO_DESC_WRITE, 0);
    virtio_push(&queue, 0);
    virtio_notify(&queue);
    spinlock_release(&spinlock);

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id, BLOCKSIZE, blockinterface_startsession);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == VIRTIO_VENDOR && (pci_inw(id, PCI_CONFIG_DEVICE) == DEVICE || pci_inw(id, PCI_CONFIG_DEVICE) == DEVICEMODERN);

}

static void driver_reset(unsigned int id)
{

    if (!virtio_setup(&device, id))
        return;

    readonly = virtio_negotiate(&device, FEATURE_RO) & FEATURE_RO;
    blockinterface.blockcount = (virtio_getconfig(&device, 4) || virtio_getconfig(&device, 5) || virtio_getconfig(&device, 6) || virtio_getconfig(&device, 7)) ? 0xFFFFFFFF : virtio_getconfig(&device, 0) | (virtio_getconfig(&device, 1) << 8) | (virtio_getconfig(&device, 2) << 16) | ((unsigned int)virtio_getconfig(&device, 3) << 24);

    if (!virtio_setqueue(&queue, &device, 0, memory) || queue.size < 3)
        return;

    pci_setmaster(id);

    ready = 1;

}

static void driver_attach(unsigned int id)
{

    if (!ready)
        return;

    msi = virtio_setinterrupt(&device, 1, handleirq);

    virtio_setstatus(&device, VIRTIO_STATUS_DRIVEROK);
    block_registerinterface(&blockinterface);

}

static void driver_detach(unsigned int id)
{

    if (!ready)
        return;

    block_unregisterinterface(&blockinterface);
    virtio_reset(&device);
    virtio_unsetinterrupt(&device, msi, handleirq);

    ready = 0;

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

