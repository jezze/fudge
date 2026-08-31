#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/block/block.h>
#include <modules/arch/x86/ide/ide.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>

struct session
{

    unsigned char data[4096];
    unsigned int source;
    unsigned int count;
    unsigned int offset;

};

static struct base_driver driver;
static struct block_interface blockinterface;
static struct session session;

static void handleirq(unsigned int irq)
{

    unsigned char status = ide_getstatus(blockinterface.id);

    if (status & 1)
        return;

    if (session.source)
    {

        ide_rblock(blockinterface.id, session.data + session.offset);

        session.offset += 512;

        if (session.offset == session.count)
        {

            session.source = 0;

            block_notifyblockresponse(&blockinterface, session.data, session.count);

        }

    }

}

static unsigned int blockinterface_onblockrequest(unsigned int source, unsigned int count, unsigned int sector)
{

    if (!session.source)
    {

        session.source = source;
        session.count = count;
        session.offset = 0;

        ide_rpio28(blockinterface.id, 0, count / 512, sector);

        return MESSAGE_OK;

    }

    return MESSAGE_RETRY;

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id, blockinterface_onblockrequest);

}

static unsigned int driver_match(unsigned int id)
{

    return id == IDE_ATA;

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
    pic_unsetroutine(irq);
    apic_unsetroutine(irq);

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

