#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/block.h>
#include <modules/ide/ide.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>

#define BLOCKSIZE                       512
#define STATUS_ERROR                    0x01
#define STATUS_DRQ                      0x08
#define STATUS_BUSY                     0x80

static unsigned int device;
static void *blockbuffer;
static unsigned int active;
static unsigned short identity[256];
static struct base_driver driver;
static struct block_interface blockinterface;
static struct spinlock spinlock;

static void finish(struct block_session *session)
{

    active = 0;

    ide_unlock(device);
    block_session_done(&blockinterface, session);

}

static void progress(void)
{

    struct block_session *session = block_getsession(&blockinterface);
    unsigned char status;

    if (!active)
        return;

    status = ide_getstatus(device);

    if (status & STATUS_BUSY)
        return;

    if (status & STATUS_ERROR)
    {

        session->count = session->offset;

        finish(session);

        return;

    }

    switch (session->type)
    {

    case BLOCK_TYPE_READ:
        if (!(status & STATUS_DRQ))
            break;

        ide_rblock(device, (char *)blockbuffer + session->offset);

        session->offset += BLOCKSIZE;

        if (session->offset >= session->count)
            finish(session);

        break;

    case BLOCK_TYPE_WRITE:
        if (session->offset < session->count)
        {

            if (!(status & STATUS_DRQ))
                break;

            ide_wblock(device, (char *)blockbuffer + session->offset);

            session->offset += BLOCKSIZE;

        }

        else
        {

            finish(session);

        }

        break;

    }

}

static void handleirq(unsigned int irq)
{

    spinlock_acquire(&spinlock);
    progress();
    spinlock_release(&spinlock);

}

static unsigned int blockinterface_startsession(struct block_session *session)
{

    void *buffer = (void *)kernel_getbuffer(session->source);

    if (!buffer)
    {

        session->count = 0;

        block_session_done(&blockinterface, session);

        return MESSAGE_OK;

    }

    if (!ide_lock(device))
        return MESSAGE_RETRY;

    spinlock_acquire(&spinlock);

    active = 1;
    blockbuffer = buffer;

    if (session->count > TASK_BUFFERSIZE)
        session->count = TASK_BUFFERSIZE;

    switch (session->type)
    {

    case BLOCK_TYPE_READ:
        ide_rpio48(device, session->count / BLOCKSIZE, session->start / BLOCKSIZE);

        break;

    case BLOCK_TYPE_WRITE:
        ide_wpio48(device, session->count / BLOCKSIZE, session->start / BLOCKSIZE);

        if (ide_wait(device))
        {

            session->offset = BLOCKSIZE;

            ide_wblock(device, blockbuffer);

        }

        else
        {

            session->count = 0;

            finish(session);

        }

        break;

    }

    spinlock_release(&spinlock);

    return MESSAGE_OK;

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, BLOCKSIZE, blockinterface_startsession);

    blockinterface.blockcount = (identity[83] & (1 << 10)) ? (identity[100] | ((unsigned int)identity[101] << 16)) : (identity[60] | ((unsigned int)identity[61] << 16));

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    return type == IDE_BUS && ide_identify(id, identity);

}

static void driver_reset(unsigned int id)
{

    device = id;

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

    base_initdriver(&driver, "ata", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

