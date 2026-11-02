#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/block.h>
#include <modules/ide/ide.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>

#define BLOCKSIZE                       2048
#define TYPE_CDROM                      0x05
#define STATUS_ERROR                    0x01
#define STATUS_DRQ                      0x08
#define REASON_COMMAND                  0x01
#define COMMAND_READCAPACITY            0x25
#define COMMAND_READ10                  0x28
#define ATTEMPTS                        3

static struct base_driver driver;
static struct block_interface blockinterface;
static struct spinlock spinlock;
static unsigned short identity[256];
static unsigned char sector[BLOCKSIZE];
static unsigned int device;
static void *blockbuffer;
static unsigned int skip;

/* a session reads the whole sectors around start and count: skip is what is left to drop of the first one */
static void receive(struct block_session *session)
{

    unsigned int count = ide_getbytecount(device);
    unsigned int from;
    unsigned int take;

    if (count > BLOCKSIZE)
        count = BLOCKSIZE;

    ide_rdata(device, sector, count);

    from = (skip < count) ? skip : count;
    take = count - from;

    if (take > session->count - session->offset)
        take = session->count - session->offset;

    buffer_copy((char *)blockbuffer + session->offset, sector + from, take);

    skip -= from;
    session->offset += take;

}

static void progress(void)
{

    struct block_session *session = block_getsession(&blockinterface);
    unsigned char status;

    if (!session->source)
        return;

    status = ide_getstatus(device);

    if (status & STATUS_ERROR)
    {

        session->count = session->offset;

        block_session_done(&blockinterface, session);

    }

    else if (status & STATUS_DRQ)
    {

        if (!(ide_getreason(device) & REASON_COMMAND))
            receive(session);

    }

    else
    {

        session->count = session->offset;

        block_session_done(&blockinterface, session);

    }

}

static void handleirq(unsigned int irq)
{

    spinlock_acquire(&spinlock);
    progress();
    spinlock_release(&spinlock);

}

static void blockinterface_startsession(struct block_session *session)
{

    void *buffer = (void *)kernel_getbuffer(session->source);
    unsigned char packet[IDE_PACKETSIZE];
    unsigned int lba = session->start / BLOCKSIZE;
    unsigned int sectors;

    if (session->count > TASK_BUFFERSIZE)
        session->count = TASK_BUFFERSIZE;

    if (!buffer || session->type != BLOCK_TYPE_READ || !session->count)
    {

        session->count = 0;

        block_session_done(&blockinterface, session);

        return;

    }

    spinlock_acquire(&spinlock);

    blockbuffer = buffer;
    skip = session->start % BLOCKSIZE;
    sectors = (skip + session->count + BLOCKSIZE - 1) / BLOCKSIZE;

    buffer_clear(packet, IDE_PACKETSIZE);

    packet[0] = COMMAND_READ10;
    packet[2] = lba >> 24;
    packet[3] = lba >> 16;
    packet[4] = lba >> 8;
    packet[5] = lba;
    packet[7] = sectors >> 8;
    packet[8] = sectors;

    if (!ide_packet(device, packet, BLOCKSIZE))
    {

        session->count = 0;

        block_session_done(&blockinterface, session);

    }

    spinlock_release(&spinlock);

}

/* polled, before the interrupt is routed: the first command after a media change fails once with unit attention */
static unsigned int readcapacity(void)
{

    unsigned char packet[IDE_PACKETSIZE];
    unsigned int i;

    buffer_clear(packet, IDE_PACKETSIZE);

    packet[0] = COMMAND_READCAPACITY;

    for (i = 0; i < ATTEMPTS; i++)
    {

        unsigned int count = 0;

        if (!ide_packet(device, packet, BLOCKSIZE))
            continue;

        while (ide_wait(device))
        {

            unsigned int length = ide_getbytecount(device);

            if (length > BLOCKSIZE)
                length = BLOCKSIZE;

            ide_rdata(device, sector, length);

            count += length;

        }

        if (count >= 8 && !(ide_getstatus(device) & STATUS_ERROR))
            return ((sector[0] << 24) | (sector[1] << 16) | (sector[2] << 8) | sector[3]) + 1;

    }

    return 0;

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, BLOCKSIZE, blockinterface_startsession);

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    return type == IDE_BUS && ide_identifypacket(id, identity) && ((identity[0] >> 8) & 0x1F) == TYPE_CDROM;

}

static void driver_reset(unsigned int id)
{

    device = id;
    blockinterface.blockcount = readcapacity();

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = ide_getirq(id);

    block_registerinterface(&blockinterface);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = ide_getirq(id);

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(irq, handleirq);
    apic_unsetroutine(irq, handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "atapi", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

