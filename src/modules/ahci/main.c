#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/block.h>
#include <modules/pci/pci.h>
#include <modules/apic/apic.h>
#include "ahci.h"

#define BLOCKSIZE                       512
#define MEMORYSIZE                      0x1000
#define TIMEOUT                         0x1000000

static struct base_driver driver;
static struct block_interface blockinterface;
static struct spinlock spinlock;
static unsigned char memory[MEMORYSIZE];
static volatile struct ahci_header *header;
static volatile struct ahci_table *table;
static unsigned long mmio;
static unsigned int port;
static unsigned int msi;

static unsigned int readreg(unsigned int offset)
{

    return *(volatile unsigned int *)(mmio + offset);

}

static void writereg(unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(mmio + offset) = value;

}

static unsigned int waitclear(unsigned int offset, unsigned int mask)
{

    unsigned int i;

    for (i = 0; i < TIMEOUT; i++)
    {

        if (!(readreg(offset) & mask))
            return 1;

    }

    return 0;

}

static void stopport(void)
{

    writereg(port + AHCI_PCMD, readreg(port + AHCI_PCMD) & ~AHCI_PCMD_ST);
    waitclear(port + AHCI_PCMD, AHCI_PCMD_CR);
    writereg(port + AHCI_PCMD, readreg(port + AHCI_PCMD) & ~AHCI_PCMD_FRE);
    waitclear(port + AHCI_PCMD, AHCI_PCMD_FR);

}

static void startport(void)
{

    writereg(port + AHCI_PSERR, 0xFFFFFFFF);
    writereg(port + AHCI_PIS, 0xFFFFFFFF);
    writereg(port + AHCI_PCMD, readreg(port + AHCI_PCMD) | AHCI_PCMD_FRE | AHCI_PCMD_SUD | AHCI_PCMD_POD);
    waitclear(port + AHCI_PTFD, AHCI_PTFD_BSY | AHCI_PTFD_DRQ);
    writereg(port + AHCI_PCMD, readreg(port + AHCI_PCMD) | AHCI_PCMD_ST);

}

static unsigned int findport(void)
{

    unsigned int pi = readreg(AHCI_PI);
    unsigned int i;

    for (i = 0; i < AHCI_PORTS; i++)
    {

        unsigned int offset = AHCI_PORT + i * AHCI_PORTSIZE;

        if ((pi & (1 << i)) && (readreg(offset + AHCI_PSSTS) & 0x0F) == 3 && readreg(offset + AHCI_PSIG) == AHCI_SIG_ATA)
            return offset;

    }

    return 0;

}

static void setupport(void)
{

    unsigned long base = ((unsigned long)memory + 0x3FF) & ~0x3FFUL;

    header = (struct ahci_header *)base;
    table = (struct ahci_table *)(base + 0x500);

    stopport();
    buffer_clear((void *)base, 0x600);

    header->ctba = (unsigned long)table;

    writereg(port + AHCI_PCLB, base);
    writereg(port + AHCI_PCLBU, 0);
    writereg(port + AHCI_PFB, base + 0x400);
    writereg(port + AHCI_PFBU, 0);
    startport();

}

static unsigned int issue(unsigned long buffer, unsigned int write, unsigned int sector, unsigned int count)
{

    volatile struct ahci_h2d *fis = (struct ahci_h2d *)table->cfis;
    unsigned int blocks = count / BLOCKSIZE;

    if (!blocks || !buffer)
        return 0;

    header->flags = (sizeof (struct ahci_h2d) / 4) | (write ? AHCI_HEADER_WRITE : 0);
    header->prdtl = 1;
    header->prdbc = 0;
    table->prd.dba = buffer;
    table->prd.dbau = 0;
    table->prd.dbc = blocks * BLOCKSIZE - 1;
    fis->type = AHCI_TYPE_H2D;
    fis->port = AHCI_FIS_COMMAND;
    fis->command = (write) ? AHCI_ATA_WRITEDMAEXT : AHCI_ATA_READDMAEXT;
    fis->device = AHCI_DEVICE_LBA;
    fis->lba0 = sector;
    fis->lba1 = sector >> 8;
    fis->lba2 = sector >> 16;
    fis->lba3 = sector >> 24;
    fis->countl = blocks;
    fis->counth = blocks >> 8;

    if (!waitclear(port + AHCI_PTFD, AHCI_PTFD_BSY | AHCI_PTFD_DRQ))
        return 0;

    writereg(port + AHCI_PIS, 0xFFFFFFFF);
    writereg(AHCI_IS, 0xFFFFFFFF);
    writereg(port + AHCI_PCI, 1);

    return blocks * BLOCKSIZE;

}

static unsigned int busy(void)
{

    return readreg(port + AHCI_PCI) & 1;

}

static unsigned int failed(void)
{

    return (readreg(port + AHCI_PIS) & AHCI_PIS_TFES) || (readreg(port + AHCI_PTFD) & AHCI_PTFD_ERR);

}

static void poll(void)
{

    unsigned int i;

    for (i = 0; i < TIMEOUT; i++)
    {

        if (!busy() || failed())
            return;

    }

}

static void finish(struct block_session *session)
{

    if (busy() || failed())
    {

        session->count = 0;

        stopport();
        startport();

    }

    block_session_done(&blockinterface, session);

}

static void handleirq(unsigned int irq)
{

    struct block_session *session = block_getsession(&blockinterface);

    spinlock_acquire(&spinlock);

    if (session->source && (!busy() || failed()))
        finish(session);

    writereg(port + AHCI_PIS, 0xFFFFFFFF);
    writereg(AHCI_IS, 0xFFFFFFFF);
    spinlock_release(&spinlock);

}

static void blockinterface_startsession(struct block_session *session)
{

    unsigned long buffer = kernel_getbuffer(session->source);
    unsigned int count = (session->count > TASK_BUFFERSIZE) ? TASK_BUFFERSIZE : session->count;

    spinlock_acquire(&spinlock);

    session->count = issue(buffer, session->type == BLOCK_TYPE_WRITE, session->start / BLOCKSIZE, count);

    if (!session->count)
    {

        block_session_done(&blockinterface, session);

    }

    else if (!msi)
    {

        poll();
        finish(session);

    }

    spinlock_release(&spinlock);

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id, BLOCKSIZE, blockinterface_startsession);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_SATA;

}

static void driver_reset(unsigned int id)
{

    mmio = pci_getbar(id, 5);

    arch_kmap(mmio, mmio, 0x2000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    pci_setmaster(id);
    writereg(AHCI_GHC, readreg(AHCI_GHC) | AHCI_GHC_AE);

    port = findport();

    if (port)
        setupport();

}

static void driver_attach(unsigned int id)
{

    if (!port)
        return;

    msi = apic_setmsiroutine(handleirq);

    if (msi && !pci_setmsi(id, apic_getmsiaddress(), apic_getmsidata(msi)))
    {

        apic_unsetroutine(msi, handleirq);

        msi = 0;

    }

    if (msi)
    {

        writereg(port + AHCI_PIE, AHCI_PIS_DHRS | AHCI_PIS_TFES);
        writereg(AHCI_GHC, readreg(AHCI_GHC) | AHCI_GHC_IE);

    }

    block_registerinterface(&blockinterface);

}

static void driver_detach(unsigned int id)
{

    if (!port)
        return;

    if (msi)
    {

        writereg(AHCI_GHC, readreg(AHCI_GHC) & ~AHCI_GHC_IE);
        pci_unsetmsi(id);
        apic_unsetroutine(msi, handleirq);

        msi = 0;

    }

    block_unregisterinterface(&blockinterface);

}

void module_init(void)
{

    base_initdriver(&driver, "ahci", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

