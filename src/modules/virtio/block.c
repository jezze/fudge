#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/block.h>
#include <modules/io/io.h>
#include <modules/pci/pci.h>
#include "virtio.h"

#define DEVICE                          0x1001
#define BLOCKSIZE                       512
#define REQUEST_READ                    0
#define REQUEST_WRITE                   1

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
static unsigned short io;
static unsigned int ready;
static unsigned int msi;

static void handleirq(unsigned int irq)
{

    struct block_session *session = block_getsession(&blockinterface);

    if (!msi)
        io_inb(io + VIRTIO_REG_ISR);

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

static void blockinterface_oninfo(struct event_blockinfo *blockinfo)
{

    blockinfo->buffer = KERNEL_VBUFFER;
    blockinfo->buffersize = TASK_BUFFERSIZE;
    blockinfo->blocksize = BLOCKSIZE;

}

static void blockinterface_startsession(struct block_session *session)
{

    void *buffer = (void *)kernel_getbuffer(session->source);
    unsigned int count = (session->count > TASK_BUFFERSIZE) ? TASK_BUFFERSIZE : session->count;

    count -= count % BLOCKSIZE;

    if (!buffer || !count)
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

    block_initinterface(&blockinterface, id, blockinterface_oninfo, blockinterface_startsession);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == VIRTIO_VENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == DEVICE;

}

static void driver_reset(unsigned int id)
{

    io = pci_getbar(id, 0);

    virtio_negotiate(io, 0);

    if (!virtio_setqueue(&queue, io, 0, memory) || queue.size < 3)
        return;

    pci_setmaster(id);

    ready = 1;

}

static void driver_attach(unsigned int id)
{

    if (!ready)
        return;

    msi = virtio_setinterrupt(id, io, 1, handleirq);

    virtio_setstatus(io, VIRTIO_STATUS_DRIVEROK);
    block_registerinterface(&blockinterface);

}

static void driver_detach(unsigned int id)
{

    if (!ready)
        return;

    block_unregisterinterface(&blockinterface);
    virtio_reset(io);
    virtio_unsetinterrupt(id, msi);

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
