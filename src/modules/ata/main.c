#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/block.h>
#include <modules/ide/ide.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>

#define BLOCKSIZE                       512

static void *blockbuffer;
static unsigned short identity[256];
static struct base_driver driver;
static struct block_interface blockinterface;
static struct spinlock spinlock;

static void progress(void)
{

    unsigned char status = ide_getstatus(blockinterface.id);
    struct block_session *session = block_getsession(&blockinterface);

    if (!session->source)
        return;

    if (status & 1)
    {

        session->count = session->offset;

        block_session_done(&blockinterface, session);

        return;

    }

    switch (session->type)
    {

    case BLOCK_TYPE_READ:
        ide_rblock(blockinterface.id, (char *)blockbuffer + session->offset);

        session->offset += BLOCKSIZE;

        if (session->offset >= session->count)
            block_session_done(&blockinterface, session);

        break;

    case BLOCK_TYPE_WRITE:
        if (session->offset < session->count)
        {

            ide_wblock(blockinterface.id, (char *)blockbuffer + session->offset);

            session->offset += BLOCKSIZE;

        }

        else
        {

            block_session_done(&blockinterface, session);

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

static void blockinterface_startsession(struct block_session *session)
{

    void *buffer = (void *)kernel_getbuffer(session->source);

    if (!buffer)
    {

        session->count = 0;

        block_session_done(&blockinterface, session);

        return;

    }

    spinlock_acquire(&spinlock);

    blockbuffer = buffer;

    if (session->count > TASK_BUFFERSIZE)
        session->count = TASK_BUFFERSIZE;

    switch (session->type)
    {

    case BLOCK_TYPE_READ:
        ide_rpio48(blockinterface.id, session->count / BLOCKSIZE, session->start / BLOCKSIZE);

        break;

    case BLOCK_TYPE_WRITE:
        ide_wpio48(blockinterface.id, session->count / BLOCKSIZE, session->start / BLOCKSIZE);

        if (ide_wait(blockinterface.id))
        {

            session->offset = BLOCKSIZE;

            ide_wblock(blockinterface.id, blockbuffer);

        }

        else
        {

            session->count = 0;

            block_session_done(&blockinterface, session);

        }

        break;

    }

    spinlock_release(&spinlock);

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id, BLOCKSIZE, blockinterface_startsession);

}

static unsigned int driver_match(unsigned int id)
{

    return id == IDE_PM && ide_identify(id, identity);

}

static void driver_reset(unsigned int id)
{

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

    base_registerdriver(&driver, IDE_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, IDE_BUS);

}

