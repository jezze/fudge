#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/block.h>
#include <modules/pci/pci.h>
#include "ahci.h"

#define BUFFERBASE                      (ARCH_MEM_BASE + 0x10000)
#define BUFFERSIZE                      0x8000
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

static unsigned int transfer(unsigned int write, unsigned int sector, unsigned int count)
{

    volatile struct ahci_h2d *fis = (struct ahci_h2d *)table->cfis;
    unsigned int blocks = count / BLOCKSIZE;
    unsigned int i;

    if (!blocks)
        return 0;

    header->flags = (sizeof (struct ahci_h2d) / 4) | (write ? AHCI_HEADER_WRITE : 0);
    header->prdtl = 1;
    header->prdbc = 0;
    table->prd.dba = BUFFERBASE;
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
    writereg(port + AHCI_PCI, 1);

    for (i = 0; i < TIMEOUT; i++)
    {

        if (readreg(port + AHCI_PIS) & AHCI_PIS_TFES)
            break;

        if (!(readreg(port + AHCI_PCI) & 1))
            return (readreg(port + AHCI_PTFD) & AHCI_PTFD_ERR) ? 0 : blocks * BLOCKSIZE;

    }

    stopport();
    startport();

    return 0;

}

static void blockinterface_oninfo(struct event_blockinfo *blockinfo)
{

    blockinfo->buffer = BUFFERBASE;
    blockinfo->buffersize = BUFFERSIZE;
    blockinfo->blocksize = BLOCKSIZE;

}

static void blockinterface_startsession(struct block_session *session)
{

    unsigned int count = (session->count > BUFFERSIZE) ? BUFFERSIZE : session->count;

    spinlock_acquire(&spinlock);

    session->count = transfer(session->type == BLOCK_TYPE_WRITE, session->start / BLOCKSIZE, count);

    block_session_done(&blockinterface, session);
    spinlock_release(&spinlock);

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id, blockinterface_oninfo, blockinterface_startsession);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_SATA;

}

static void driver_reset(unsigned int id)
{

    mmio = pci_ind(id, PCI_CONFIG_BAR5) & 0xFFFFFFF0;

    arch_kmap(mmio, mmio, 0x2000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    pci_setmaster(id);
    writereg(AHCI_GHC, readreg(AHCI_GHC) | AHCI_GHC_AE);

    port = findport();

    if (port)
        setupport();

}

static void driver_attach(unsigned int id)
{

    if (port)
        block_registerinterface(&blockinterface);

}

static void driver_detach(unsigned int id)
{

    if (port)
        block_unregisterinterface(&blockinterface);

}

void module_init(void)
{

    arch_kmap(BUFFERBASE, BUFFERBASE, BUFFERSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE);
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

