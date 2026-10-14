#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/block.h>
#include <modules/pci/pci.h>
#include <modules/apic/apic.h>
#include "nvme.h"

#define PAGESIZE                        0x1000
#define QUEUESIZE                       8
#define MEMORYSIZE                      0x7000
#define TIMEOUT                         0x1000000
#define FAILED                          0xFFFF

struct queue
{

    volatile unsigned int *sq;
    volatile unsigned int *cq;
    unsigned int id;
    unsigned int sqtail;
    unsigned int cqhead;
    unsigned int phase;

};

static struct base_driver driver;
static struct block_interface blockinterface;
static struct spinlock spinlock;
static unsigned char memory[MEMORYSIZE];
static volatile unsigned int *prplist;
static struct queue admin;
static struct queue io;
static unsigned long mmio;
static unsigned int stride;
static unsigned int ready;
static unsigned int msi;

static unsigned int readreg(unsigned int offset)
{

    return *(volatile unsigned int *)(mmio + offset);

}

static void writereg(unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(mmio + offset) = value;

}

static void writereg64(unsigned int offset, unsigned long value)
{

    writereg(offset, value);
    writereg(offset + 4, (value >> 16) >> 16);

}

static unsigned int waitstatus(unsigned int mask, unsigned int value)
{

    unsigned int i;

    for (i = 0; i < TIMEOUT; i++)
    {

        unsigned int status = readreg(NVME_CSTS);

        if (status & NVME_CSTS_CFS)
            return 0;

        if ((status & mask) == value)
            return 1;

    }

    return 0;

}

static void initqueue(struct queue *queue, unsigned int id, unsigned long sq, unsigned long cq)
{

    queue->sq = (volatile unsigned int *)sq;
    queue->cq = (volatile unsigned int *)cq;
    queue->id = id;
    queue->sqtail = 0;
    queue->cqhead = 0;
    queue->phase = 1;

}

static void submit(struct queue *queue, unsigned int opcode, unsigned int nsid, unsigned long prp1, unsigned long prp2, unsigned int cdw10, unsigned int cdw11, unsigned int cdw12)
{

    volatile unsigned int *entry = queue->sq + queue->sqtail * 16;
    unsigned int i;

    for (i = 0; i < 16; i++)
        entry[i] = 0;

    entry[0] = opcode | (queue->sqtail << 16);
    entry[1] = nsid;
    entry[6] = prp1;
    entry[7] = (prp1 >> 16) >> 16;
    entry[8] = prp2;
    entry[9] = (prp2 >> 16) >> 16;
    entry[10] = cdw10;
    entry[11] = cdw11;
    entry[12] = cdw12;
    queue->sqtail = (queue->sqtail + 1) % QUEUESIZE;

    writereg(NVME_DOORBELL + (queue->id * 2) * stride, queue->sqtail);

}

static unsigned int completed(struct queue *queue)
{

    return ((queue->cq[queue->cqhead * 4 + 3] >> 16) & 1) == queue->phase;

}

static unsigned int consume(struct queue *queue)
{

    unsigned int status = (queue->cq[queue->cqhead * 4 + 3] >> 17) & 0x7FFF;

    queue->cqhead++;

    if (queue->cqhead == QUEUESIZE)
    {

        queue->cqhead = 0;
        queue->phase ^= 1;

    }

    writereg(NVME_DOORBELL + (queue->id * 2 + 1) * stride, queue->cqhead);

    return status;

}

static unsigned int wait(struct queue *queue)
{

    unsigned int i;

    for (i = 0; i < TIMEOUT; i++)
    {

        if (completed(queue))
            return consume(queue);

    }

    return FAILED;

}

static unsigned int command(unsigned int opcode, unsigned int nsid, unsigned long prp1, unsigned int cdw10, unsigned int cdw11)
{

    submit(&admin, opcode, nsid, prp1, 0, cdw10, cdw11, 0);

    return wait(&admin) == 0;

}

static unsigned int setup(void)
{

    unsigned long base = ((unsigned long)memory + PAGESIZE - 1) & ~(PAGESIZE - 1UL);
    volatile unsigned char *identify = (volatile unsigned char *)(base + 5 * PAGESIZE);

    stride = 4 << (readreg(NVME_CAP + 4) & 0x0F);
    prplist = (volatile unsigned int *)(base + 4 * PAGESIZE);

    buffer_clear((void *)base, 4 * PAGESIZE);
    initqueue(&admin, 0, base, base + PAGESIZE);
    initqueue(&io, 1, base + 2 * PAGESIZE, base + 3 * PAGESIZE);
    writereg(NVME_CC, readreg(NVME_CC) & ~NVME_CC_EN);

    if (!waitstatus(NVME_CSTS_RDY, 0))
        return 0;

    writereg(NVME_AQA, (QUEUESIZE - 1) | ((QUEUESIZE - 1) << 16));
    writereg64(NVME_ASQ, (unsigned long)admin.sq);
    writereg64(NVME_ACQ, (unsigned long)admin.cq);
    writereg(NVME_CC, NVME_CC_EN | NVME_CC_IOSQES | NVME_CC_IOCQES);

    if (!waitstatus(NVME_CSTS_RDY, NVME_CSTS_RDY))
        return 0;

    if (!command(NVME_ADMIN_IDENTIFY, 1, (unsigned long)identify, 0, 0))
        return 0;

    blockinterface.blocksize = 1 << identify[128 + (identify[26] & 0x0F) * 4 + 2];

    if (blockinterface.blocksize < 512 || blockinterface.blocksize > PAGESIZE)
        return 0;

    if (!command(NVME_ADMIN_CREATECQ, 0, (unsigned long)io.cq, io.id | ((QUEUESIZE - 1) << 16), NVME_QUEUE_CONTIGUOUS | NVME_QUEUE_INTERRUPTS))
        return 0;

    if (!command(NVME_ADMIN_CREATESQ, 0, (unsigned long)io.sq, io.id | ((QUEUESIZE - 1) << 16), NVME_QUEUE_CONTIGUOUS | (io.id << 16)))
        return 0;

    return 1;

}

static unsigned int issue(unsigned long buffer, unsigned int write, unsigned int sector, unsigned int count)
{

    unsigned int blocks = count / blockinterface.blocksize;
    unsigned int pages = (blocks * blockinterface.blocksize + PAGESIZE - 1) / PAGESIZE;
    unsigned long prp2 = 0;
    unsigned int i;

    if (!blocks || !buffer)
        return 0;

    if (pages == 2)
        prp2 = buffer + PAGESIZE;

    if (pages > 2)
    {

        for (i = 1; i < pages; i++)
        {

            prplist[(i - 1) * 2] = buffer + i * PAGESIZE;
            prplist[(i - 1) * 2 + 1] = 0;

        }

        prp2 = (unsigned long)prplist;

    }

    submit(&io, (write) ? NVME_IO_WRITE : NVME_IO_READ, 1, buffer, prp2, sector, 0, blocks - 1);

    return blocks * blockinterface.blocksize;

}

static void finish(struct block_session *session, unsigned int status)
{

    if (status)
        session->count = 0;

    block_session_done(&blockinterface, session);

}

static void handleirq(unsigned int irq)
{

    struct block_session *session = block_getsession(&blockinterface);

    spinlock_acquire(&spinlock);

    if (session->source && completed(&io))
        finish(session, consume(&io));

    spinlock_release(&spinlock);

}

static void blockinterface_startsession(struct block_session *session)
{

    unsigned long buffer = kernel_getbuffer(session->source);
    unsigned int count = (session->count > TASK_BUFFERSIZE) ? TASK_BUFFERSIZE : session->count;

    spinlock_acquire(&spinlock);

    session->count = issue(buffer, session->type == BLOCK_TYPE_WRITE, session->start / blockinterface.blocksize, count);

    if (!session->count)
        block_session_done(&blockinterface, session);
    else if (!msi)
        finish(session, wait(&io));

    spinlock_release(&spinlock);

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id, 0, blockinterface_startsession);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_NVME;

}

static void driver_reset(unsigned int id)
{

    mmio = pci_getbar(id, 0);

    if (!mmio)
        return;

    arch_kmap(mmio, mmio, 0x4000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    pci_setmaster(id);

    msi = apic_setmsiroutine(handleirq);

    if (msi && !pci_setmsix(id, 0, apic_getmsiaddress(), apic_getmsidata(msi)))
    {

        apic_unsetroutine(msi);

        msi = 0;

    }

    ready = setup();

}

static void driver_attach(unsigned int id)
{

    if (ready)
        block_registerinterface(&blockinterface);

}

static void driver_detach(unsigned int id)
{

    if (ready)
        block_unregisterinterface(&blockinterface);

    if (msi)
    {

        pci_unsetmsix(id);
        apic_unsetroutine(msi);

        msi = 0;

    }

}

void module_init(void)
{

    base_initdriver(&driver, "nvme", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

